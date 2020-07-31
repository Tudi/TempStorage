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
{
	$IntervalString = " ".$StartDate->format('d/m/Y');
	//show remaining hours from today
	if($start == 0)
	{
		$today      = new DateTime('now');
		$tomorrow   = new DateTime('tomorrow');
		$difference = $today->diff($tomorrow);
		$IntervalString .= $difference->format('<br>Remaning time until tomorrow : <u><b>%h hours %i minutes </b></u>');
	}
}
else
	$IntervalString = "s ".$StartDate->format('d/m/Y')."-".$EndDate->format('d/m/Y');

$DaysInterval = -($end + $start) + 1;

//merge multiple rows to 1
if(isset($MergedList))
{
	unset($MergedList);
	$MergedList = NULL;
}

$lvl2coef = 4/6; //costs 6 times more, gives 4 times reward
$lvl3coef = (4*4)/(6*6); //costs 6 times more, gives 4 times reward
$lvl4coef = (4*4*4)/(6*6*6); //costs 6 times more, gives 4 times reward
$lvl5coef = (4*4*4*4)/(6*6*6*6); //costs 6 times more, gives 4 times reward
$query1 = "select PlayerName,lvl1,lvl2,lvl3,lvl4,lvl5,(lvl1+$lvl2coef*lvl2+$lvl3coef*lvl3+$lvl4coef*lvl4+$lvl5coef*lvl5) as playerscore from PlayerHunts where day>=$day+$start and day<=$day+$end and year=$year order by playerscore desc";
$result1 = mysqli_query($dbi, $query1) or die("Error : 2017022002 <br> ".$query1." <br> ".mysqli_error($dbi));
while(list($PlayerName,$lvl1,$lvl2,$lvl3,$lvl4,$lvl5,$score) = mysqli_fetch_row($result1))
{
	@$MergedList[$PlayerName][-1] = $PlayerName;
	@$MergedList[$PlayerName][0] += $score;
	@$MergedList[$PlayerName][1] += $lvl1;
	@$MergedList[$PlayerName][2] += $lvl2;
	@$MergedList[$PlayerName][3] += $lvl3;
	@$MergedList[$PlayerName][4] += $lvl4;
	@$MergedList[$PlayerName][5] += $lvl5;
}
$MergedList = OrderMergedList($MergedList);
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
	</tr>
	<?php
	foreach($MergedList as $Index => $Stats)
		{
			$lvl1 = $Stats[1];
			$lvl2 = $Stats[2];
			$lvl3 = $Stats[3];
			$lvl4 = $Stats[4];
			$lvl5 = $Stats[5];
			$PlayerName = $Stats[-1];
			$PassedVerification = 0;
			$BgColor = "";
			if($lvl1 >= 15 * $DaysInterval && $lvl2 >= 3 * $DaysInterval)
				$PassedVerification = 1;
			if($lvl2 >= 7 * $DaysInterval)
				$PassedVerification = 1;
			if($lvl3 >= 2 * $DaysInterval)
				$PassedVerification = 1;
			if($lvl4 >= 1 * $DaysInterval)
				$PassedVerification = 1;
			if($PassedVerification)
				$BgColor = "bgcolor=\"0x0000FF00\"";
			?>
			<tr <?php echo $BgColor; ?>>
				<td><?php echo $Index;?></td>
				<td><?php echo $PlayerName;?></td>
				<td><?php echo $lvl1;?></td>
				<td><?php echo $lvl2;?></td>
				<td><?php echo $lvl3;?></td>
				<td><?php echo $lvl4;?></td>
				<td><?php echo $lvl5;?></td>
			</tr>
			<?php
		}
	?>
</table>
