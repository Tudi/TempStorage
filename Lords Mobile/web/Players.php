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
		<td>Last Burned at</td>
		<td>Last Burned at might</td>
		<td>Aprox troops available</td>
		<td>Last active at</td>
		<td>Nodes gathering from</td>
		<td>Guild ranks</td>
		<td>Level</td>
		<td>VIP</td>
		<td>Castle lvl</td>
		<td>Bounty</td>
		<td>Prisoners</td>
		<td>Distance to hive</td>
		<td>Active at X hours</td>
		<td>Active Y hours a day</td>
	</tr>
<?php
	$query1 = "select k,x,y,name,guild,might,kills,lastupdated from players";
	$result1 = mysql_query($query1,$dbi) or die("2017022001".$query1);
	while( list( $k,$x,$y,$name,$guild,$might,$kills,$lastupdated ) = mysql_fetch_row( $result ))
	{
		$LastUpdatedHumanFormat = gmdate("Y-m-d\TH:i:s\Z", $lastupdated);
		?>
		<tr>
			<td><?php echo $k; ?></td>
			<td><?php echo $x; ?></td>
			<td><?php echo $y; ?></td>
			<td><?php echo $name; ?></td>
			<td><?php echo $guild; ?></td>
			<td><?php echo $might; ?></td>
			<td><?php echo $kills; ?></td>
			<td><?php echo $LastUpdatedHumanFormat; ?></td>
		</tr>
		<?php
	}
?>	
</table>
