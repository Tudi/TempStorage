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
		<td>Time since</td>
	</tr>
<?php
	$query1 = "select Name1,Name2,NewNameSeenAt from player_renames order by NewNameSeenAt desc";
	$result1 = mysql_query($query1,$dbi) or die("2017022001".$query1);
	while( list( $Name1,$Name2,$Stamp ) = mysql_fetch_row( $result1 ))
	{
		$Player1ArchiveLink = "players.php?FN=".urlencode($Name1);
		$Player2ArchiveLink = "players.php?FN=".urlencode($Name2);
		$when = GetTimeDiffShortFormat($Stamp);
		?>
	<tr>
		<td><a href="<?php echo $Player1ArchiveLink; ?>"><?php echo $Name1; ?></a></td>
		<td><a href="<?php echo $Player2ArchiveLink; ?>"><?php echo $Name2; ?></a></td>
		<td><?php echo $when; ?></td>
	</tr>
	<?php
	}
?>	
</table>
