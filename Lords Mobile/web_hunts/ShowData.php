<?php
require_once("db_connection.php");
require_once("functions.php");

$year = GetYear();
$day = GetDayOfYear();

//get all players that hunted today
$ServerTimeCompensation = -2;
$StartDate = getDateFromDay($day+$start+$ServerTimeCompensation, $year);
$EndDate = getDateFromDay($day+$end+$ServerTimeCompensation, $year);
if($StartDate == $EndDate)
	$IntervalString = " ".$StartDate->format('d/m/Y');
else
	$IntervalString = "s ".$StartDate->format('d/m/Y')."-".$EndDate->format('d/m/Y');

$DaysInterval = $end - $start + 1;

//merge multiple rows to 1
if(isset($MergedList))
{
	unset($MergedList);
	$MergedList = NULL;
}
if(isset($MergedList1))
{
	unset($MergedList1);
	$MergedList1 = NULL;
}
if(isset($MergedList2))
{
	unset($MergedList2);
	$MergedList2 = NULL;
}
//get the list from the popup events
{
	$query1 = "select PlayerName,lvl1,lvl2,lvl3,lvl4,lvl5 from PlayerHunts where day>=$day+$start and day<=$day+$end and year=$year";
	$result1 = mysqli_query($dbi, $query1) or die("Error : 2017022002 <br> ".$query1." <br> ".mysqli_error($dbi));
	while(list($PlayerName,$lvl1,$lvl2,$lvl3,$lvl4,$lvl5) = mysqli_fetch_row($result1))
	{
		@$MergedList1[$PlayerName][-1] = $PlayerName;
		@$MergedList1[$PlayerName][1] += $lvl1;
		@$MergedList1[$PlayerName][2] += $lvl2;
		@$MergedList1[$PlayerName][3] += $lvl3;
		@$MergedList1[$PlayerName][4] += $lvl4;
		@$MergedList1[$PlayerName][5] += $lvl5;
	}
}
//get the list from opening gifts
{
	$query1 = "select PlayerName,lvl from PlayerHuntsList where day>=$day+$start and day<=$day+$end and year=$year";
	$result1 = mysqli_query($dbi, $query1) or die("Error : 2017022002 <br> ".$query1." <br> ".mysqli_error($dbi));
	while(list($PlayerName,$lvl) = mysqli_fetch_row($result1))
	{
		@$MergedList2[$PlayerName][-1] = $PlayerName;
		@$MergedList2[$PlayerName][$lvl] += 1;
	}
}
//merge the 2 lists
{
	if( isset($MergedList1) && $MergedList1 != NULL )
	foreach($MergedList1 as $PlayerName => $score)
	{
		@$MergedList[$PlayerName][-1] = $PlayerName;
		for($lvl=1;$lvl<=5;$lvl++)
			@$MergedList[$PlayerName][$lvl] = max($MergedList1[$PlayerName][$lvl],$MergedList2[$PlayerName][$lvl]);
	}
	if( isset($MergedList2) && $MergedList2 != NULL )
	foreach($MergedList2 as $PlayerName => $score)
	{
		@$MergedList[$PlayerName][-1] = $PlayerName;
		for($lvl=1;$lvl<=5;$lvl++)
			@$MergedList[$PlayerName][$lvl] = max($MergedList1[$PlayerName][$lvl],$MergedList2[$PlayerName][$lvl]);
	}
	if(!isset($MergedList) || $MergedList == NULL)
		$MergedList = array();
	//calc score
	$lvlCoeff[1] = 1;
	$lvlCoeff[2] = 0.56; //costs 6 times more, gives 4 times reward
	$lvlCoeff[3] = 0.42; //costs 6 times more, gives 4 times reward
	$lvlCoeff[4] = 0.29; //costs 6 times more, gives 4 times reward
	$lvlCoeff[5] = 0.14; //costs 6 times more, gives 4 times reward
	foreach($MergedList as $PlayerName => $score)
	{
		@$MergedList[$PlayerName][0] = 0;
		for($lvl=1;$lvl<=5;$lvl++)
			$MergedList[$PlayerName][0] += $MergedList[$PlayerName][$lvl] * $lvlCoeff[$lvl];
	}
	//get total kills
	for($lvl=0;$lvl<=5;$lvl++)
		$TTKills[$lvl]=0;
	foreach($MergedList as $PlayerName => $score)
	{
		for($lvl=1;$lvl<=5;$lvl++)
		{
			@$TTKills[0] += $MergedList[$PlayerName][$lvl];
			@$TTKills[$lvl] += $MergedList[$PlayerName][$lvl];
		}
	}
	$MergedList = OrderMergedList($MergedList);
}
?>
Hunts Made on day<?php echo $IntervalString; ?><br />
<table border='1'>
	<tr>
		<td>Rank</td>
		<td>Player Name</td>
		<td>lvl 1 kills</td>
		<td>lvl 2 kills</td>
		<td>lvl 3 kills</td>
		<td>lvl 4 kills</td>
		<td>lvl 5 kills</td>
		<?php if($start!=$end) echo "<td>Days worth of hunts</td>"; ?>
	</tr>
	<?php
	$TotalDaysWorthOfHunts = 0;
	if(isset($MergedList[0]))
	foreach($MergedList as $Index => $Stats)
		{
			$PlayerName = $Stats[-1];
			$BgColor = "";
			$DaysWorthOfHunts = CalcNumberOfDaysWorthOfHunts($Stats);
			if($DaysWorthOfHunts >= $DaysInterval)
				$BgColor = "bgcolor=\"0x0000FF00\"";
			$TotalDaysWorthOfHunts += $DaysWorthOfHunts;			
			?>
			<tr <?php echo $BgColor; ?>>
				<td><?php echo $Index;?></td>
				<td><?php echo $PlayerName;?></td>
				<td style="text-align: center;"><?php echo $Stats[1];?></td>
				<td style="text-align: center;"><?php echo $Stats[2];?></td>
				<td style="text-align: center;"><?php echo $Stats[3];?></td>
				<td style="text-align: center;"><?php echo $Stats[4];?></td>
				<td style="text-align: center;"><?php echo $Stats[5];?></td>
				<?php if($start!=$end) echo "<td style=\"text-align: center;\">$DaysWorthOfHunts</td>"; ?>
			</tr>
			<?php
		}
	?>
	<?php
	//get a list of distinct names that hunted past month and print those that did not yet hunt
