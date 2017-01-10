#include <AutoItConstants.au3>

func GetKoPlayer()
	Local $hWnd = WinGetHandle("[Title:KOPLAYER 1.4.1052")
	if( @error ) then
		$hWnd = WinGetHandle("[CLASS:Qt5QWindowIcon]")
	endif
	return $hWnd
endfunc

func ReduceColorPrecision( $color, $Mask = 0 )
	if( $Mask == 0 ) then
		$Mask = 0x00F0F0F0
	endif
	return BitAND( $color, $Mask )
endfunc

func GetKoPlayerAndPos()
	Local $hWnd = GetKoPlayer()
	Local $aPos = WinGetPos( $hWnd ) ; x, y, w, h
	Local $bPos[5]
	$bPos[0] = $aPos[0] + 2; there is a left pannel that can change it's size. Seems like it pinches off the content 2 pixels
	$bPos[1] = $aPos[1] + 38 ; content of the player starts below the menu bar
	$bPos[2] = $aPos[2] - 63 ; Borders
	$bPos[3] = $aPos[3] - 38 - 2 - 1; More borders
	$bPos[4] = $hWnd
	return $bPos
endfunc

; just in case our positioning is not perfect, Pixel getcolor should still work okish
func IsPixelAroundPos( $x, $y, $Color, $Mask = 0, $Radius = 0 )
	if( $Radius == 0 ) then
		$Radius = 2
	endif
	if( $Mask == 0 ) then
		$Mask = 0x00FCFCFC
	endif
	$Color = ReduceColorPrecision( $Color, $Mask )
	for $y2 = $y - $Radius to $y + $Radius
		for $x2 = $x - $Radius to $x + $Radius
			local $col = PixelGetColor( $x2, $y2 )
			$col = ReduceColorPrecision( $col, $Mask )
;			MouseMove( $x2, $y2 )
;			FileWriteLine ( "PixelsAroundMouse.txt", Hex($Color) & "!=" & Hex($col) & " Mask " & Hex($Mask) & " rad " & $Radius )
			if( $col == $Color ) then 
;				FileWriteLine ( "PixelsAroundMouse.txt", "Matched" )
				return 1
			endif
		next
	next
	return 0
endfunc

func LMIsCastleScreen()
	Local $aPos = GetKoPlayerAndPos()
	return ( IsPixelAroundPos( $aPos[0] + 114, $aPos[1] + 80, 0x00EFB489 ) == 1 )
endfunc

func LMIsRealmScreen()
	Local $aPos = GetKoPlayerAndPos()
	return ( IsPixelAroundPos( $aPos[0] + 115, $aPos[1] + 81, 0x00DEC152 ) == 1 )
endfunc

; pushing this button will make screen less clutered
func LMIsZoomInButtonVisible()
	Local $aPos = GetKoPlayerAndPos()
	return ( IsPixelAroundPos( $aPos[0] + 18, $aPos[1] + 498, 0x00CAA84D ) == 1 )
endfunc

func CloseLordsMobilePopupWindows()
	;Local $aPos = GetKoPlayerAndPos()
	Local $InfinitLoopBreak = 3
;	while( IsPixelAroundPos( $aPos[0] + 986, $aPos[1] + 36, 0x00FFBE38 ) == 1 and $InfinitLoopBreak > 0 )
	local $Pos = ImageIsAt("Images/Close_Help_Red_986_37.bmp",986,37)
	;MsgBox( 64, "", "found at " & $Pos[0] & " " & $Pos[1] & " SAD " & $Pos[2])
	while( $Pos[2] == 0 and $InfinitLoopBreak > 0 )
	
		;MouseMove( $aPos[0] + 986, $aPos[1] + 36 )
		;WinActivate( $aPos[4] )
		;MouseClick( $MOUSE_CLICK_LEFT, $aPos[0] + 986, $aPos[1] + 36, 1, 0 )
		MouseClick( $MOUSE_CLICK_LEFT, $Pos[0] + 16, $Pos[1] + 16, 1, 0 )
		;ControlClick( $aPos[4],"", "", "left", 1, 986, 36)
		;_MouseClickPlus($aPos[4], "left", $aPos[0] + 986, $aPos[1] + 36 )
		;MouseDown($MOUSE_CLICK_LEFT)
		;Sleep(10000)
		;MouseUp($MOUSE_CLICK_LEFT)
		Sleep( 500 ) ; wait for the window to refresh
		$InfinitLoopBreak = $InfinitLoopBreak - 1
		$Pos = ImageIsAt("Images/Close_Help_Red_986_37.bmp",986,37)
	wend
