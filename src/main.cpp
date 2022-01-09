void InitializeLog()
{
    auto path = logger::log_directory();
    if (!path) {
        util::report_and_fail("Failed to find standard logging directory"sv);
    }

    *path /= fmt::format("{}.log"sv, Plugin::NAME);
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

#ifndef NDEBUG
    const auto level = spdlog::level::trace;
#else
    const auto level = spdlog::level::info;
#endif

    auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
    log->set_level(level);
    log->flush_on(level);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
}

RE::INPUT_DEVICE lastInputDevice = RE::INPUT_DEVICE::kNone;
int lastIdCode = 0;
bool toggleRunActivated = true;

class RunHandlerEx : public RE::RunHandler
{
public:
    using CanProcess_t = decltype(static_cast<bool (RE::RunHandler::*)(RE::InputEvent*)>(&RE::RunHandler::CanProcess));
    static inline REL::Relocation<CanProcess_t> Old_CanProcess;

    bool New_CanProcess(RE::InputEvent* a_event)
    {
        auto userEvents = RE::UserEvents::GetSingleton();
        auto playerControls = RE::PlayerControls::GetSingleton();
        auto inputDeviceManager = RE::BSInputDeviceManager::GetSingleton();

        // Check if the key is pressed.
        int isPressed = -1;
        if (lastInputDevice == RE::INPUT_DEVICE::kMouse) {
            isPressed = inputDeviceManager->GetMouse()->IsPressed(lastIdCode) ? 1 : 0;
        } else if (lastInputDevice == RE::INPUT_DEVICE::kKeyboard) {
            isPressed = inputDeviceManager->GetKeyboard()->IsPressed(lastIdCode) ? 1 : 0;
        } else if (lastInputDevice == RE::INPUT_DEVICE::kGamepad) {
            isPressed = inputDeviceManager->GetGamepad()->IsPressed(lastIdCode) ? 1 : 0;
        } else {
            // Either the user hasn't pressed the key yet or we don't support his input device.
        }

        if (isPressed != -1) {
            playerControls->data.running = (!!isPressed ^ toggleRunActivated) ? 1 : 0;
        } else {
            // If we don't have a known state yet just set the `running` flag to the `Toggle Run` key state.
            playerControls->data.running = toggleRunActivated;
        }

        auto buttonEvent = a_event->AsButtonEvent();
        if (buttonEvent) {
            if (buttonEvent->userEvent == userEvents->toggleRun) {
                if (buttonEvent->IsDown()) {
                    toggleRunActivated = !toggleRunActivated;
                }
            }
            if (buttonEvent->userEvent == userEvents->run) {
                uint32_t idCode = buttonEvent->idCode;

                // Save the `Run` key identifier and input device for later.
                lastIdCode = idCode;
                lastInputDevice = buttonEvent->GetDevice();

                // Call old function in case someone else has hooked it too.
                Old_CanProcess(this, a_event);
                return false;
            }
        }

        return Old_CanProcess(this, a_event);
    }

    static void InstallHook()
    {
        REL::Relocation<std::uintptr_t> runHandlerVtabl(RE::Offset::RunHandler::Vtbl);
        Old_CanProcess = runHandlerVtabl.write_vfunc(0x01, &New_CanProcess);
    }
};
static_assert(sizeof(RunHandlerEx) == sizeof(RE::RunHandler));

extern "C" DLLEXPORT bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = Plugin::NAME.data();
    a_info->version = Plugin::VERSION[0];

    return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    InitializeLog();
    logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

    SKSE::Init(a_skse);

    RunHandlerEx::InstallHook();

    return true;
}

