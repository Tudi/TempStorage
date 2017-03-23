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
		<td>Might</td>
		<td>kills</td>
<!--		<td>Avg PLevel</td> -->
		<td>Avg CLevel</td>
		<td>Player count</td>
	</tr>
<?php
	$HiddenGuilds = "";
	$query1 = "select name from guilds_hidden where EndStamp > ".time();
	$result1 = mysql_query($query1,$dbi) or die("2017022001".$query1);
	while( list( $name ) = mysql_fetch_row( $result1 ) )
		$HiddenGuilds .= "####$name####";

	$query1 = "select x,y,gname,might,kills,plevel,clevel,pcount from guild_hives_multi order by pcount desc,gname";		
	$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
	while( list( $x,$y,$guild,$might,$kills,$plevel,$clevel,$pcount ) = mysql_fetch_row( $result1 ))
	{
		if( $HiddenGuilds != "" && strpos($HiddenGuilds,$guild) != 0 )
			continue;
/*		
			<td><?php echo $plevel;?></td>
*/			
		?>
		<tr>
			<td><?php echo $x;?></td>
			<td><?php echo $y;?></td>
			<td><?php echo $guild;?></td>
			<td><?php echo GetValShortFormat($might);?></td>
			<td><?php echo GetValShortFormat($kills);?></td>
			<td><?php echo $clevel;?></td>
			<td><?php echo $pcount;?></td>
		</tr>
		<?php
	}
?>	
</table>
