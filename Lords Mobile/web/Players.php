<?php
include("db_connection.php");
?>
<br>Show list of players and the gathered data about them. Some players might not be shown as they are using antiscout.<br>
<table border=1>
	<tr>
		<td>k</td>
		<td>x</td>
		<td>y</td>
		<td>Name</td>
		<td>Guild</td>
		<td>Might</td>
		<td>Kills</td>
		<td>Last Updated</td>
		<td>Last seen with prisoners</td>
		<td>Innactive</td>
		<td>Guild rank</td>
		<td>VIP Level</td>
		<td>Player Level</td>
		<td>Last Burned at</td>
		<td>Last Burned at might</td>
		<td>Aprox troops available</td>
		<td>Nodes gathering from</td>
		<td>Castle lvl</td>
		<td>Bounty</td>
		<td>Distance to hive</td>
		<td>Active at X hours</td>
		<td>Active Y hours a day</td>
		<td>First seen ever(age)</td>
	</tr>
<?php
	// do not show hidden players
	$HiddenNames = "";
	$query1 = "select name from players_hidden where EndStamp > ".time();
//echo "$query1<br>";
	$result1 = mysql_query($query1,$dbi) or die("2017022001".$query1);
	while( list( $name ) = mysql_fetch_row( $result1 ) )
		$HiddenNames .= "####$name####";

	$HiddenGuilds = "";
	$query1 = "select name from guilds_hidden where EndStamp > ".time();
//echo "$query1<br>";
	$result1 = mysql_query($query1,$dbi) or die("2017022001".$query1);
	while( list( $name ) = mysql_fetch_row( $result1 ) )
		$HiddenGuilds .= "####$name####";
		
	$query1 = "select k,x,y,name,guild,might,kills,lastupdated,innactive,HasPrisoners,VIP,GuildRank from ";
	if(isset($FilterN))
		$query1 .= "players_archive ";
	else
		$query1 .= "players ";
	
	if($FilterK)
		$Filter = " and k='".mysql_real_escape_string($FilterK)."' ";
	if(isset($FilterN))
		$Filter .= " and name like '".mysql_real_escape_string($FilterN)."' ";
	if(isset($FilterG))
		$Filter .= " and guild like '".mysql_real_escape_string($FilterG)."' ";
	
	if($Filter)
		$query1 .= " where 1=1 $Filter ";
	
	$result1 = mysql_query($query1,$dbi) or die("2017022001".$query1);
	while( list( $k,$x,$y,$name,$guild,$might,$kills,$lastupdated,$innactive,$HasPrisoners,$VIP,$GuildRank ) = mysql_fetch_row( $result1 ))
	{
		if( strpos($HiddenNames,"#".$name."#") != 0 )
			continue;
		if( strpos($HiddenGuilds,"#".$guild."#") != 0 )
			continue;
		
		$LastUpdatedHumanFormat = gmdate("Y-m-d\TH:i:s\Z", $lastupdated);
		//$innactiveHumanFormat = gmdate("Y-m-d\TH:i:s\Z", $innactive);
		$PlayerArchiveLink = $_SERVER['PHP_SELF']."?FilterK=$FilterK&FilterN=".urlencode($name);
		$GuildFilterLink = $_SERVER['PHP_SELF']."?FilterK=$FilterK&FilterG=".urlencode($guild);
		$HasPrisonersHumanFormat = gmdate("Y-m-d\TH:i:s\Z", $HasPrisoners);		
		?>
		<tr>
			<td><?php echo $k; ?></td>
			<td><?php echo $x; ?></td>
			<td><?php echo $y; ?></td>
			<td><a href="<?php echo $PlayerArchiveLink; ?>"><?php echo $name; ?></a></td>
			<td><a href="<?php echo $GuildFilterLink; ?>"><?php echo $guild; ?></a></td>
			<td><?php echo $might; ?></td>
			<td><?php echo $kills; ?></td>
			<td><?php echo $LastUpdatedHumanFormat; ?></td>
			<td><?php echo $HasPrisonersHumanFormat; ?></td>
			<td><?php echo $innactive; ?></td>
			<td><?php echo $VIP; ?></td>
			<td><?php echo $GuildRank; ?></td>
		</tr>
		<?php
	}
?>	
</table>
