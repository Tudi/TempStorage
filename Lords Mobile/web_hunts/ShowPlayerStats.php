<?php
require_once("db_connection.php");
if(!isset($FilterPlayerName))
	die("Missing Player name");
if(SafeToExecuteOnMysql($FilterPlayerName)==0)
	die("ask a dev why this name is not accepted : $FilterPlayerName");
$FilterPlayerName = " and PlayerName='$FilterPlayerName'";
?>
<table border='1'>
	<tr>
		<td>Date</td>
		<td>Player Name</td>
		<td>lvl 1 kills</td>
		<td>lvl 2 kills</td>
		<td>lvl 3 kills</td>
		<td>lvl 4 kills</td>
		<td>lvl 5 kills</td>
	</tr>
<?php
for($i=0;$i>-20;$i--)
{
	$start=$i;
	$end=$i;
	include("ShowData.php");
}
?>
</table>