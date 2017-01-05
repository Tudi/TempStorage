#include "Defense.au3"
#RequireAdmin

Opt("PixelCoordMode",1)
Opt("MustDeclareVars", 1)
HotKeySet("9", "ExitBot")
HotKeySet("5", "HaveShieldActive")

global $BotIsRunning = 1

while( $BotIsRunning == 1)
	Sleep(1000)
wend

func ExitBot()
	global $BotIsRunning = 0
endfunc