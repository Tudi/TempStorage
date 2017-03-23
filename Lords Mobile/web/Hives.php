<?php
include("db_connection.php");
if(!isset($k))
	$k = 67;
?>
<link href="css/table.css" rel="stylesheet">
<table class="TFtable">
	<tr>
		<td>x</td>
		<td>y</td>
		<td>Guild name</td>
		<td>Hive radius</td>
		<td>Player count at hive</td>
		<td>Might at hive</td>
		<td>Player count total</td>
		<td>Might total</td>
<!--		<td>Max PLevel</td>
		<td>Avg PLevel</td> -->
		<td>Avg CLevel</td>
	</tr>
<?php
	$HiddenGuilds = "";
	$query1 = "select name from guilds_hidden where EndStamp > ".time();
	$result1 = mysql_query($query1,$dbi) or die("2017022001".$query1);
	while( list( $name ) = mysql_fetch_row( $result1 ) )
		$HiddenGuilds .= "####$name####";

	$query1 = "select x,y,guild,radius,HiveCastles,TotalCastles,HiveMight,TotalMight,MaxPLevel,AvgPLevel,AvgCastleLevel from guild_hives order by HiveCastles desc";		
	$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
	while( list( $x,$y,$guild,$radius,$HiveCastles,$TotalCastles,$HiveMight,$TotalMight,$MaxPLevel,$AvgPLevel,$AvgCLevel ) = mysql_fetch_row( $result1 ))
	{
		if( strpos($HiddenGuilds,$guild) != 0 )
			continue;

		?>
		<tr>
			<td><?php echo $x;?></td>
			<td><?php echo $y;?></td>
			<td><?php echo $guild;?></td>
			<td><?php echo $radius;?></td>
			<td><?php echo $HiveCastles;?></td>
			<td><?php echo GetValShortFormat($HiveMight);?></td>
			<td><?php echo $TotalCastles;?></td>
			<td><?php echo GetValShortFormat($TotalMight);?></td>
			<td><?php echo $AvgCLevel;?></td>
		</tr>
		<?php
	}
?>	
</table>
<br>Show list of locations where the number of players from the same guild is high<br>