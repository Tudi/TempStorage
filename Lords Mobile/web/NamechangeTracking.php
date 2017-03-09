<?php
include("db_connection.php");
if(!isset($FilterK))
	$FilterK = 67;
?>
<br>Find a player based on name. Or maybe an older name. Data is only estimated!, based on position change, castle level, VIP, kills<br>
<table border=1>
	<tr>
		<td>Player Name</td>
		<td>Old names</td>
	</tr>
<?php
	$query1 = "select Name1,Name2 from player_renames";
	$result1 = mysql_query($query1,$dbi) or die("2017022001".$query1);
	while( list( $Name1,$Name2 ) = mysql_fetch_row( $result1 ))
	{
		$Player1ArchiveLink = "players.php?FilterK=$FilterK&FilterN=".urlencode($Name1);
		$Player2ArchiveLink = "players.php?FilterK=$FilterK&FilterN=".urlencode($Name2);
		?>
	<tr>
		<td><a href="<?php echo $Player1ArchiveLink; ?>"><?php echo $Name1; ?></a></td>
		<td><a href="<?php echo $Player2ArchiveLink; ?>"><?php echo $Name2; ?></a></td>
	</tr>
	<?php
	}
?>	
</table>
