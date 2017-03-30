<?php
session_start();
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
if(!isset($s_grank))
{
	if(isset($_REQUEST['s_grank']))
		$s_grank=$_REQUEST['s_grank'];
	else
		$s_grank="";
}
if(isset($_REQUEST['s_innactive']))
	$s_innactive=$_REQUEST['s_innactive'];
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
		<tr>
			<td>Guild rank</td>
			<td><select name="s_grank">
					<option value="">Any</option>
					<option value="6" <?php if($s_grank==6) echo "selected";?> >Guildless</option>
					<option value="1" <?php if($s_grank==1) echo "selected";?> >R1</option>
					<option value="2" <?php if($s_grank==2) echo "selected";?> >R2</option>
					<option value="3" <?php if($s_grank==3) echo "selected";?> >R3</option>
					<option value="4" <?php if($s_grank==4) echo "selected";?> >R4</option>
					<option value="5" <?php if($s_grank==5) echo "selected";?> >Owner</option>
				</select></td>
			<td></td>
			<td><input type="submit" value="Search"></td>
		</tr>
		<tr>
			<td>Innactive</td>
			<td><input type="checkbox" name="s_innactive" <?php if(isset($s_innactive)) echo "checked";?>></td>
			<td></td>
			<td><input type="submit" value="Search"></td>
		</tr>
	</table>
</form>
<?php
$ShowResults = "1";
if(strlen($s_player)>0)
{
	if(isset($ExactPlayerName))
	{
		$FN=$s_player;
		$ShowResults .= "&FN=".urlencode($s_player);
	}
	else
	{
		$FNS=$s_player;
		$ShowResults .= "&FNS=".urlencode($s_player);
	}
}
if(strlen($s_guild)>0)
{
	if(isset($ExactGuildName))
	{
		$FG = $s_guild;
		$ShowResults .= "&FG=".urlencode($s_guild);
	}
	else
	{
		$FGS = $s_guild;
		$ShowResults .= "&FGS=".urlencode($s_guild);
	}
}
if(isset($s_grank) && $s_grank != "" && $s_grank<6)
{
	$FGR = $s_grank;
	$ShowResults .= "&FGR=".urlencode($s_grank);
}
if(isset($s_innactive) && $s_innactive != "" )
{
	$FI=1;
	$ShowResults .= "&FI=1";
}
if($ShowResults!="1")
{
//	echo "ShowResults=$ShowResults";
	$PlayersPhpIncluded = 1;
	include("players.php");
}
?>
