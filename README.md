This plugin overrides the game's handling of the `Run` and `ToggleRun` keys to fix an issue where
the `running` flag gets stuck in the wrong position after opening menus, crossing loading doors, etc, while
the `Run` key is pressed.

## Requirements
* [See example plugin requirements.](https://github.com/Ryan-rsm-McKenzie/ExamplePlugin-CommonLibSSE)
* You may need to make some patches to CommonLibSSE for it to compile depending on your toolchain version. If so, after
cloning in the submodule, apply the [CommonLibSSE.patch](CommonLibSSE.patch) file included.

## Building
```
cmake -B BUILD --preset vs2022-windows .
cmake --build BUILD --config Release
```

## Known issues
* This plugin assumes you have bAlwaysRunByDefault set to 1 in your INI. If you don't it will behave as if you have it
anyway.

* Normally the game stores your previous `running` state in the save game and uses that when you load it. With this
plugin installed that will be disregarded. In other words, if you press the `Toggle Run` key to walk, and then load a
save game, you will be walking after, regardless of whether you were running or walking when you created that save game.
