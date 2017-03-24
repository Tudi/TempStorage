<?php
if(!isset($dbi))
	include("db_connection.php");

ob_start();

$ShowAvgVal = 0;
if(!isset($k))
	$k = 67;
if(!isset($YStep))
	$YStep = 40;
if(!isset($XStep))
	$XStep = 20;
$ExtraFilter="";
$FromWhere="players";
if(!isset($TrackWhat))
	$TrackWhat = "might";
if($TrackWhat == "might")
	$SelectWhat = "might";
if($TrackWhat == "kills")
	$SelectWhat = "kills";
if($TrackWhat == "pcount")
	$SelectWhat = "count(*)";
if($TrackWhat == "castlelevel")
{
	$SelectWhat = "castlelevel";
	$ShowAvgVal = 1;
}
if($TrackWhat == "guildless")
{
	$SelectWhat = "count(*)";
	$ExtraFilter = " and (guild='' or isnull(guild))";
}
if($TrackWhat == "guildless_innactive")
{
	echo "Players who's might did not change in the past X days<br>. Only works if it has recent data updated. Check manually !";
	$SelectWhat = "count(*)";
	$ExtraFilter = " and (guild='' or isnull(guild)) and ( statusflags & 0x01000000) <> 0";
}
if($TrackWhat == "resourcelevel")
{
	$FromWhere="resource_nodes";
	$SelectWhat = "level";
	$ExtraFilter = "";
	$ShowAvgVal = 1;
}
if($TrackWhat == "resourcefree")
{
	$FromWhere="resource_nodes";
	$SelectWhat = "1";
	$ExtraFilter = " and (isnull(playername) or playername='')";
}
$MaxX = 510;
$MaxY = 1030;
?>
<table>
	<tr>
		<td></td>
<?php
	for( $x=0;$x<$MaxX;$x += $XStep)
	{
		$from = ($x);
		if($from<0)
			$from=0;
		?>
		<td><?php echo $from."-".($x+$XStep); ?></td>
		<?php
	}
	?>
	</tr>
	<?php
		
	$MaxMight = 0;
	//prepare data
	for( $y=0;$y<$MaxY;$y+=$YStep)
		for( $x=0;$x<$MaxX;$x += $XStep)
		{
			//fetch players in this cell
			$MightSum[$x][$y] = 0;
			$MightCount[$x][$y] = 0;
			$query1 = "select $SelectWhat from $FromWhere where x>=".($x)." and x<=".($x+$XStep)." and y>=".($y)." and y<=".($y+$YStep)."$ExtraFilter";		
			$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
			while( list( $might ) = mysql_fetch_row( $result1 ))
			{
				$MightSum[$x][$y] += $might;
				$MightCount[$x][$y]++;
			}
			if( $MightSum[$x][$y] >$MaxMight)
				$MaxMight = $MightSum[$x][$y];
		}

	for( $y=0;$y<$MaxY;$y+=$YStep)
	{
		$from = ($y);
		if($from<0)
			$from=0;
		?>
		<tr>
			<td><?php echo $from."-".($y+$YStep); ?></td>
		<?php
		for( $x=0;$x<$MaxX;$x += $XStep)
		{
			//fetch players in this cell
			if($MaxMight>0)
				$ColorPCT = 255 - (int)( 255 * $MightSum[$x][$y] / $MaxMight );
			else
				$ColorPCT = 255;
			$val = $MightSum[$x][$y];
			if($ShowAvgVal == 1 && $MightCount[$x][$y] > 0 )
				$val = (int)( $val / $MightCount[$x][$y] * 10 ) / 10;
			?>
			<td style="background-color:rgb(<?php echo $ColorPCT; ?>,0,0)"><?php echo GetValShortFormat($val); ?></td>
			<?php
		}
		?>
		</tr>
		<?php
	}
?>
</table>
<?php
$StaticFileContent = ob_get_contents();
ob_end_clean();
//dump page content to file
$f = fopen("${TrackWhat}_$k.html","wt");
if($f)
{
	fwrite($f,$StaticFileContent);
	fclose($f);
	$f = 0;
}
else
	echo "$StaticFileContent";
?>