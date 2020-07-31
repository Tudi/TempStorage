<?php
require_once("db_connection.php");

if(!isset($name) || !isset($monstertype))
	die("Not a proper upload");

//get monster level for type
$Level = GetMonsterLevel($monstertype);
if($Level==0)
	die();
$year = GetYear();
$day = GetDayOfYear();
//check if we have an id for this player
$query1 = "update PlayerHunts set Lvl$Level=Lvl$Level+1 where Day=$day and year=$year and PlayerName = '".mysqli_real_escape_string($dbi,$name)."'";
$result1 = mysqli_query($dbi,$query1) or die("Error : 2017022001 <br> ".$query1." <br> ".mysqli_error($dbi));

list($matched, $changed, $warnings) = sscanf($conn->info, "Rows matched: %d Changed: %d Warnings: %d");
if($changed == 0)
{
	$query1 = "insert into PlayerHunts (PlayerName, Lvl$Level, year, day) values('".mysqli_real_escape_string($dbi,$name)."',1,$year,$day)";
	$result1 = mysqli_query($dbi,$query1) or die("Error : 2017022001 <br> ".$query1." <br> ".mysqli_error($dbi));
}

$ch = curl_init("http://rum-lm.eu5.org/UploadData.php?name=$name&monstertype=$monstertype");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HEADER, 0);
$data = curl_exec($ch);
echo "$data";
curl_close($ch);

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
		$query1 = "insert into MonsterTypes (MonsterType,MonsterLevel)values($Type,0)";
		$result1 = mysqli_query($dbi,$query1) or die("Error : 2017022003 <br> ".$query1." <br> ".mysqli_error($dbi));
		return 0;
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