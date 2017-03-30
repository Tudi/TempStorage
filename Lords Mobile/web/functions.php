<?php

function GetTimeDiffShortFormat($time, $IsDiff = 0)
{
	if($time==0)
		return "";
	if(!isset($IsDiff) || $IsDiff == 0 )
		$diff = time() - $time;
	else
		$diff = $time;
//echo $time." - ".$diff." ; ";	
	if($diff > 356 * 24 * 60 * 60 )
	{
		$diff = ((int)($diff / 356 / 24 / 60 / 6 ) / 10 );
		if( $diff >= "48")
			$diff = "";
		else
			$diff .= " y";
	}
	else if($diff > 31 * 24 * 60 * 60 )
		$diff = ((int)($diff / 31 / 24 / 60 / 6 ) / 10 ) ."m";
	else if($diff > 24 * 60 * 60 )
		$diff = ((int)($diff / 24 / 60 / 6 ) / 10 ) ."d";
	else if($diff > 60 * 60 )
		$diff = ((int)($diff / 60 / 6 ) / 10 )."h";
	else if($diff>60)
		$diff = ( (int)($diff / 6 ) / 10 )."m";
	else
		$diff = $diff."s";
	return $diff;
}

function GetValShortFormat($val)
{
	$sign = 1;
	if($val < 0)
	{
		$sign = -1;
		$val = -$val;
	}
	if($val > 1000000000 )
		$val = ((int)($val / 100000000 ) / 10 ) ."b";
	else if($val > 1000000 )
		$val = ((int)($val / 100000 ) / 10 ) ."m";
	else if($val > 1000 )
		$val = ((int)($val / 100 ) / 10 )."k";
	
	if($sign == -1)
		$val = "-$val";
	
	return $val;
}

?>