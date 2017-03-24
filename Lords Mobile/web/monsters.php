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
		<td>type</td>
		<td>Last Updated</td>
	</tr>
  </thead>
  <tbody class="TFtable">
	<?php
	if(!isset($type) || $type=="rare")
		$type="19,18,16";
	$query1 = "select x,y,level,mtype,lastupdated from monsters where mtype not in (".mysql_real_escape_string($type).")";
	$result1 = mysql_query($query1,$dbi) or die("2017022001".$query1);
	while( list( $x,$y,$level,$mtype,$lastupdated ) = mysql_fetch_row( $result1 ))
	{	
?>
<tr>
<td><?php echo $x; ?></td>
<td><?php echo $y; ?></td>
<td><?php echo $level; ?></td>
<td><?php echo MonsterTypeToName($mtype); ?></td>
<td><?php echo GetTimeDiffShortFormat($lastupdated); ?></td>
</tr>
<?php
}
?>	
  </tbody>
</table>
<?php
function MonsterTypeToName($type)
{
	$ret = "";
	return $ret;
}
?>