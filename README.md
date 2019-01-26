# RTSS_Crosshair
Crosshair overlay for RivaTuner Statistics Server <br/>
A simple app that prints '+' symbol in RTSS overlay and lets you control position and size of the symbol. <br/>
## Usage
To download press ["releases"](https://github.com/RecursiveLife/RTSS_Crosshair/releases) button up there. <br/>
All controls are shown on launch. <br/>
For some reason order of pressing RightShift combination matters. <br/>
If you set "Onscreen display zoom" setting in rivatuner above zero, center coordinates gonna move past screen borders. If you need that zoom setting, you have to manually move symbol from default position. <br/>
Adjusting position on every launch was annoying, so now you can save current settings in registry key (HKEY_CURRENT_USER\Software\RTSS_Crosshair). <br/>
## Building your own binary
You need an "nmake" and "cl" installed and have their path be in PATH variable. <br/>
If you have Visual Studio installed you already got them, just launch "VS Command Prompt" via Tools->Command Prompt, then change directory to one where you downloaded the files and execute "nmake /f makefile". <br/>
## Credits
Thank you, ShittyCodeMan for creating and sharing [RTSS_Timer](https://github.com/ShittyCodeMan/RTSS_Timer). <br/>
Thanks Microsoft for MSDN. <br/>
Thanks Guru3D for RTSS and SDK for it. <br/>
