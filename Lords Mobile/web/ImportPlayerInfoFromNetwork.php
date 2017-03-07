<?php
set_time_limit(2 * 60 * 60);
	
include("db_connection.php");

$SkipMapgen=0;

$f = fopen("parsed_input_07.txt","rt");
if(!$f)
	exit("Could not open file");

//need to filter out players that do not get updated in this process.
//If we scanned other plears near him and he did not get updated there is a chance he moved or got renamed
// we should process "new" players yet unseen, and players that we lost in a special way. There is a chance they simply namechanged or swapped realm
$guid_ind = 0;
$name_ind = 1;
$guild_ind = 2;
$guild2_ind = 3;
$Castle_ind = 4;
$GuildRank_ind = 5;
$kills_ind = 6;
$might_ind = 7;
$VIP_ind = 8;

$LastInd = $VIP_ind;

$query1 = "delete from players_network";
$result1 = mysql_query($query1,$dbi) or die("Error : 20170220023 <br>".$query1." <br> ".mysql_error($dbi));

$FiveMinutes = 5 * 60;
while (($line = fgets($f)) !== false) 
	if(strlen($line)>5)
	{
		$line = str_replace("\n","",$line);
		$parts = explode(" \t ",$line);
		
		//to avoid warnings about vector being too small
//		for($i=count($parts);$i<=$LastInd;$i++)
//			$parts[$i]=0;	
//		echo "<br>";
	
		//delete it
//		$query1 = "delete from players_network where name like '".mysql_real_escape_string($parts[$name_ind])."' and kills >='".$parts[$kills_ind]."' and vip >='".$parts[$VIP_ind]."'";
//		$result1 = mysql_query($query1,$dbi) or die("Error : 20170220023 <br>".$query1." <br> ".mysql_error($dbi));

		//create new
		$query1 = "insert into players_network ( name,guild,guildF,kills,might,VIP,GuildRank,CastleLvl)values(";
		$query1 .= "'".mysql_real_escape_string($parts[$name_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$guild_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$guild2_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$kills_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$might_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$VIP_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$GuildRank_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$Castle_ind])."'";
		$query1 .= ")";

//			echo "$query1<br>";
		$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
	}
/**/
?>