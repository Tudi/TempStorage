<?php
set_time_limit(2 * 60 * 60);
	
include("db_connection.php");

$SkipMapgen=0;

$f = fopen("parsed_input_03_09.txt","rt");
if(!$f)
	exit("Could not open file");

//need to filter out players that do not get updated in this process.
//If we scanned other plears near him and he did not get updated there is a chance he moved or got renamed
// we should process "new" players yet unseen, and players that we lost in a special way. There is a chance they simply namechanged or swapped realm
$x_ind = 0;
$y_ind = 1;
$name_ind = 2;
$guild_ind = 3;
$Castle_ind = 4;
$guild2_ind = 5;
$GuildRank_ind = 6;
$kills_ind = 7;
$might_ind = 8;
$VIP_ind = 9;

$LastInd = $VIP_ind;

$LastUpdated = time() - 12 * 60 *60;

$k = 67;

//$query1 = "delete from players_network";
//$result1 = mysql_query($query1,$dbi) or die("Error : 20170220023 <br>".$query1." <br> ".mysql_error($dbi));

$FiveMinutes = 5 * 60;
while (($line = fgets($f)) !== false) 
	if(strlen($line)>5)
	{
		$line = str_replace("\n","",$line);
		$parts = explode(" \t ",$line);
		
		//to avoid warnings about vector being too small
		for($i=count($parts);$i<=$LastInd;$i++)
			$parts[$i]=0;	
//		echo "<br>";
/*
		//create new
		$query1 = "insert into players_network ( name,guild,guildF,kills,might,VIP,GuildRank,CastleLvl";
		$query1 .= ",x,y";
		$query1 .= ")values(";
		$query1 .= "'".mysql_real_escape_string($parts[$name_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$guild_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$guild2_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$kills_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$might_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$VIP_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$GuildRank_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$Castle_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$x_ind])."'";
		$query1 .= ",'".mysql_real_escape_string($parts[$y_ind])."'";
		$query1 .= ")";
//			echo "$query1<br>";
		$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));

continue;		
*/
		
		$x = $parts[$x_ind];
		$y = $parts[$y_ind];
		$name = $parts[$name_ind];
		$guild = $parts[$guild_ind];
		$guildF = $parts[$guild2_ind];
		$kills = $parts[$kills_ind];
		$vip = $parts[$VIP_ind];
		$HasPrisoners = 0;
		$PLevel = 0;
		$CLevel = $parts[$Castle_ind];
		$GuildRank = $parts[$GuildRank_ind];
		$might = $parts[$might_ind];
		
		if(strlen($guild)>0)
		{
			$NewName = "[$guild]$name";
			$NewGuild = "[$guild]$guildF";
		}
		else
		{
			$NewName = "$name";
			$NewGuild = "None";
		}
		
		// load old data for this player
		$query1 = "select rowid,LastUpdated,PLevel,kills,might,vip,guildrank from players where name like '%".mysql_real_escape_string($name)."' limit 0,1";
		$result1 = mysql_query($query1,$dbi) or die("Error : 2017022001 <br> ".$query1." <br> ".mysql_error($dbi));
		list( $rowid,$LastUpdated2,$PLevel2,$kills2,$might2,$vip2,$guildrank2 ) = mysql_fetch_row( $result1 );
		// player got renamed ? Try to search it based on coords
		if( $LastUpdated2 == 0 || $LastUpdated2 == "" )
		{
			$query1 = "select rowid,name,LastUpdated,PLevel,kills,might,vip,guildrank,castlelevel from players where k ='$k' and x='$x' and y='$y' and kills >= '$kills' and vip >= '$vip' and castlelevel>=$CLevel limit 0,1";
			$result1 = mysql_query($query1,$dbi) or die("Error : 2017022001 <br> ".$query1." <br> ".mysql_error($dbi));
			list( $rowid, $oldname, $LastUpdated2, $kills, $PLevel2,$kills2,$might2,$vip2,$guildrank2,$castlelevel2 ) = mysql_fetch_row( $result1 );			
			//save the namechange
			if($rowid>0)
			{
				$query1 = "insert into player_renames values('".mysql_real_escape_string($oldname)."','".mysql_real_escape_string($NewName)."')";
				$result1 = mysql_query($query1,$dbi) or die("Error : 2017022001 <br> ".$query1." <br> ".mysql_error($dbi));
			}
		}
		
		//restore non updated from old values
		if($PLevel == 0)
			$PLevel = $PLevel2;
		if($kills == 0)
			$kills = $kills2;
		if($might == 0)
			$might = $might2;
		if($vip == 0)
			$vip = $vip2;
		if($GuildRank == 0)
			$GuildRank = $guildrank2;
		
		//if we have old records, archive and delete them
		if( $rowid > 0 )
		{
			// keep data that does not get updated
			
			//archive if it comes from an older scan session
			if($LastUpdated2 < $LastUpdated - 5 * 60 )
			{
				$query1 = "insert ignore into players_archive (select * from players where rowid=$rowid)";
				$result1 = mysql_query($query1,$dbi) or die("Error : 2017022001 <br> ".$query1." <br> ".mysql_error($dbi));
			}
			
			//ditch old
			$query1 = "delete from players where rowid=$rowid";
			$result1 = mysql_query($query1,$dbi) or die("Error : 2017022001 <br> ".$query1." <br> ".mysql_error($dbi));
		}
		
		$LastUpdated += 1;
		
		//insert new
		$query1 = "insert into players ( k,x,y,name,guild,kills,might,lastupdated,VIP,GuildRank,PLevel,CastleLevel)values(";
		$query1 .= "'".mysql_real_escape_string($k)."'";
		$query1 .= ",'".mysql_real_escape_string($x)."'";
		$query1 .= ",'".mysql_real_escape_string($y)."'";
		$query1 .= ",'".mysql_real_escape_string($NewName)."'";
		$query1 .= ",'".mysql_real_escape_string($NewGuild)."'";
		$query1 .= ",'".mysql_real_escape_string($kills)."'";
		$query1 .= ",'".mysql_real_escape_string($might)."'";
		$query1 .= ",'".mysql_real_escape_string($LastUpdated)."'";
		$query1 .= ",'".mysql_real_escape_string($vip)."'";
		$query1 .= ",'".mysql_real_escape_string($GuildRank)."'";
		$query1 .= ",'".mysql_real_escape_string($PLevel)."'";
		$query1 .= ",'".mysql_real_escape_string($CLevel)."'";
//		$query1 .= ",'".mysql_real_escape_string($HasPrisoners)."'";
		$query1 .= ")";

//			echo "$query1<br>";
		$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
		
		//delete old ones in the neighbourhood
		$xmin = $parts[$x_ind] - 10;
		$xmax = $parts[$x_ind] + 10;
		$ymin = $parts[$y_ind] - 10;
		$ymax = $parts[$y_ind] + 10;
		$olderthan = $LastUpdated - 24 * 60 * 60;
		$LastTime = $LastUpdated;
		//move to archive 
		$query1 = "insert into players_archive ( select * from players where x < $xmax and x > $xmin and y < $ymax and y < $ymin and LastUpdated < $olderthan)";
		$result1 = mysql_query($query1,$dbi) or die("Error : 20170220022 <br>".$query1." <br> ".mysql_error($dbi));
		//delete from actual
		$query1 = "delete from players where x < $xmax and x > $xmin and y < $ymax and y < $ymin and LastUpdated < $olderthan";
		$result1 = mysql_query($query1,$dbi) or die("Error : 20170220023 <br>".$query1." <br> ".mysql_error($dbi));
	}
/**/
$query1 = "delete from players where name like 'Dark.nest'";
$result1 = mysql_query($query1,$dbi) or die("Error : 20170220023 <br>".$query1." <br> ".mysql_error($dbi));
$query1 = "delete from players_archive where name like 'Dark.nest'";
$result1 = mysql_query($query1,$dbi) or die("Error : 20170220023 <br>".$query1." <br> ".mysql_error($dbi));
//include("PostImportActions.php");
?>