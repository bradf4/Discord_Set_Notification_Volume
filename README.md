# DISCORD_NOTIFICATION_SET

## **What does this batch file does**

### *MANIFESTO* (feel free to skip)
 
Have you ever noticed in Discord the notification sound is extremely annoying? This includes Calling, entering/leaving a call, message ping, stream started/ended, muting mic, etc. One way is turning off notification sounds for all of Discord, but that doesn't help. I do like notifications(except from text based communication), they help in certain situation like, "Did I mute my mic?" You would think that a company that loves its users like Discord who offers great deals like Nitro for $9.99 a month(as of 9/12/2023), would simply add a notification sound slider.

> see all the forum posts requesting this from back to 2016.

Well there is another way around it that is slightly better and its BetterDiscord. There they have ways to write plugins and already have plugins to do this exact function. I enjoyed using BetterDiscord, but Discord seemed to update every day, and every day the plugin would seem to be outdated and I quickly became annoyed by the "solution".

Another way is to set the Volume mixer in Windows to the desired volume. Oh wow why isn't this the first solution to the problem this is so easy. **BUT WAIT...** "Theres two instances of discord in my volume mixer!" Discord creates processes that are named the exact same, so changing them is your very own game of roulette. "Okay that was slightly inconvenient, but surely this small indie company Microsoft will remember the process Id in the background and remember what I set the values to!" Yeah maybe? Sometimes its perfect and remembers on launch, most times it makes both volumes the same.

Now comes to the crux of my issue, after updating to Windows 11, Volume Mixer is... bad. It used to show the bar changing on what is making sound, so a simple test of muting my mic would tell me which discord in volume mixer is the notification sounds, but they removed that. Its so bad that I immediately downloaded EarTrumpet, which is better and I still use it, but after a certain user complained about a million chrome tabs appearing(fair), they concatinated all processes (it seems, I could be extremely dumb) so theres only ever one Discord.exe that shows.

### What it does

Running the batch file will run/open discord, and then change the values of both volumes on the as they appear in the Windows 10 volume mixer. The volumes can be changed see details below. You will need the batch file and the .exe file, the .cpp is optional.

## **Whats in this repository**

1. "DiscordVolume.bat"
    - This is what you will run to both open discord and run the set notification volume
	- You can edit the volumes you want for discord in the batch file using a file editor (notepad.exe, notepad++.exe, vscode, etc)
	- **You will need to edit this file and add your PATH to discord.exe and update.exe (for discord)**
	- **Both PATHs can be found in your AppData/Roaming**
	- If you would like to add this batch file to your taskbar, on Windows 11 you need to make the .bat a .exe pin to taskbar and back to a .bat. You can change the icon of the pinned batch file to be a discord icon.

2. "set_volume_appname.cpp" for *"advanced"* users
    - This is the code for the .exe file you can use this to fix any errors you might have or change certain aspects. Go wild.
	- To compile this needs to have access to the libraries **ole32.lib** and **mmdevapi.lib**
	- An example of compiling this file is: "g++ -o set_volume_appname.exe set_volume_appname.cpp -lole32 -lmmdevapi" 
	- To compile and run the code, you will need the Windows SDK and the Windows Runtime Library. 

3. "set_volume_appname.exe"
    - This is the executable called by "DiscordVolume.bat" to set the volumes
	- This takes three inputs:
		1. An Application Name (In this case Discord.exe)
		2. Notification Volume (float value [0.0 - 1.0], 0.63 equates to a 63% in volume mixer)
		3. Normal Voice Volume (float value [0.0 - 1.0], 0.89 equates to a 89% in volume mixer)
	- Make sure to keep this in the same directory as "DiscordVolume.bat" otherwise you will need to change the path in "DiscordVolume.bat"
	
## **Assumptions**

1. This was written for Windows 11, didn't test it for Windows 10, not going to either
2. The biggest assumption is that the PID for the notifications is higher than the PID for noraml voice. I have no way of discerning, but after thousands of tests (one computer opening discord ~5 times) it seems to follow this. If discord ever updates this assumption I'll look into it.