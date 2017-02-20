<?php
include("db_connection.php");
$k = 67;
if(!isset($TrackWhat))
	$TrackWhat = "might";
if(!isset($YStep))
	$YStep = 40;
if(!isset($XStep))
	$XStep = 20;
if($TrackWhat == "guildless")
{
	$TrackWhat = "count(*)";
	$ExtraFilter = " and guild='none'";
}
if($TrackWhat == "guildless_innactive")
{
	$TrackWhat = "count(*)";
	$ExtraFilter = " and guild='none' and LastBurnedStamp<".time();
}
?>
<table>
	<tr>
<?php
	for( $x=0;$x<500;$x += $XStep)
	{
		?>
		<td><?php echo ($x-$XStep)."-".($x+$XStep); ?></td>
		<?php
	}
	?>
	</tr>
	<?php
		
	$MaxMight = 0;
	//prepare data
	for( $y=0;$y<1000;$y+=$YStep)
		for( $x=0;$x<500;$x += $XStep)
		{
			//fetch players in this cell
			$query1 = "select $TrackWhat from players where k=$k and x>=".($x-$XStep)." and x<=".($x+$XStep)." and y>=".($y-$YStep)." and y<=".($y+$YStep)."$ExtraFilter";		
			$result1 = mysql_query($query1,$dbi) or die("Error : 2017022004 <br>".$query1." <br> ".mysql_error($dbi));
			$MightSum[$x][$y] = 0;
			while( list( $might ) = mysql_fetch_row( $result1 ))
				$MightSum[$x][$y] += $might;
			if( $MightSum[$x][$y] >$MaxMight)
				$MaxMight = $MightSum[$x][$y];
		}

	for( $y=0;$y<1000;$y+=$YStep)
	{
		?>
		<tr>
			<td><?php echo ($y-$YStep)."-".($y+$YStep); ?></td>
		<?php
		for( $x=0;$x<500;$x += $XStep)
		{
			//fetch players in this cell
			$ColorPCT = 255 - (int)( 255 * $MightSum[$x][$y] / $MaxMight );
			?>
			<td style="background-color:rgb(<?php echo $ColorPCT; ?>,0,0)"><?php echo $MightSum[$x][$y]; ?></td>
			<?php
		}
		?>
		</tr>
		<?php
	}
?>
</table>