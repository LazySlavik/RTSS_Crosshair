RTSS_Crosshair.exe: RTSS_Crosshair.cpp
	cl /O2 /GS- RTSS_Crosshair.cpp kernel32.lib user32.lib shell32.lib winmm.lib Advapi32.lib
