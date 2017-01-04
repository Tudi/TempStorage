#include "CommonFunctions.au3"

func IsIncommingAttack()
	; find koplayer
	Local $hWnd = GetKoPlayer()
	
	; where is the window
	Local $aPos = WinGetPos( $hWnd ) ; x, y, w, h
	Local $Width = $aPos[2]
	Local $Height = $aPos[3]
	
	; check if we have almost same red color in first colum
	Local $FirstPixelColor = PixelGetColor( $aPos[0] + $Width / 4, $aPos[1] )
	$FirstPixelColor = ReduceColorPrecision( $FirstPixelColor )
	for( local $i = $Width/4 to $Width/2 step 3 )
		Local $CurPixelColorX = PixelGetColor( $aPos[0] + $i, $aPos[1] )
		$CurPixelColorX = ReduceColorPrecision( $CurPixelColorX )
		if( $CurPixelColorX <> $FirstPixelColor ) then
			return 0
		endif
	next
	
	; check if we have almost same red color in first row
	$FirstPixelColor = PixelGetColor( $aPos[0], $aPos[1] + $Height / 4 )
	$FirstPixelColor = ReduceColorPrecision( $FirstPixelColor )
	for( local $i = $Height/4 to $Height/2 step 3 )
		Local $CurPixelColorY = PixelGetColor( $aPos[0], $aPos[1] + $i )
		$CurPixelColorY = ReduceColorPrecision( $CurPixelColorY )
		if( $CurPixelColorY <> $FirstPixelColor ) then
			return 0
		endif
	next
	
	; if we got here, it means we have an incomming attack
	return 1
endfunc

func HaveShieldActive()
	return 0
endfunc

func ShieldIfAttacked()
	if( HaveShieldActive() == 1 ) then
		return 0
	endif
	if( IsIncommingAttack() == 0 ) then 
		return 0
	endif
	return 1
endfunc