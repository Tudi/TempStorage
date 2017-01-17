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

func ClickButtonIfAvailable( $ImageName, $X, $Y, $Sleep = 500 )
	;Local $aPos = GetKoPlayerAndPos()
	Local $InfinitLoopBreak = 3
;	while( IsPixelAroundPos( $aPos[0] + 986, $aPos[1] + 36, 0x00FFBE38 ) == 1 and $InfinitLoopBreak > 0 )
	local $Pos = ImageIsAt($ImageName, $X, $Y)
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
		Sleep( $Sleep ) ; wait for the window to refresh
		$InfinitLoopBreak = $InfinitLoopBreak - 1
		$Pos = ImageIsAt($ImageName, $X, $Y)
	wend
endfunc

func CloseLordsMobilePopupWindows()
	ClickButtonIfAvailable("Images/Close_Help_Red_986_37.bmp",986,37)
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
	local $res = SearchResultToVectSingleRes( $result )
	return $res
endfunc

func SearchResultToVectSingleRes( $result )
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

func SearchResultToVectMultiRes( $result )
	local $array = StringSplit($result[0],"|")
	local $resCount = Number( $array[1] )
	;MsgBox( 64, "", "res count " & $resCount )
	local $ret[3]
	$ret[0]=-1
	$ret[1]=-1
	$ret[2]=-1
    For $i = 0 To $resCount
		$ret[$i * 3 + 0]=Int(Number($array[$i * 3 + 2]))
		$ret[$i * 3 + 1]=Int(Number($array[$i * 3 + 3]))
		$ret[$i * 3 + 2]=Int(Number($array[$i * 3 + 4]))	; SAD
		;MouseMove( $ret[0], $ret[1] );
		;MsgBox( 64, "", "found at " & $ret[0] & " " & $ret[1] & " SAD " & $ret[2])
	next
	return $ret
endfunc

func SearchFoodOnScreen()
	global $dllhandle
	Local $aPos = GetKoPlayerAndPos()
	; get a list of all possible gold locations
	local $result = DllCall( $dllhandle, "NONE", "TakeScreenshot", "int", $aPos[0], "int", $aPos[1], "int", $aPos[0] + $aPos[2], "int", $aPos[1]+$aPos[3])
	$result = DllCall( $dllhandle, "NONE", "ApplyColorBitmask", "int", 0x00F0F0F0)
	$result = DllCall( $dllhandle, "str", "ImageSearch_Multiple_ExactMatch", "str", "Fields_food.bmp")
	; now parse the locations we found on the screen
	local $array = StringSplit($result[0],"|")
	local $resCount = Number( $array[1] )
	;MsgBox( 64, "", "res count " & $resCount )
    For $i = 0 To $resCount
		$x=Int(Number($array[$i * 2 + 1]))
		$y=Int(Number($array[$i * 2 + 2]))
		; open up to check for level and location
		MouseMove( $x, $y );
		;MsgBox( 64, "", "found at " & $ret[0] & " " & $ret[1] & " SAD " & $ret[2])
	next
	
endfunc

func GoToKingdomViewScreen()
	ClickButtonIfAvailable("Images/KingdomViewButton_31_568.bmp",31,568, 4000)
endfunc

func ZoomOutKingdomView()
	ClickButtonIfAvailable("Images/ZoomOutKingView_27_482.bmp",27,482)
endfunc

func PushCoordDigit( $Digit )
	Local $aPos = GetKoPlayerAndPos()
	if( $Digit == 1 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 700, $aPos[1] + 250, 1, 0 )
	endif
	if( $Digit == 2 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 775, $aPos[1] + 250, 1, 0 )
	endif
	if( $Digit == 3 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 850, $aPos[1] + 250, 1, 0 )
	endif
	
	if( $Digit == 4 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 700, $aPos[1] + 300, 1, 0 )
	endif
	if( $Digit == 5 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 775, $aPos[1] + 300, 1, 0 )
	endif
	if( $Digit == 6 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 850, $aPos[1] + 300, 1, 0 )
	endif
	
	if( $Digit == 7 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 700, $aPos[1] + 350, 1, 0 )
	endif
	if( $Digit == 8 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 775, $aPos[1] + 350, 1, 0 )
	endif
	if( $Digit == 9 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 850, $aPos[1] + 350, 1, 0 )
	endif
	
	if( $Digit == 0 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 700, $aPos[1] + 400, 1, 0 )
	endif

	; allow the window to update
	Sleep(500)
endfunc

func FlipNumber( $Nr )
	Local $ret = 0
	while($Nr > 0)
		local $Digit = mod($Nr,10)
		$ret = $ret * 10 + $Digit
		$Nr = (int)( $Nr / 10 )
	wend
	return $ret
