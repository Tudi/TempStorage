<?php
set_time_limit(60 * 30);
	
include("db_connection.php");

$f = fopen("Players3.txt","rt");
if(!$f)
	exit("Could not open file");

//need to filter out players that do not get updated in this process.
//If we scanned other plears near him and he did not get updated there is a chance he moved or got renamed
// we should process "new" players yet unseen, and players that we lost in a special way. There is a chance they simply namechanged or swapped realm
$k_ind = 0;
$x_ind = 1;
$y_ind = 2;
$name_ind = 3;
$guild_ind = 4;
$might_ind = 5;
$kills_ind = 6;
$time_ind = 7;

while (($line = fgets($f)) !== false) 
	if(strlen($line)>5)
	{
		$line = str_replace("\n","",$line);
		$parts = explode(" \t ",$line);
		
		//remove HTML chars from fonts before saving to DB
		foreach($parts as $key => $val)
		{
			$parts[$key] = htmlspecialchars_decode($parts[$key]);
			$parts[$key] = str_replace("&colon;",':',$parts[$key]);
			$parts[$key] = str_replace("&lowbar;",'_',$parts[$key]);
			$parts[$key] = str_replace("&vert;",'|',$parts[$key]);
			$parts[$key] = str_replace("&ast;",'*',$parts[$key]);
			$parts[$key] = str_replace("&bsol;",'\\',$parts[$key]);
			$parts[$key] = str_replace("&gt;",'>',$parts[$key]);
			$parts[$key] = str_replace("&lt;",'<',$parts[$key]);
//			$parts[$key] = str_replace(" ",'',$parts[$key]);
//			if( $val != $parts[$key] )				echo "$val == ".$parts[$key]."<br>\n";
		}
//		echo "<br>";
		$LastUpdated = 0;
		
		//chek if this location exists in DB and if it's newer than what we know
		$query1 = "select LastUpdated,kills from players where k ='".$parts[$k_ind]."' and x='".$parts[1]."' and y='".$parts[2]."'";
		$result1 = mysql_query($query1,$dbi) or die("Error : 2017022001 <br> ".$query1." <br> ".mysql_error($dbi));
		list( $LastUpdated, $kills ) = mysql_fetch_row( $result1 );
		//if the value in the DB is newer than the one we provided in the scan, it means it should be skipped and not updated. This can happen when multiple bots are scanning the same map and one goes faster than the other
		// maybe we should insert this into archives ? Handle it later
		if($LastUpdated > $parts[$time_ind] || $kills > $parts[$kills_ind] )
			continue;
		
		//check if this player already exists in another location. Maybe he teleported to a new location
		$query1 = "select LastUpdated,kills from players where name like '".mysql_real_escape_string($parts[$name_ind])."' limit 0,1";
		$result1 = mysql_query($query1,$dbi) or die("Error : 20170220012 <br> ".$query1." <br> ".mysql_error($dbi));
		list( $NameExistsStamp,$kills ) = mysql_fetch_row( $result1 );
		if($NameExistsStamp > $parts[$time_ind] && $kills <= $parts[$kills_ind] )
			continue;	//the name in the DB is newer than the one we loaded from the file. We ignore the file
		else if( $NameExistsStamp != 0 )
		{
			//move to archive the one from DB 
			$query1 = "insert into players_archive ( select * from players where name like '".mysql_real_escape_string($parts[$name_ind])."' limit 0,1";
			$result1 = mysql_query($query1,$dbi) or die("Error : 20170220022 <br>".$query1." <br> ".mysql_error($dbi));
			//delete it
			$query1 = "delete from players where name like '".mysql_real_escape_string($parts[$name_ind])."'";
			$result1 = mysql_query($query1,$dbi) or die("Error : 20170220023 <br>".$query1." <br> ".mysql_error($dbi));
			//make sure we create a new entry for this player 
			$LastUpdated = 0;
		}
		
		if( $LastUpdated != 0)
		{
			//move old to archive
			$query1 = "insert into players_archive ( select * from players where k='".$parts[$k_ind]."' and x='".$parts[1]."' and y='".$parts[2]."')";
			$result1 = mysql_query($query1,$dbi) or die("Error : 2017022002 <br>".$query1." <br> ".mysql_error($dbi));
		
			//update current
			$query1 = "update players set k='".mysql_real_escape_string($parts[$k_ind])."'";
			$query1 .= ", x='".mysql_real_escape_string($parts[$x_ind])."'";
			$query1 .= ", y='".mysql_real_escape_string($parts[$y_ind])."'";
			$query1 .= ", name='".mysql_real_escape_string($parts[$name_ind])."'";
			$query1 .= ", guild='".mysql_real_escape_string($parts[$guild_ind])."'";
			$query1 .= ", might='".mysql_real_escape_string($parts[$might_ind])."'";
			$query1 .= ", kills='".mysql_real_escape_string($parts[$kills_ind])."'";
			$query1 .= ", LastUpdated='".mysql_real_escape_string($parts[$time_ind])."'";
			$query1 .= "where k ='".$parts[$k_ind]."' and x='".$parts[1]."' and y='".$parts[2]."'";
			//echo "$query1<br>";
			$result1 = mysql_query($query1,$dbi) or die("Error : 2017022003 <br>".$query1." <br> ".mysql_error($dbi));
		}
		//create new
		else
		{			
			//insert new
			$query1 = "replace into players ( k,x,y,name,guild,kills,might,lastupdated)values(";
			$query1 .= "'".mysql_real_escape_string($parts[$k_ind])."'";
			$query1 .= ",'".mysql_real_escape_string($parts[$x_ind])."'";
			$query1 .= ",'".mysql_real_escape_string($parts[$y_ind])."'";
			$query1 .= ",'".mysql_real_escape_string($parts[$name_ind])."'";
			$query1 .= ",'".mysql_real_escape_string($parts[$guild_ind])."'";
			$query1 .= ",'".mysql_real_escape_string($parts[$might_ind])."'";
			$query1 .= ",'".mysql_real_escape_string($parts[$kills_ind])."'";
			$query1 .= ",'".mysql_real_escape_string($parts[$time_ind])."'";
			$query1 .= ")";

			echo "$query1<br>";
			$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
		}
	}
?>