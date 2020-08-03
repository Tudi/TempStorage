<?php
function GetYear()
{
	return date("Y", GetCompensatedTime());
}

function GetDayOfYear()
{
	return date('z', GetCompensatedTime()) + 1;
}

function getDateFromDay($dayOfYear, $year) 
{
  $date = DateTime::createFromFormat('z Y', strval($dayOfYear) . ' ' . strval($year));
  return $date;
}

function GetCompensatedTime()
{
	global $GameServerTimeDifference;
	return time() + $GameServerTimeDifference * 60;
}

function GetCompensatedDate()
{
	return date('Y-m-d H:i:s', GetCompensatedTime());
}

function OrderMergedList($l)
{
	$RowsAdded=0;
	do{
		$PN = "";
		$BestScore = -1;
		foreach($l as $key => $Stats)
			if($Stats[0] > $BestScore)
			{
				$BestScore = $Stats[0];
				$PN = $key;
			}
		if($BestScore != -1)
		{
			for($i=-1;$i<6;$i++)
				$ret[$RowsAdded][$i]=$l[$PN][$i];
			$RowsAdded++;
			$l[$PN][0] = -1;
		}
	}while($BestScore != -1);
	return $ret;
}
?>