<?php
if(!isset($dbi))
	include("db_connection.php");

// ditch old data 
$query1 = "delete from guild_hives";		
$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));


//get the list of possible maps we have
$query1 = "select distinct(k) from players";		
$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
$itr=0;
while( list( $k ) = mysql_fetch_row( $result1 ))
	$KList[$itr++] = $k;

//generate hives for each server we parsed
foreach( $KList as $key => $k)
{
	$query1 = "select distinct(guild) from players where k=$k";		
	$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
	$itr=0;
	while( list( $guild ) = mysql_fetch_row( $result1 ))
		$GuildList[$itr++] = $guild;
	
	//get the hive for each guild
	foreach( $GuildList as $key => $guild)
	{
		//get all players for this guild
		$tguild = str_replace("\\","%",$guild); //random bugs ?
		$query1 = "select x,y,might,PLevel from players where k=$k and guild like '".mysql_real_escape_string($tguild)."'";		
		$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
		unset($Guildx);
		unset($Guildy);
		unset($Guildmight);
		// initial central location
		$xavg = 0;
		$yavg = 0;
		$playercount = 0;
		$TotalMight = 0;
		while( list( $x,$y,$might,$PLevel ) = mysql_fetch_row( $result1 ))
		{
			$Guildx[$playercount] = $x;
			$Guildy[$playercount] = $y;
			$Guildmight[$playercount] = $might;
			$GuildPLevel[$playercount] = $PLevel;
			$TotalMight += $might;
			$xavg += $x;
			$yavg += $y;
			$playercount++;
		}
		if($playercount==0)		
			echo "Mistical bug found, check guild '$guild' = ".mysql_real_escape_string($guild)."<br>";
		$cordcount = $playercount;
		$xavg_prev = $xavg / $cordcount;
		$yavg_prev = $yavg / $cordcount;
		$MaxDistAllowed = 1000 * 1000 + 500 * 500;
		// what is the coordinate to minimize sum of distances ?
		// get avg central location
		// get the avg distance to this location
		// get a new avg location counting only points that are below the avg distance
		//refine central location
		for($PrecisionIncrease=0;$PrecisionIncrease<7;$PrecisionIncrease++)
		{
			$xavg = 0;
			$yavg = 0;
			$cordcount = 0;
			$DistSum = 0;
			$mightsum = 0;
			$MaxPLevel = 0;
			$PLevelSum = 0;
			$PLevelCount = 0;
			for($i=0;$i<count($Guildx);$i++)
			{
				$x = $Guildx[ $i ];
				$y = $Guildy[ $i ];
				$distx = ($x-$xavg_prev);
				$disty = ($y-$yavg_prev);
				$dist = $distx * $distx + $disty * $disty;
//				if( $dist * $dist < $MaxDistAllowed * $MaxDistAllowed) // make long paths a punishment
				if( $dist< $MaxDistAllowed ) // make long paths a punishment
				{
					$xavg += $x;
					$yavg += $y;
					$DistSum += $dist;
					$cordcount++;
					$mightsum += $Guildmight[ $i ];
					if($GuildPLevel[$i]>$MaxPLevel)
						$MaxPLevel = $GuildPLevel[$i];
					if( $GuildPLevel[$i] > 0 )
					{
						$PLevelSum += $GuildPLevel[$i];
						$PLevelCount++;
					}
				}
			}
			if($cordcount>0)
			{
				$xavg_prev = ($xavg / $cordcount);
				$yavg_prev = ($yavg / $cordcount);
				$MaxDistAllowed = $DistSum / $cordcount;
//echo "Refine $xavg_prev $yavg_prev $MaxDistAllowed<br>";
			}
			else
				break;
			// if the space is almost filled with castles than it's good enough central location
			if( $MaxDistAllowed <= $playercount)
				break;
			$DistSumPrev = (int)($DistSum / $cordcount);
		}
		if( $PLevelCount > 0 )
			$PLevelAvg = (int)($PLevelSum / $PLevelCount);
		else
			$PLevelAvg = 0;
		$MaxDist = (int)sqrt( $DistSumPrev );
		$xavg_prev = (int)($xavg_prev);
		$yavg_prev = (int)($yavg_prev);
//echo "Guild $guild central location is at $xavg_prev $yavg_prev with radius $MaxDist and castles $cordcount. Total castle count $playercount<br>";
//exit();
		$query1 = "insert into guild_hives (k,x,y,guild,radius,HiveCastles,TotalCastles,HiveMight,TotalMight,MaxPLevel,AvgPLevel)values($k,$xavg_prev,$yavg_prev,'".mysql_real_escape_string($guild)."',$MaxDist,$cordcount,$playercount,$mightsum,$TotalMight,$MaxPLevel,$PLevelAvg)";		
		$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
	}	
}
?>