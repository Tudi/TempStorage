<?php
if(!isset($s_player))
{
	if(isset($_REQUEST['s_player']))
		$s_player=$_REQUEST['s_player'];
	else
		$s_player="";
}
if(isset($_REQUEST['ExactPlayerName']))
	$ExactPlayerName=$_REQUEST['ExactPlayerName'];

if(!isset($s_guild))
{
	if(isset($_REQUEST['s_guild']))
		$s_guild=$_REQUEST['s_guild'];
	else
		$s_guild="";
}
if(isset($_REQUEST['ExactGuildName']))
	$ExactGuildName=$_REQUEST['ExactGuildName'];

?>
<form name="SearchForm" id="SearchForm" action="">
	<table>
		<tr>
			<td>Player</td>
			<td><input type="text" name="s_player" value="<?php echo $s_player; ?>"></td>
			<td>Exact match<input type="checkbox" name="ExactPlayerName" <?php if(isset($ExactPlayerName)) echo "checked";?>></td>
			<td><input type="submit" value="Search"></td>
		</tr>
		<tr>
			<td>Guild</td>
			<td><input type="text" name="s_guild" value="<?php echo $s_guild; ?>"></td>
			<td>Exact match<input type="checkbox" name="ExactGuildName" <?php if(isset($ExactGuildName)) echo "checked";?>></td>
			<td><input type="submit" value="Search"></td>
		</tr>
	</table>
</form>
<?php
$ShowResults = "1";
if(strlen($s_player)>0)
{
	if(isset($ExactPlayerName))
		$ShowResults .= "&FN=".urlencode($s_player);
	else
		$ShowResults .= "&FNS=".urlencode($s_player);
}
if(strlen($s_guild)>0)
{
	if(isset($ExactGuildName))
		$ShowResults .= "&FG=".urlencode($s_guild);
	else
		$ShowResults .= "&FGS=".urlencode($s_guild);
}
if($ShowResults!="1")
{
//	echo "ShowResults=$ShowResults";
	?>
	<iframe src="players.php?<?php echo $ShowResults;?>" width="100%" height="100%" frameBorder="0"></iframe>
	<?php
}
?>
