# Explorer Patcher Change log

This document includes the same release notes as in the [Releases](https://github.com/valinet/ExplorerPatcher/releases) section on GitHub.

## 22000.348.39

Tested on build 22000.348.

#### New features

* Built-in support for build 22000.348.
* Implemented option to toggle taskbar auto-hide when double clicking the main taskbar (#389)
* Running `ep_setup.exe` again while EP is already installed will now update the program to the latest version. To uninstall, as the previous behavior did, run `ep_setup.exe /uninstall`
* Implemented absolute height and width parameters for the Windows 10 switcher. These are especially useful for ultra wide monitors, in a scenario similar to the one described in [this post](https://github.com/valinet/ExplorerPatcher/discussions/110#discussioncomment-1673007) - to configure, set `MaxWidthAbs` and/or `MaxHeightAbs` DWORD values in `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher\sws` (#110)
* Provides a simple mechanism for chainloading a custom library when the shell interface is created, from which you can execute your custom code (subject to change, see [this](https://github.com/valinet/ExplorerPatcher/discussions/408#discussioncomment-1674348) for more details) (#408)

#### Feature enhancements

* Option to receive pre-release versions, if available, when checking for updates
* Improved behavior regarding symbol data information; please refer to https://github.com/valinet/ExplorerPatcher/wiki/Symbols for more information (.1)

#### Fixes

* Fixed mismatches between defaults from EP and Windows' defaults
* Application starts with limited functionality on builds lacking hardcoded symbol information; symbol downloading is disabled for now, by default, but can be enabled in the "Advanced" settings section of "Properties"
* Improvements to how hung windows are treated by the Windows 10 window switcher; fixed an issue that severely delayed the time it took the window switcher to display when a window hung on the screen (#449)
* Clicking "Close" in the Windows 10 window switcher is now more tolerant to small mouse movements (#110) (.1)
* The existing "Properties" window is properly displayed if opening it when another instance is already running and is minimized (.2)

## 22000.318.38

Tested on build 22000.318.

#### New features

* Functional Windows 10 network flyout
* Functional Windows 10 battery flyout
* Implemented support for Windows 7 battery flyout (#274)
* Implemented `/extract` switch which unpacks the files from `ep_setup.exe` to disk (#396) (.1):
  * `ep_setup /extract` - extracts `ExplorerPatcher.IA-32.dll` and `ExplorerPatcher.amd64.dll` to current directory
  * `ep_setup /extract test` - extracts to `test` folder in current directory
  * `ep_setup /extract "C:\test with space"` - extracts to `C:\test with space` directory
* Taskbar toolbar layouts are preserved when switching between Windows 10 and Windows 11 taskbars and in general (these will be reset when installing this update but should be subsequently remembered) (#395) (.2)
* Implemented option to toggle taskbar auto-hide when double clicking the main taskbar (#389) (.3)
* Running `ep_setup.exe` again while EP is already installed will now update the program to the latest version. To uninstall, as the previous behavior did, run `ep_setup.exe /uninstall` (.4)
* Implemented absolute height and width parameters for the Windows 10 switcher. These are especially useful for ultra wide monitors, in a scenario similar to the one described in [this post](https://github.com/valinet/ExplorerPatcher/discussions/110#discussioncomment-1673007) - to configure, set `MaxWidthAbs` and/or `MaxHeightAbs` DWORD values in `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\ExplorerPatcher\sws` (#110) (.5)
* Provides a simple mechanism for chainloading a custom library when the shell interface is created, from which you can execute your custom code (subject to change, see [this](https://github.com/valinet/ExplorerPatcher/discussions/408#discussioncomment-1674348) for more details) (#408) (.6)


#### Feature enhancements

* Improved reliability when invoking Control Center (`Win`+`A`) when the taskbar icon is disabled (the icon should now not reappear anymore sometimes) (#242)
* Small reorganization of some options in "Properties"
* Option to receive pre-release versions, if available, when checking for updates (.9)

#### Fixes

* Windows 10 network and battery flyout should now always launch when the tray icon is clicked (#410) (.1)
* Fixed mismatches between defaults from EP and Windows' defaults (.3)
* Application starts with limited functionality on builds lacking hardcoded symbol information; symbol downloading is disabled for now, by default, but can be enabled in the "Advanced" settings section of "Properties" (.7)
* Improvements to how hung windows are treated by the Windows 10 window switcher; fixed an issue that severely delayed the time it took the window switcher to display when a window hung on the screen (#449) (.8)

## 22000.318.37

Tested on build 22000.318 and 22000.346 (currently in Windows Insider beta and release preview channels).

#### New features

* The configuration interface is now accessed by right clicking on the taskbar and choosing "Properties" (previously, it was available in the `Win`+`X` menu). This behavior works when either the Windows 10 or Windows 11 taskbar is enabled. As well, you can launch the "Properties" window directly by pressing `Win`+`R` and typing `rundll32 C:\Windows\dxgi.dll,ZZGUI`, followed by `Enter`.
* Implemented a setup program:
  * To install, simply run `ep_setup.exe`. File Explorer will restart and the program will be enabled right away.
  * To uninstall, there are 2 options:
    * Run `ep_setup.exe` again.
    * Via "Programs and Features" in Control Panel, or "Apps and features" in the Settings app.
  * Learn more about the setup program [here](https://github.com/valinet/ExplorerPatcher/wiki/Installer-How-To)
* Implemented automatic updates; there are 3 settings to choose from when File Explorer starts:
  * Notify about available updates (default) - the program will check for updates and display a notification if a new build is available; you can go to "Properties" and install the update from there
  * Prompt to install available updates - the program will check for updates, download them if any, and prompt you to install them automatically
  * Do not check for updates - the program never checks for updates in the background
  * Of course, you can manually check for updates at any point using "Properties" - "Updates" - "Check for updates". To install the update you were notified about, go to "Properties" - "Updates" - "Install latest version".
  * When installing an update, you will be prompted using UAC to allow elevation - always check that the update originates from matches what you expect; official updates are currently served via https://github.com/valinet/ExplorerPatcher.
  * Learn more about how to configure updates on your system, including how to set a custom endpoint [here](https://github.com/valinet/ExplorerPatcher/wiki/Configure-updates)
* Implemented a proper right click menu for the Windows 11 taskbar - it displays the most common options, similar to previous Windows releases and the Windows 10 taskbar, including frequently accessed items like "Taskbar settings", "Task Manager" and "Show the desktop"
* System tray icons are now left intact when switching between the Windows 10 and Windows 11 taskbars, or when switching builds, reinstalling the application etc. Basically, now, once you set a certain layout for the system tray with the Windows 10 taskbar, it will always be remembered, instead of the annoying behavior where Windows was discarding your choices in order to accommodate the Windows 11 taskbar

#### Feature enhancements

* Hardcoded symbols are now based on file hashes, not build numbers
* Better organization for the settings in "Properties"
* Update toast notifications are displayed only as long as they are required. Subsequent notifications no longer have to wait for the previous ones to timeout, but rather they replace the previous ones (#346) (.2)

#### Fixes

* Mitigated an issue that prevented the Windows 11 taskbar from displaying properly under certain circumstances
* Fixed an issue that would crash the Windows 11 taskbar when it was enabled after positioning the Windows 10 taskbar on either side of the screen (left/right)
* Fixed a bug in "Windows 10 Window switcher" that may have lead to `explorer.exe` crashing when more than 20 windows are opened on the screen (probably the cause for a lot of crashes)
* Fixed numerous issues when injecting processes, including as shell extension; reliability improvements
* Adjusted the padding of the system tray hidden icons indicator so that it is now properly centered vertically when using the classic theme mitigations
* Fixed a memory leak in "Settings Manager"
* Removed verbose output from "Settings Manager"
* Corrected import from `dxgi.dll`
* Fixed typo in configuration UI (#346) (.1)
* Fixed typos and spelling in error message (#346) (.2)
* Fixed bug that prevented "Properties" from working when invoked from Quick Launch or other toolbars (#349) (.2)
* As you may have noticed, releases do not contain unpacked files anymore. Thus, for people looking for a quick way to get the unpacked files, the release notes now include a link to the artifacts generated during during the build process. The artifacts include the usual DLLs (including `dxgi.dll`), plus symbol files and all the helper executables generated during the build. (#351) (.2)
* Setup program version is synchronized with the version of the application (.2)
* Fixed a mismatch between the default value for the setting "Add shortcut to program settings in Win+X menu" displayed in the UI and actually used in the software (#352) (.2)
* Fixed an issue that prevented "Restore default settings" in the "Properties" UI from working (#374) (.3)
* Improved some wording in the Properties UI (#377) (.4)
* Added option to show separators between toolbars in the taskbar (#379) (.4)
* "Properties" is restarted when doing an install/update and closed when uninstalling the application (.5)
* "Properties" can open the last used section when starting (.5)

## 22000.318.36

Tested on build 22000.318.

#### Fixes

* Fixes an issue that prevented Explorer from starting up when Windows 10 taskbar was disabled and Windows 10 window switcher was enabled (#313) (.1)
* Lots of bug and issue fixes for shell extension failing to work under certain circumstances (#259)

## 22000.318.35

Tested on build 22000.318.

#### Feature enhancements

* Start menu position can now be changed without needing to restart File Explorer

#### Fixes

* Improved reliability of Start menu positioning when the monitor topology changes and at startup
* Start menu injection returns error codes from the remote process in the debug console
* Other Start menu quality of life improvements

## 22000.318.34

Tested on build 22000.318.

#### New features

* Added option to enable legacy list view ("SysListView32") in File Explorer (credit @toiletflusher, @anixx from WinClassic) (.1)

#### Feature enhancements

* Built-in support for build 22000.318

## 22000.282.33

Tested on build 22000.282.

#### New features

* Ability to choose language switcher flyout style (option available in the "Properties" - "System tray" section)

#### Fixes

* The key above the `Tab` key is now correctly identified for all keyboard layouts, so the per-application Windows 10 window switcher mode works correctly (#283)

## 22000.282.32

Tested on build 22000.282.

#### New features
* Windows 10 window switcher features new configuration options:
  * Always show switcher on primary monitor
  * Show windows only from current monitor
  * Theme selector: Mica, Acrylic and None
  * Corner preference: Rounded, Rounded small, Not rounded
  * More options for row height
  * [Alt]+[\`] shows the switcher only for windows of the foreground application
  * `[Enter]` switches to selected window (same as `[Space]`) (#240) (.1)
  * Navigation using arrow keys (#240) (.1)
* Ability to choose behavior for Cortana button: hidden, shown and opens Cortana, shown and opens Widgets (#255) (.3)
* Builds are now automatic, generated as soon as new content is pushed to the repository and compiled on GitHub's infrastructure (.6)

#### Feature enhancements
* Clock context menu options "Adjust date/time" and "Customize notification icons" open in Control Panel instead of Settings

#### Fixes
* Taskbar context menu not displaying properly when using classic theme mitigations should now be fixed
* Reliability improvements, correct injection, avoid double patching
* All windows should now be properly detected and included in the Windows 10 window switcher
* Performance enhancements for Windows 10 window switcher: layouts are now precomputed when a window change occurs, so it should be very fast to open and consistent, no matter the current load; also, fast switching does not trigger the window, as it should
* "Show Cortana button" taskbar menu entry now works again (#252) (.2)
* Fixes a bug that prevented correct opening of some applications (like `powershell`) when EP was registered as shell extension (#256) (.4) - PLEASE NOTE THAT RUNNING AS SHELL EXTENSION IS STILL EXPERIMENTAL, UNSUPPORTED, AND ONLY RECOMMENDED FOR SPECIFIC USE CASES THAT YOU SHOULD KNOW ABOUT ALREADY; otherwise, just dropping the DLL in `C:\Windows` is enough
* Windows 10 window switcher switcher now correctly displays UWP apps immediately after launch (#266) (.5)
* Selection and highlight rectangle are now correctly drawn when using light theme on Windows 10 window switcher (.5)
* Keyboard selection (left, right, up, down, space, Return) also works when window switcher is shown by holding down the `ALT` key (#263) (.7)

## 22000.282.31

Tested on build: 22000.282.

#### New features

* Cortana button now opens the Widgets panel
* Ability to choose what happens when clicking the Network icon in the system tray
* Possibility to use the legacy clock flyout
* Possibility to use the legacy volume flyout
* Fixes to fully support the classic theme, with a functional taskbar, system tray, Explorer windows, working context menus; read more about this feature [here](https://github.com/valinet/ExplorerPatcher/discussions/101)
* Choose type of flyout for clock (.1)
* Compatibility with build 22000.282 (.2)

#### Feature enhancements

* Reorganized settings in the GUI
* Added option not to have an accelerator for the `Properties` menu entry in `Win`+`X` (#162)
* The "Adjust date/time" and "Customize notification icons" links in the clock context menu now open the more versatile Control Panel applets (.4)
* The console can now be disabled and then dismissed without causing the Explorer process to restart (.4)
* Implemented a new exported function which restarts File Explorer cleanly, reloading your folder windows: `rundll32 C:\Windows\dxgi.dll,ZZRestartExplorer` (.4)

#### Fixes

* Fixed an issue where the Windows 10 window switcher failed to display some windows (#161)
* Fixed an issue that prevented Start from opening again until Explorer was restarted after opening File Explorer via the Start menu Explorer icon (#145)
* Fixed patching in libvalinet
* Fixed GUI launch path; GUI now launches in an external process, survives Explorer restarts
* Addresses an issue that prevented correct operation under certain circumstances and that could lead to a rare bug where Explorer would crash after a Control Panel window was opened (also related to Control Panel windows not always respecting preferences like "disable navigation bar") (.3)
* Fixed a bug that caused the cursor to move when invoking `Win`+`X` (.4)
* Fixed a bug that caused incorrect positioning of `Win`+`X` when the main taskbar was present on a monitor different than the primary one (.4)
* The Start menu now automatically reloads in the background only when its settings change, instead of when any settings change (.4)
* Improved the reliability of the "Restart File Explorer" link in the Properties GUI (.4)
* Improved the detection of scenarios when the patcher should inject and apply the full set of patches to File Explorer (.4)
* Other bug fixes (.4)

#### Experimental

**PLEASE NOTE THAT RUNNING AS SHELL EXTENSION IS STILL EXPERIMENTAL, UNSUPPORTED, AND ONLY RECOMMENDED FOR SPECIFIC USE CASES THAT YOU SHOULD KNOW ABOUT ALREADY. IT IS NOT REQUIRED FOR OBTAINING ANY OF THE CORE FUNCTIONALITY, IT IS AVAILABLE ONLY FOR SOME ADVANCED USE CASES AND IS STILL A BIGGER WORK-IN-PROGRESS THAN THE MAIN PROJECT.**

The application can now be registered as a shell extension. This will enable the Explorer related functionality to work in Open/Save file dialogs as well. This is especially useful for users wanting proper support of the classic theme in Windows 11.

Please note that this is experimental. For the moment, the preferred installation method remains dropping the DLL in `C:\Windows`. For interested users, I invite you to test this functionality and report your findings in the discussions board.

To enable this, put the 2 DLLs (`ExplorerPatcher.amd64.dll` and `ExplorerPatcher.IA-32.dll`) in a secure folder (for example, `C:\Program Files\ExplorerPatcher`). Then, in that folder, run this command: `regsvr32 ExplorerPatcher.amd64.dll`. After elevation, a message will display informing you of the operation outcome, and if it went well, Explorer will restart displaying the old taskbar.

To uninstall, run `regsvr32 /u ExplorerPatcher.amd64.dll` in the same folder and preferably reboot the computer to unload the DLLs from all applications. Then, the files can be deleted just fine.


## 22000.258.30.6

Tested on build: 22000.258.

* Reworked settings framework
  * More settings are available to customize
  * Most setting changes take effect immediatly
* Implemented Windows 10 window switcher (Alt+Tab)
* GUI
  * Revamped GUI, now the interface is split by categories and is displayed on two columns
  * Regular items do not display a "+" sign anymore at the beginning of their label
  * The current choice is ticked in the drop down menu
  * Regions are now calculated correctly
  * Solved memory leaks
* Option to disable immersive context menus (#96)
* General bug fixes
* Window switcher is now disabled by default (.1)
* Corrected typo in settings (.2)
* Added option to set tray clock to display seconds (.3)
* Added option to set the right click menu of the network system tray icon to launch either (.3):
  * Network settings in the Settings app
  * Network and Sharing Center in the Control Panel
  * Network Connections in the Control Panel
* Added preliminary support for advanced mitigations for correct rendering when using the classic theme (.3)
* GUI optionally loads UI file from DLL folder (helps for easy debugging) (.4)
* Small bug fix for symbols download (.5)
* Better method for closing windows in window switcher (.6)

## 22000.258.26.3

Tested on build: 22000.258.

* Compatibility with OS build 22000.258
* Option to open Network and Sharing Center instead if Network settings when right clicking the network icon in the system tray
* Centered network and sound right click menus and made them toggle on right click
* Reliability enhancements for Start menu positioning (#78) (.1)
* Fixes #85 (.2)
* Fixes #90 (added option to change taskbar icon size) (.3)

## 22000.194.0.25

Tested on build: 22000.194.

* Start menu is hooked from File Explorer; please remove the DLL from `C:\Windows\SystemApps\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy` when using this new version
* `Win`+`X` now opens even when the taskbar is set to autohide (fixes #63)
* `Win`+`C` now opens even when the taskbar is set to autohide (fixes #63)
* Bluetooth and Safe to Remove menus toggle their visibility when clicked
* Bluetooth and Safe to Remove menus are centered relative to the icon they are invoked from
* WiFi list now correctly toggles when clicking the Network icon in the taskbar
* The settings GUI now supports dark mode and switches correctly when the system theme changes
* The settings GUI draws correctly when themes are disabled (classic theme compatibility)
* Removed interoperability with StartAllBack or StartIsBack

## 22000.194.0.23

Tested on build: 22000.194.

* Fixed a bug that showed`Win`+`X` on the wrong monitor in certain scenarios
* `Win`+`X` shows in Windows 11 fashion (centered, above the Start button) if using a centered taskbar with centered Start button as well (using a program like [TaskbarX](https://github.com/valinet/TaskbarX))
* Fixed the bug that prevented the application from loading in`StartMenuExperienceHost.exe` (thanks to @BraINstinct0 for the report)
* Fixed padding and element sizes in GUI so it better fits on smaller screens (thanks to @Gaurav-Original-ClassicShellTester for the report)
* GUI shows application title when run outside of File Explorer
* GUI stays on screen and just reloads the settings when restoring defaults (instead of closing)
* Keyboard (tab) support for GUI: `Esc` to close the window, `Tab` to select the next option, `Shift`+`Tab` to select the previous option, `Space` to toggle (or activate) the option
* Possibility of running the GUI standalone; run this command: `rundll32.exe C:\Windows\dxgi.dll,ZZGUI`; this has the advantage that it stays on the screen after restarting File Explorer

## 22000.194.0.22

Tested on build: 22000.194.

* When the taskbar is located at the bottom of the screen, opening the power user menu (`Win`+`X`) now automatically highlights the "Desktop" entry in the list. Also, the menu items can be activated either with left click, either with right click. Thus, this enables a behavior where you can double click the Start button with the right mouse button in order to quickly show the desktop (thanks to @Gaurav-Original-ClassicShellTester for the suggestion)

## 22000.194.0.21

Tested on build: 22000.194.

* Implemented configuration GUI; to access it, right click the Start button (or press `Win`+`X`) and choose "Properties" (thanks to @BraINstinct0 for the suggestion)

## 22000.194.0.20

Tested on build: 22000.194.

* Huge code refactoring, improved memory patching
* Updated README with better description of the software and how to use it
* Drastically reduced the number of symbols required (around 40MB to download, instead of over 400MB previously)
* Improved Start menu and search positioning, now it is not necessary to have the DLL in `C:\Windows\SystemApps\MicrosoftWindows.Client.CBS_cw5n1h2txyewy`, please remove it from there.
* Skin "Bluetooth" pop-up menu
* Option to hide the search bar in File Explorer completely
* Option to disable the control center button in the taskbar
* Removed the option to disable the modern search box in File Explorer. Instead, you now run a command which disables it globally on your user account (works in "Open" dialogs as well); read [here](https://github.com/valinet/ExplorerPatcher#disable-the-modern-search-box-in-File-Explorer)
* Removed the option to disable the immersive (new) context menu in File Explorer. Instead, you now run a command which disables it globally on your user account; read [here](https://github.com/valinet/ExplorerPatcher#disable-the-immersive-context-menu)
* Ability to disable command bar is described [here](https://github.com/valinet/ExplorerPatcher#disable-the-command-bar-in-File-Explorer)
* Option to apply Mica effect on File Explorer windows (requires `StartIsBack64.dll`), read [here](https://github.com/valinet/ExplorerPatcher#configuration)
* Option to skin system tray icons to match Windows 11 style (requires `StartisBack64.dll`), read [here](https://github.com/valinet/ExplorerPatcher#configuration)

## 22449.1000.0.18

Tested on the following builds: 22449.1000, 22000.176, 22000.1.

New in this version:

* Ability to disable the "modern search box" in File Explorer and uses the classic functional search from early Windows 10 versions or Windows 7/8. To disable this and still use the new search, set `General\AllowModernSearchBox = 1` in `settings.ini`

Fixes:

* Much improved algorithm for enabling the classic taskbar after symbols have downloaded on newer builds
* Restored compatibility with RTM build of Windows 11 (22000.1)
* Fixes the "modern search box" not working correctly when the DLL is used in `C:\Windows\SystemApps\MicrosoftWindows.Client.CBS_cw5n1h2txyewy`

## 22449.1000.0.16

New in this version:

- Compatibility with OS build 22449.1000.0.16.
- Fixed bug that prevented console from showing when the `AllocConsole` setting was specified in `settings.ini`

## 22000.168.0.14

* Start menu and search now respect the taskbar alignment setting
* Ability to customize the number of "Most used" apps in the Start menu apps list
* Symbols are automatically downloaded for Start menu and search; to have the application work with those two, place the DLL in the following additional 2 locations:
  * `C:\Windows\SystemApps\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy`
  * `C:\Windows\SystemApps\MicrosoftWindows.Client.CBS_cw5n1h2txyewy`

## 22000.168.0.12

* Support for showing the app list by default in the Windows 11 Start menu; to enable this feature, copy the DLL to `C:\Windows\SystemApps\Microsoft.Windows.StartMenuExperienceHost_cw5n1h2txyewy` and restart Explorer (works only on 22000.168 for the moment, will be generally available after more testing is performed).
* `Win+X` is now shown correctly on multi monitor setups
* Other bug fixes

## 22000.168.0.11

Fixes [#3](https://github.com/valinet/ExplorerPatcher/issues/3) and [#10](https://github.com/valinet/ExplorerPatcher/issues/10).

## 22000.168.0.10

Improved Explorer hooking.

The application now comes in the form of a single DLL file (`dxgi.dll`) which you have to place in `%windir%` (usually `C:\Windows`). Restart Explorer and that's it.

Please make sure to uninstall the old version before using this new one.

## 22000.168.0.9

Implements [#6](https://github.com/valinet/ExplorerPatcher/issues/6) (option to revert to classic context menu). To disable this feature, add this to the settings.ini file:

```
[General]
AllowImmersiveContextMenus=1
```

## 22000.168.0.8

The popup menu for "Safe to Remove Hardware" is now skinned in the same  style as the Win+X menu and the taskbar context menus, in order to  improve UI consistency.

## 22000.168.0.7

Enables compatibility with [ArchiveMenu](https://github.com/valinet/archivemenu).

## 22000.168.0.6

Fixes [#5](https://github.com/valinet/ExplorerPatcher/issues/5) (removes the delay at logon on newer builds like 22000.168; the bug is similar to the effect introduced by `UndockingDisabled` on these newer builds).

## 22000.1.0.5

Offsets are now determined at runtime

The application was tested on builds 22000.1 and 22000.168.

- Library downloads and parses symbols in order to determine function hooking offsets at runtime and saves the data in a "settings.ini" file located in the application folder for future use; the file is invalidated when a new OS build is detected
- The main executable attempts to determine the location where a jump has to be patched out so that Explorer remains on the 'show old taskbar' code path; it will systematically patch each jz/jnz instruction and will check whether Explorer still runs fine, and, if it does so and does not crash, whether the old taskbar got actually shown; once the offset is determined, it is saved in the "settings.ini" file for future use
- Please have an unmetered active working Internet connection when running for the first time
- Messages from the patcher (i.e. install/uninstall successful message, symbol downloading message) will now display in a toast (Windows 10 notification) if possible; when Explorer is not running, it falls back to using standard MessageBox'es
- Disabled the pre/post build command that restarted sihost.exe in Debug builds

## 22000.1.0.4

New in this release

- Win+X combination is now handled and opens the power user menu

Of note from previous releases:

- Start menu is now displayed on monitor containing the cursor when  invoked with the Windows key if the appropriate configuration is made  into the registry (enables a functionality which was previously  available in Windows 8.1). To activate this, follow this tutorial: https://www.tenforums.com/tutorials/5943-start-menu-open-main-last-display-windows-10-a.html
- The power user menu (Win+X) is now skinned using the system theme, as it was in Windows 10

## 22000.1.0.3

Start menu is now displayed on monitor containing the cursor when  invoked with the Windows key if the appropriate configuration is made  into the registry (enables a functionality which was previously  available in Windows 8.1).

To activate this, follow this tutorial: https://www.tenforums.com/tutorials/5943-start-menu-open-main-last-display-windows-10-a.html

## 22000.1.0.2

Fixes [#1](https://github.com/valinet/ExplorerPatcher/issues/1) (the menu is now skinned using the system theme, as it was in Windows 10).

## 22000.1.0.1

Added functionality to bring back the power user menu (Win+X).

## 22000.1.0.0

Initial version of the application. Only x64 builds.
