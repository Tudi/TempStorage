Import registry file
MinCounter.reg

To install the service : 
C:\Windows\Microsoft.NET\Framework\v4.0.30319\InstallUtil.exe CountMinutesUsed.exe
sc.exe create MinuteCounterService binPath="C:\\Windows\\System32\\Monitoring\\CountMinutesUsed\\CountMinutesUsed.exe" start=auto
sc stop MinuteCounterService
SC DELETE MinuteCounterService

To start the service :
net start MinuteCounterService

Install watchdog
C:\Windows\Microsoft.NET\Framework\v4.0.30319\InstallUtil.exe WatchdogService.exe
sc.exe create Watchdog binPath="C:\\Windows\\System32\\Monitoring\\WatchDog\\WatchdogService.exe" start=auto

Start watchdog
net start Watchdog

If you want to check if values indeed increase
reg query "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\MinCounter" /v "SumOf5"