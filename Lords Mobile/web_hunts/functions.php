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
	if(!isset($ret))
		return NULL;
	return $ret;
}
function CalcNumberOfDaysWorthOfHunts($Stats)
{
	$DaysWorth = 0;
	do{
		if($Stats[1] >= 15 && $Stats[2] >= 3)
		{
			$DaysWorth++;
			$Stats[1] -= 15;
			$Stats[2] -= 3;
		}
		else if($Stats[2] >= 7)
		{
			$DaysWorth++;
			$Stats[2] -= 7;
		}
		else if($Stats[3] >= 2)
		{
			$DaysWorth++;
			$Stats[3] -= 2;
		}
		else if($Stats[4] >= 1)
		{
			$DaysWorth++;
			$Stats[4] -= 1;
		}
		else
			break;
	}while(1);
	return $DaysWorth;
}
function SafeToExecuteOnMysql($val)
{
	$len = strlen($val);
	$isSafe = 0;
	for($i=0;$i<$len;$i++)
	{
		if($val[$i]=='\\' || $val[$i]=='\`' || $val[$i]=='\'' || $val[$i]=='\"' || $val[$i]==';' || $val[$i]=='\n' || $val[$i]=='\r' || $val[$i]==0x1a)
		{
//			echo "'".$val[$i]."'";
			return 0;
		}
		if($val[$i] >= 'a' && $val[$i] <= 'z')
			$isSafe++;
		if($val[$i] >= 'A' && $val[$i] <= 'Z')
			$isSafe++;
		if($val[$i] >= '0' && $val[$i] <= '9')
			$isSafe++;
		if($val[$i] == ' ')
			$isSafe++;
	}
//	echo "$isSafe==$len";
	return $isSafe==$len;
}
?>