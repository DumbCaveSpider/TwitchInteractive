# v0.1.14-beta.1
- Added **Change Account** button to the Twitch login popup
- Added **Player Effect** event, currently only supports inbuilt effects
- Simplified the process for the **Jumpscare** settings
- Fixed **Wait** action time value disappearing when removing/copying actions
- Added **Player Preview** in the **Color Player** Settings Popup
- Fixed the **Show Cooldown** and **User/Role Restrictions** not being preserved when editing commands
- Added **Tutorial** in the **Handbook** popup

# v0.1.13-beta.1
- Fixing codebase
  - Fixed the hash mismatch
  - Replaced `std::transform` with provided string utils
  - Replaced `dynamic_cast` with `typeinfo_cast` for better compatibility
  - Fixed the Mod Config directory not being used correctly.
- Fancying Jumpscare Info Popup :D

# v0.1.12-beta.1
- Added **HoverAPI** dependency for **Action Tooltips** *(dont let me stop cooking rah)*
- Fixed UI issues with **Sound Settings Popup**
- Added **Jumpscare** event
- Handbook UI looking pretty

# v0.1.11-beta.1
- **Noclip** event will be disabled when exiting the level (Also added extra info about the event)
- Fixed the `${displayname}` identifiers not displaying properly
- Fixed the **Key Code** action node formatting and functionality
- Added Close confirmation on the Command Settings and editing commands when escape button is clicked
- Added extra options for the **Sound Effect** action settings (Speed, Volume, Pitch, Start, End)
- Fixed **Wait** action time value disappearing when adding/moving actions.
- Replaced the input fonts to **Pusab**
- Added Time value on the **Notification** action settings.
- Added Copy Action
- Added **Custom SFX** support for the **Sound Effect** action.
- Added `${rng<min>:<max>}` support for random number generation. *(More info on the Handbook Identifiers)*

# v0.1.10-beta.1
- Added **Experimental Features** option in the Mod Settings.
- Fixed Command Cooldown not detecting correctly.
- Fixed the missing Settings Text Label on certain action node

# v0.1.9-beta.1
- Reworked the codebase
  - Finished replacing all functions to Geode-standard functions

# v0.1.8-beta.1
- Change the codebase (So nothing changed on the end user side)
  - Replace all use of `stoi` and other alternative functions
  - All `buf` variables are all replaced with their respectful variables
  - Replace all `snprintf` to use the Geode Standards `fmt::format`

# v0.1.7-beta.1
- Fixed the texture scaling on the action & event nodes
- Add a close confirmation on the command settings

# v0.1.6-beta.1
- Fixed the popup to fit based on your screen resolution
- Removed **Edit Camera** due to issues with crashes on levels and compliances with the Geode Index guidelines. sorry :(
- Fixed TextBox inputs not allowing user to set an decimal point on certain actions
- Added **Set Gravity**,**Speed Player** & **Noclip** Events
- Added a delete command confirmation popup

# v0.1.5-beta.1
- Added **Sound Effect** & **Stop All Sounds** Events
- Changed the **Wait** Action value to use float value instead of just integer.

# v0.1.4-beta.1
- Added **Search Event** and sorted the events in an alphabetical order
- Added **Scale Player** Event
- Added **Reverse Player** Event
- Attempted to fix the issue with the **Color Player** settings action crash on Android

# v0.1.3-beta.1
- Fixed/Adjusted the Action Node UI
- Removed Unnecessary/useless data stored in the Commands JSON
- Code Cleanup on the Command Settings
- Fixed the crash when Android users attempted to open settings for **Color Player** 

# v0.1.2-beta.1
- Fixed the **Edit Camera** event having a misleading label and fixed the default value
- Fixed the issues with Commands not saving correctly after quitting the game on Android + iOS

# v0.1.1-beta.1
- Added Twitch Status Label on the PauseLayer
- Fixed Touch Priorities on the Scroll Layers
- Code Cleanup

# v0.1.0-beta.1
- Initial release