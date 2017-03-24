<?php
include("db_connection.php");
?>
<link href="css/table.css" rel="stylesheet">
<table>
  <thead style="background-color: #60a917">
	<tr>
		<td>x</td>
		<td>y</td>
		<td>level</td>
		<td>player</td>
		<td>Last Updated</td>
	</tr>
  </thead>
  <tbody class="TFtable">
	<?php
	if(!isset($type))
		$type=6;
	$query1 = "select x,y,level,playername,lastupdated from resource_nodes where rtype='".mysql_real_escape_string($type)."'";
	$result1 = mysql_query($query1,$dbi) or die("2017022001".$query1);
	while( list( $x,$y,$level,$playername,$lastupdated ) = mysql_fetch_row( $result1 ))
	{	
		$PlayerArchiveLink = "?FN=".urlencode($playername);
		$LastUpdatedAsDiff = GetTimeDiffShortFormat($lastupdated);
?>
<tr>
<td><?php echo $x; ?></td>
<td><?php echo $y; ?></td>
<td><?php echo $level; ?></td>
<td><a href="<?php echo $PlayerArchiveLink; ?>"><?php echo $playername; ?></a></td>
<td><?php echo $LastUpdatedAsDiff; ?></td>
</tr>
<?php
}
?>	
  </tbody>
</table>