//	if( $start==0 && $end==0)
	{
		$query1 = "select distinct(PlayerName) from PlayerHunts where day>=($day-31) and year=$year";
		$result1 = mysqli_query($dbi, $query1) or die("Error : 2017022002 <br> ".$query1." <br> ".mysqli_error($dbi));
		while(list($PlayerName1) = mysqli_fetch_row($result1))
		{
			//check if the name is already printed as hunted
			$AlreadyHunted = 0;
			if(isset($MergedList[0]))
			foreach($MergedList as $key => $score)
				if( strcmp($PlayerName1,$score[-1])==0 )
				{
//					echo "equals=$PlayerName1,${score[-1]} ";
					$AlreadyHunted = 1;
					break 1;
				}
			if($AlreadyHunted == 1)
				continue;
			
			?>
			<tr bgcolor="#FFAAAA">
				<td></td>
				<td><?php echo $PlayerName1;?></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
			</tr>
			<?php
		}
	}
	?>
	<tr>
		<td><?php echo $TTKills[0];?></td>
		<td>Total Kills</td>
		<td><?php echo @$TTKills[1];?></td>
		<td><?php echo @$TTKills[2];?></td>
		<td><?php echo @$TTKills[3];?></td>
		<td><?php echo @$TTKills[4];?></td>
		<td><?php echo @$TTKills[5];?></td>
		<?php if($start!=$end) echo "<td>$TotalDaysWorthOfHunts</td>"; ?>
	</tr>	
</table>
