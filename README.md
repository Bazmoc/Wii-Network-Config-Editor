# Wii-Network-Config-Editor
This Wii homebrew program allows you to change your Wifi Settings without going to the Wii Menu. Especially useful for portable consoles.

WARNING : This program makes permanent changes to the NAND, use (and/or modify) at your own risks.
Please make a NAND Backup before using my program, just in case.

What is this program and what it does:

	This program is a Wifi settings editor designed especially for Portable Wiis that cannot access the Wii menu to change Network settings. (no Bluetooth module)
	However it works fine with normal Wii's also (if you are too lazy to go to the wii menu or you can't access it for some reason).
	This program changes the SSID, Password, and security type of the network configuration file called "config.dat" located at /shared2/sys/net/02/config.dat on the NAND.
	This program has been tested on both Dolphin and 2 real Wiis (hundreds of times) without any issues.
	
	
Thanks to Fix94 for his "simple Cert.sys extractor" which inspired me for the ISFS reading part.
Thanks to Aurelio for helping me out for the **numerous** issues i had with my program.
Thanks to Supertazon for helping me out for the sound effects and GRRLIB.