endfunc

func CountDigits( $Nr )
	Local $ret = 0
	while($Nr > 0)
		$ret = $ret + 1
		$Nr = (int)( $Nr / 10 )
	wend
	return $ret
endfunc

func GetNthDigit( $Nr, $Index )
	$Index = $Index - 1
	for $i = 0 to $Index step 1
		$Nr = (int)( $Nr / 10 )
	next
	local $Digit = mod($Nr,10)
	return $Digit
endfunc

func EnterCoord( $Coord )
	; just a self reminder how far you can go
	if( $Coord > 510 ) then 
		$Coord = 510
	endif
	
	Local $aPos = GetKoPlayerAndPos()
	Local $DigitCount = CountDigits( $Coord )
	while( $DigitCount > 0 )
		local $Digit = GetNthDigit( $Coord, $DigitCount - 1 )
		PushCoordDigit($Digit)
		$DigitCount = $DigitCount - 1
	wend
	; push the ok button 
	MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 800, $aPos[1] + 400, 1, 0 )
	; allow the window to update
	Sleep(500)
endfunc

; We presume we on kingdom view screen
func JumpToKingdomCoord( $k, $x, $y, $IsZoomedOut = 1 )
	Local $aPos = GetKoPlayerAndPos()
	; Open the coord window
	if( $IsZoomedOut == 1 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 550, $aPos[1] + 20, 1, 0 )
	endif
	if( $IsZoomedOut == 0 ) then 
		MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 450, $aPos[1] + 80, 1, 0 )
	endif
	; allow the window to open
	Sleep(500)
	; open edit Kigndom 
	;MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 410, $aPos[1] + 220, 1, 0 )
	; Enter kingdom
	;EnterCoord( $k )
	; Open Edit X
	MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 510, $aPos[1] + 220, 1, 0 )
	; allow the window to open
	Sleep(500)
	EnterCoord( $x )
	MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 610, $aPos[1] + 220, 1, 0 )
	; allow the window to update it's content
	Sleep(500)
	EnterCoord( $y )
	; click the GO button
	MouseClick($MOUSE_CLICK_LEFT, $aPos[0] + 510, $aPos[1] + 310, 1, 0 )
	; allow the window to update it's content
	Sleep(500)
endfunc

func DragScreenToLeft()
	Local $MarginUndragged = 0	; maybe around 50 pixels
	Local $DragLatency = 0 ; might require even 100 to fully drag the screen
	Local $aPos = GetKoPlayerAndPos()
	MouseMove( $aPos[0] + $MarginUndragged, $aPos[1] + $aPos[3] / 2, 0 )
	MouseDown($MOUSE_CLICK_LEFT)
	MouseMove( $aPos[0] + $aPos[2] + $DragLatency - $MarginUndragged , $aPos[1] + $aPos[3] / 2, 9 )
	MouseUp($MOUSE_CLICK_LEFT)
endfunc

; about 12 coord units / screen
func DragScreenToRight()
	Local $aPos = GetKoPlayerAndPos()
	MouseMove( $aPos[0] + $aPos[2] - 100, $aPos[1], 0 )
	MouseDown($MOUSE_CLICK_LEFT)
	Sleep(100)
	MouseMove( $aPos[0] + 100 , $aPos[1], 0 )
	MouseUp($MOUSE_CLICK_LEFT)
endfunc

func SearchForResources()
	Local $aPos = GetKoPlayerAndPos()
	; search patterns : 
	; - starting from a specific location, we try to increase radius
	; - scan a box and note down locations, than sort / search in locations 
	; - start from a specific location and we box search increasing box size
	; go to the kingdom view screen
	GoToKingdomViewScreen()
	; zoom out the map to see as much as possible
	ZoomOutKingdomView()
	; jump to a specific coord on the map
	for $row=0 to 12 step 12 
		JumpToKingdomCoord( 69, 0, $row )
		for $col=0 to 2 step 1
			SearchFoodOnScreen()
			DragScreenToLeft()
		next
	next
endfunc

func SearchGoldOnScreen()
	global $dllhandle
	Local $aPos = GetKoPlayerAndPos()
	; get a list of all possible gold locations
	local $result = DllCall( $dllhandle, "NONE", "TakeScreenshot", "int", $aPos[0], "int", $aPos[1], "int", $aPos[0] + $aPos[2], "int", $aPos[1]+$aPos[3])
	$result = DllCall( $dllhandle, "NONE", "ApplyColorBitmask", "int", 0x00F0F0F0)
	$result = DllCall( $dllhandle, "str", "ImageSearch_Multiple_ExactMatch", "str", "Ruins_gold.bmp")
	local $res = SearchResultToVectMultiRes( $result )
endfunc