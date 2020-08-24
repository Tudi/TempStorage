<?php
require_once("db_connection.php");

AddNewPrefix(195,10);
AddNewPrefix(203,9);
AddNewPrefix(59,10);
AddNewPrefix(44,10);
AddNewPrefix(105,10);
AddNewPrefix(120,10);

function AddNewPrefix($OldId,$NewPrefix)
{
	global $dbi;
	$query1 = "select MonsterName from MonsterTypes where MonsterType=$OldId";
	$result1 = mysqli_query($dbi, $query1) or die("Error : 2017022002 <br> ".$query1." <br> ".mysqli_error($dbi));
	list($MonsterName) = mysqli_fetch_row($result1);
	//remove level from name
	$MonsterName = str_replace(array(" 1"," 2"," 3"," 4"," 5"), "", $MonsterName);
	//get all the possible Ids for this mosnter
	$query1 = "select * from MonsterTypes where MonsterName like '$MonsterName%' and MonsterType<256";
	$result1 = mysqli_query($dbi, $query1) or die("Error : 2017022002 <br> ".$query1." <br> ".mysqli_error($dbi));
	while(list($Type, $Name, $Level) = mysqli_fetch_row($result1))
	{
		$NewType = $Type + $NewPrefix * 256;
		$q2 = "Insert ignore into MonsterTypes (MonsterType, MonsterName, MonsterLevel) values ($NewType, '$Name', $Level)";
		$result2 = mysqli_query($dbi, $q2) or die("Error : 2017022002 <br> ".$q2." <br> ".mysqli_error($dbi));
		echo $q2.";<br>";
	}
}
