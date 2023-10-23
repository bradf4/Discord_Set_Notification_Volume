
@echo off

::IF YOU DO NOT WANT TO HAVE THE BATCH FILE OPEN DISCORD
::REMOVE THE UPDATE/RUN DISCORD START LINE AND THE TIMEOUT LINE

::NotificationVolume set [0.0-1.0]
set "notifVolume=0.05"

::NormalVolume set [0.0-1.0]
set "normalVolume=0.65"

::CHANGE THIS PATH TO YOUR SHORTCUT TO AVOID CHANGING THE PATH FOR DISCORD EVERYTIME THERE IS AN UPDATE
::search for discord in bottom right right click open file location and use that shortcut path
set "shortcutDiscPath=C:\Users\USER\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Discord Inc\Discord.lnk"

::wait time in seconds for discord to open before setting volumes
set "waitTime=25"

::path to where application was downloaded
set "executablePath=.\set_discord_volume.exe"

::parameters for above application, format is <Application> <NotificationVolume> <NormalVolume>
::Volume is a float with values from [0.0 - 1.0] 1.0 = 100% in volume mixer, 0.63 = 63% in volume mixer
set "parameters=Discord.exe %notifVolume% %normalVolume%"

::wait time to see the echos
set "readTime=2"

echo Starting...
::update discord/run discord
::start "" "%discUpdatePath%" %appParam%

::update discord test
start "" "%shortcutDiscPath%%"

echo Waiting for Discord to open...
::wait
timeout /t %waitTime% /nobreak >nul

echo Running volume adjuster...
start "" "%executablePath%" %parameters%

echo DONE!

:: Add any other actions you want to perform after the executable has run
echo Executable has finished running and waiting.

timeout /t %readTime% /nobreak >nul
:: Close the batch file
exit
