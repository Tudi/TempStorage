<?php
require_once("db_connection.php");

if(!isset($name) || !isset($monstertype))
	die("Not a proper upload");

//get monster level for type
$year = GetYear();
$day = GetDayOfYear();

if($objtype == 110)
{
	$Level = GetMonsterLevel($monstertype);
	if($Level==0)
		die();
	
	//check if we have an id for this player
	$query1 = "update PlayerHunts set Lvl$Level=Lvl$Level+1 where Day=$day and year=$year and PlayerName = '".mysqli_real_escape_string($dbi,$name)."'";
	$result1 = mysqli_query($dbi,$query1) or die("Error : 2017022001 <br> ".$query1." <br> ".mysqli_error($dbi));

	list($matched, $changed, $warnings) = sscanf($conn->info, "Rows matched: %d Changed: %d Warnings: %d");
	if($changed == 0)
	{
		$query1 = "insert into PlayerHunts (PlayerName, Lvl$Level, year, day) values('".mysqli_real_escape_string($dbi,$name)."',1,$year,$day)";
		$result1 = mysqli_query($dbi,$query1) or die("Error : 2017022001 <br> ".$query1." <br> ".mysqli_error($dbi));
	}
}
else if($objtype == 111)
{
	//check if we have an id for this player
/*	$query1 = "select 1 from PlayerHuntsList where PlayerName = '".mysqli_real_escape_string($dbi,$name)."' and guid!=$x";
	$result1 = mysqli_query($dbi,$query1) or die("Error : 2017022001 <br> ".$query1." <br> ".mysqli_error($dbi));
	list($AlreadyInserted) = mysqli_fetch_row($result1);
	if($AlreadyInserted == 1)
		die();*/

	$Level = GetMonsterLevel($monstertype);
	if($Level==0)
		die();

	$query1 = "insert into PlayerHuntsList (Lvl,Day,Year,PlayerName,GUID,Monster,Gift,GiftCount) values ($Level,$day,$year,'".mysqli_real_escape_string($dbi,$name)."',$x,$monstertype,$y,$CLevel)";
	$result1 = mysqli_query($dbi,$query1) or die("Error : 2017022001 <br> ".$query1." <br> ".mysqli_error($dbi));
}

//if this is on local server, forward it to main server
if( isset($k) && isset($vip) )
{
	$Escaped_name=urlencode($name);
	//$ch = curl_init("http://rum-lm.eu5.org/UploadData.php?name=$name&monstertype=$monstertype");
	$ch = curl_init("http://rum-lm.eu5.org/UploadData.php?name=$Escaped_name&monstertype=$monstertype&objtype=$objtype&x=$x&y=$y&CLevel=$CLevel");
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_HEADER, 0);
	$data = curl_exec($ch);
	echo "$data";
	curl_close($ch);
}/*/

/*
$NameHash = hash("adler32",$name);
$query1 = "select rowid,Lvl$Level from PlayerHunts where Day=$day and year=$year and PlayerName = '".mysql_real_escape_string($name)."' limit 0,1";
$result1 = mysql_query($query1,$dbi) or die("Error : 2017022001 <br> ".$query1." <br> ".mysqli_error($dbi));
list($rowid,$killcount) = mysql_fetch_row($result1);	
*/
function GetMonsterLevel($Type)
{
	global $dbi;
//	$NameHash = hash("adler32",$name);
	$query1 = "select MonsterLevel from MonsterTypes where MonsterType='$Type' limit 0,1";
	$result1 = mysqli_query($dbi, $query1) or die("Error : 2017022002 <br> ".$query1." <br> ".mysqli_error($dbi));
	list($MonsterLevel) = mysqli_fetch_row($result1);
	if(!isset($MonsterLevel) || $MonsterLevel==0)
	{
		$query1 = "insert into MonsterTypes (MonsterType,MonsterLevel)values($Type,1)";
		$result1 = mysqli_query($dbi,$query1) or die("Error : 2017022003 <br> ".$query1." <br> ".mysqli_error($dbi));
		return 0;
	}
	else
	{
		$query1 = "update MonsterTypes set HuntedCount= HuntedCount + 1 where MonsterType='$Type'";
		$result1 = mysqli_query($dbi, $query1) or die("Error : 2017022002 <br> ".$query1." <br> ".mysqli_error($dbi));
	}
	return $MonsterLevel;
}
/*
function GetPlayerID($name)
{
	global $dbi;
	$NameHash = hash("adler32",$name);
	$query1 = "select rowid from PlayerNames where HashedName='$NameHash' and name = '".mysql_real_escape_string($name)."' limit 0,1";
	$result1 = mysql_query($query1,$dbi) or die("Error : 2017022001 <br> ".$query1." <br> ".mysqli_error($dbi));
	list($rowid) = mysql_fetch_row( $result1 );	
	//create new 
	if( !isset($rowid) || $rowid == 0 )
	{
	}
}

function CreateNewPlayer($name)
{
	global $dbi;
	$NameHash = hash("adler32",$name);
	$query1 = "insert into PlayerNames (HashedName,name)values('$NameHash','".mysql_real_escape_string($name)."')";
	$result1 = mysql_query($query1,$dbi) or die("Error : 2017022001 <br> ".$query1." <br> ".mysqli_error($dbi));
	return GetPlayerID($name);
}*/

?>