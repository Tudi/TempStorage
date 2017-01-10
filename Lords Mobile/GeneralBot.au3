#include "Defense.au3"
#RequireAdmin

Opt("PixelCoordMode",1)
Opt("MustDeclareVars", 1)
HotKeySet("9", "ExitBot")
HotKeySet("5", "CloseLordsMobilePopupWindows")

global $BotIsRunning = 1
global $dllhandle = DllOpen ( "ImageSearchDLL_x86.dll" )

while( $BotIsRunning == 1)
	Sleep(1000)
wend

DllClose ( $dllhandle )

func ExitBot()
	global $BotIsRunning = 0
endfunc