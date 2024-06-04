<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

$MinX = -1700;
$MaxX = 1700;
$MinY = -1700;
$MaxY = 1700;
$MinSpeed = 150;
$MaxSpeed = 225;

// load existing state
$ExistingPeople = executeMySQLQuery("select * from DemoRadarState");

// advance existing states
foreach($ExistingPeople as $index => $pers)
{
	$tpers = AdvancePerson($pers);
	// make fields strings
	$tpers['ID'] = "".$tpers['ID'];
	$tpers['x'] = "".$tpers['x'];
	$tpers['y'] = "".$tpers['y'];
	$ExistingPeople[$index] = $tpers;
}

// save new states
foreach($ExistingPeople as $index => $pers)
{
	executeMySQLQuery("update DemoRadarState set 
	x=".$pers['x'].",
	y=".$pers['y'].",
	xs=".$pers['xs'].",
	ys=".$pers['ys']." where id=".$pers['ID']."");
}

//pack states to JSON
$myret['people']=$ExistingPeople;
echo json_encode($myret);
	
function AdvancePerson($pers)
{
	global $MinX,$MaxX,$MinY,$MaxY,$MinSpeed,$MaxSpeed;
	
	$pers['x'] += $pers['xs'];
	if($pers['x'] < $MinX || $pers['x'] == 0 || $pers['xs'] == 0)
	{
		$pers['xs'] = mt_rand($MinSpeed, $MaxSpeed);
	}
	if($pers['x'] > $MaxX)
	{
		$pers['xs'] = -mt_rand($MinSpeed, $MaxSpeed);
	}	
	
	$pers['y'] += $pers['ys'];
	if($pers['y'] < $MinX || $pers['y'] == 0 || $pers['ys'] == 0)
	{
		$pers['ys'] = mt_rand($MinSpeed, $MaxSpeed);
	}
	if($pers['y'] > $MaxX)
	{
		$pers['ys'] = -mt_rand($MinSpeed, $MaxSpeed);
	}	
	
	return $pers;
}

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
