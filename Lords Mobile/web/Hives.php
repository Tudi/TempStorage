<?php
include("db_connection.php");
if(!isset($k))
	$k = 67;
?>
<table border=1>
	<tr>
		<td>x</td>
		<td>y</td>
		<td>Guild name</td>
		<td>Hive radius</td>
		<td>Player count at hive</td>
		<td>Might at hive</td>
		<td>Player count total</td>
		<td>Might total</td>
	</tr>
<?php
	$query1 = "select x,y,guild,radius,HiveCastles,TotalCastles,HiveMight,TotalMight from guild_hives where k=$k order by HiveCastles desc";		
	$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
	while( list( $x,$y,$guild,$radius,$HiveCastles,$TotalCastles,$HiveMight,$TotalMight ) = mysql_fetch_row( $result1 ))
	{
		?>
		<tr>
			<td><?php echo $x;?></td>
			<td><?php echo $y;?></td>
			<td><?php echo $guild;?></td>
			<td><?php echo $radius;?></td>
			<td><?php echo $HiveCastles;?></td>
			<td><?php echo $HiveMight;?></td>
			<td><?php echo $TotalCastles;?></td>
			<td><?php echo $TotalMight;?></td>
		</tr>
		<?php
	}
?>	
</table>
<br>Show list of locations where the number of players from the same guild is high<br>