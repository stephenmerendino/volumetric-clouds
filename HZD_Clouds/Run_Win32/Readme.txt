To Run From Source:
Compile in x86 (We were never able to compile in x64 last mod with Squirrel)
Set working directory of MechGame project to be $(SolutionDir)Run_$(PlatformName) in Visual Studio (otherwise the the app won't be able to read in files)


Keyboard Controls:
F1, F2, F3 - Change window resolution
F4 - Set window to borderless
F5 - Set window to bordered
F6 - Set window to fullscreen borderless window
F7 - Change window title to "Yo, wat up?"
F8 - Center window
F9 - Output config list to debug output (Visual Studio)
Left/Right Arrow - Cycle Presentation Modes


Console Stuff:
~ (Tilde) - Open/Close Console
Left/Right Arrow - move cursor in the input buffer
Up/Down Arrow - cycle through previously entered commmands
Ctrl-C/Ctrl-X/Ctrl-V - Copy/Cut/Paste like you would expect (works with windows clipboard)
Page Up/Page Down - Scroll the console log up and down
Home/End - Jump to beginning/end of input buffer
Shift - Hold down to highlight text in the input region (works with arrow keys and also home/end)


Console Commands:
clear - clears the developer console
exit - exits the developer console
help - shows all registered console commands
list_config - shows all settings that are currently loaded in the config system
quit - quits the app
save_console - [string:filepath] :saves the console log out to a file. Can't use spaces or quotes (eg: save_console console.txt)

Optionals:
Baby's first deferred renderer
