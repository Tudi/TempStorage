<?php

function GetTimeDiffShortFormat($time)
{
	$diff = time() - $time;
	if($diff > 356 * 24 * 60 * 60 )
	{
		$diff = ((int)($diff / 356 / 24 / 60 / 6 ) / 10 ) ." y";
		if( $diff == "48.3 y")
			$diff = "";
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
	if($val > 1000000000 )
		$val = ((int)($val / 100000000 ) / 10 ) ."b";
	else if($val > 1000000 )
		$val = ((int)($val / 100000 ) / 10 ) ."m";
	else if($val > 1000 )
		$val = ((int)($val / 100 ) / 10 )."k";
	return $val;
}

?>