endfunc

Func _MouseClickPlus($Window, $Button = "left", $X = "", $Y = "", $Clicks = 1)
    MsgBox(1, "", "112333")
    Local $MK_LBUTTON = 0x0001
    Local $WM_LBUTTONDOWN = 0x0201
    Local $WM_LBUTTONUP = 0x0202

    Local $MK_RBUTTON = 0x0002
    Local $WM_RBUTTONDOWN = 0x0204
    Local $WM_RBUTTONUP = 0x0205

    Local $WM_MOUSEMOVE = 0x0200

    Local $i = 0

    Select
        Case $Button = "left"
            $Button = $MK_LBUTTON
            $ButtonDown = $WM_LBUTTONDOWN
            $ButtonUp = $WM_LBUTTONUP
        Case $Button = "right"
            $Button = $MK_RBUTTON
            $ButtonDown = $WM_RBUTTONDOWN
            $ButtonUp = $WM_RBUTTONUP
    EndSelect

    If $X = "" Or $Y = "" Then
        $MouseCoord = MouseGetPos()
        $X = $MouseCoord[0]
        $Y = $MouseCoord[1]
    EndIf

    For $i = 1 To $Clicks
        DllCall("user32.dll", "int", "SendMessage", _
                "hwnd", WinGetHandle($Window), _
                "int", $WM_MOUSEMOVE, _
                "int", 0, _
                "long", _MakeLong($X, $Y))

        DllCall("user32.dll", "int", "SendMessage", _
                "hwnd", WinGetHandle($Window), _
                "int", $ButtonDown, _
                "int", $Button, _
                "long", _MakeLong($X, $Y))
		
		Sleep( 100 )
		
        DllCall("user32.dll", "int", "SendMessage", _
                "hwnd", WinGetHandle($Window), _
                "int", $ButtonUp, _
                "int", $Button, _
                "long", _MakeLong($X, $Y))
    Next
EndFunc  ;==>_MouseClickPlus

Func _MakeLong($LoWord, $HiWord)
    Return BitOR($HiWord * 0x10000, BitAND($LoWord, 0xFFFF))
EndFunc  ;==>_MakeLong

Func ImageIsAt( $ImgName, $x, $y )
	global $dllhandle
	Local $AcceptedMisplaceError = 2
	Local $Radius = 16 + $AcceptedMisplaceError
	Local $aPos = GetKoPlayerAndPos()
	local $x2 = $x + $aPos[0]
	local $y2 = $y + $aPos[1]
	local $result = DllCall( $dllhandle, "NONE", "TakeScreenshot", "int", $x2 - $Radius, "int", $y2 - $Radius, "int", $x2 + $Radius, "int", $y2 + $Radius)
	$result = DllCall( $dllhandle, "NONE", "ApplyColorBitmask", "int", 0x00F0F0F0)
	$result = DllCall( $dllhandle, "str", "ImageSearch_SAD", "str", $ImgName)
	local $res = SearchResultToVect( $result )
	return $res
endfunc

func SearchResultToVect( $result )
	local $array = StringSplit($result[0],"|")
	local $resCount = Number( $array[1] )
	;MsgBox( 64, "", "res count " & $resCount )
	local $ret[3]
	$ret[0]=-1
	$ret[1]=-1
	$ret[2]=-1
	if( $resCount > 0 ) then
		$ret[0]=Int(Number($array[2]))
		$ret[1]=Int(Number($array[3]))
		$ret[2]=Int(Number($array[4]))	; SAD
		;MouseMove( $ret[0], $ret[1] );
		;MsgBox( 64, "", "found at " & $ret[0] & " " & $ret[1] & " SAD " & $ret[2])
	endif
	return $ret
endfunc