<?php
error_reporting(E_ERROR | E_WARNING | E_PARSE);
set_time_limit( 2 * 60 );

$itemcount = 0;
$itemCatcount = 0;
$SlotSetup = array("helm","body","feet","mhand","ohand","trinket","trinket");

$f = fopen("GearsLM.txt","rt");
if($f)
{
    while (($line = fgets($f)) !== false) 
		if(strlen($line)>5)
	{
        $parts = explode(" ",$line);
		$item = array();
		$item["slot"] = $parts[0];
		$item["GearSet"] = $parts[1];
		$item["name"] = $parts[2];
		$item["chp"] = $item["rhp"] = $item["catk"] = $item["ratk"] = $item["cdef"] = $item["rdef"] = 0; // group notice warnings
		foreach($parts as $key => $val)
			if( $key > 2 )
			{
				if( $key % 2 == 0 )
					$IndName = $val;
				else
					$IndVal = $val;

				if( $key % 2 == 0 )
				{
					if( strcmp( $IndName, "hp" ) == 0 || $IndName[0] == 'h' )
					{
						$item["chp"] += $IndVal;
						$item["rhp"] += $IndVal;
					}					
					else if( strcmp( $IndName, "def" ) == 0 || $IndName[0] == 'd' )
					{
						$item["cdef"] += $IndVal;
						$item["rdef"] += $IndVal;
					}					
					else if( strcmp( $IndName, "atk" ) == 0 || $IndName[0] == 'a')
					{
						$item["catk"] += $IndVal;
						$item["ratk"] += $IndVal;
					}					
					else
						$item[$IndName] += $IndVal;
				}
			}
		$item = GetItemScore( $item );
		$items[ $itemcount++ ] = $item;
		$itemsCat[$item["slot"]][count($itemsCat[$item["slot"]])] = $item;
    }	
	fclose($f);
}
//print_r($items);
GenGearSet();

function IsParamImportant( $IndexName )
{
	// cavalry + ranged
//	$InterestedParams = array("chp","cdef","catk","ratk");
	// cavalry
//	$InterestedParams = array("chp","cdef","catk");
	// ranged
	$InterestedParams = array("ratk");
	foreach( $InterestedParams as $key => $val )
		if( strcmp( $val, $IndexName ) == 0 )
			return 1;
	return 0;
}

function GetParamGroup( $IndexName )
{
//	return $IndexName[0];
//	return $IndexName[0].$IndexName[1];
	return $IndexName;
}

function GetItemScore( $item )
{
	foreach($item as $key => $val)
		if( IsParamImportant( $key ) )
		{
			$g = GetParamGroup( $key );
			$item["ScroreGroups"][$g] += $val;
		}
	return $item;
}

function SetGearsetScore( $GearSet, $PrintInfo )
{
	if( count( $GearSet ) == 0 )
		return 0;	
	global $SlotSetup, $itemsCat;
	foreach( $GearSet as $key => $val )
	{
//echo $key." ".$val;
		$CurSlotName = $SlotSetup[ $key ];
		$CurSlotIndex = $val;
//echo $CurSlotName." ".$CurSlotIndex;
		$CurItem = $itemsCat[ $CurSlotName ][ $CurSlotIndex ];
		if( $PrintInfo )
			echo $key.")item name ".$CurItem["name"]." in slot ".$CurSlotName." from set ".$CurItem["GearSet"]."<br>";
		foreach( $CurItem["ScroreGroups"] as $key2 => $val2 )
			$GearSetScore[ $key2 ] += $val2;
	}
	//square each of the scores, than multiply each other
	$SumScore = 1;
	foreach( $GearSetScore as $key => $val )
	{
		$logs = sqrt( $val );
		$SumScore *= $logs;
		if( $PrintInfo )
			echo $key."=".$val.",";
//echo $key."=".$val."->".(int)$logs."=".(int)$SumScore.",";
	}
	if( $PrintInfo )
		echo "Sumscore=".(int)$SumScore."<br>";
	return $SumScore;
}

function IsCollectable( $GearSet )
{
	global $SlotSetup, $itemsCat;
	foreach( $GearSet as $key => $val )
	{
//echo $key." ".$val;
		$CurSlotName = $SlotSetup[ $key ];
		$CurSlotIndex = $val;
		$CurItem = $itemsCat[ $CurSlotName ][ $CurSlotIndex ];
//echo $CurSlotName." ".$CurSlotIndex." ".$CurItem["GearSet"][0];
		if( $CurItem["GearSet"][0] != '1' )
			return 0;
	}
	return 1;
}

function CountDifferentSets( $GearSet )
{
	global $SlotSetup, $itemsCat;
	$DiffSetCount = 0;
	foreach( $GearSet as $key => $val )
	{
//echo $key." ".$val;
		$CurSlotName = $SlotSetup[ $key ];
		$CurSlotIndex = $val;
//echo $CurSlotName." ".$CurSlotIndex;
		$CurItem = $itemsCat[ $CurSlotName ][ $CurSlotIndex ];
		if( $CurItem["GearSet"][0] == '1' || !isset( $unique[ $CurItem["GearSet"] ]) )
		{
			$unique[ $CurItem["GearSet"] ] = 1;
			$DiffSetCount++;
		}
	}
	return $DiffSetCount;
}

function GenGearSet()
{
	global $SlotSetup, $itemsCat;
	// we want to pick items for these slots
	$SlotCount = count($SlotSetup);
	for($PickIndex = 0; $PickIndex < $SlotCount; $PickIndex++ )
		$CurGearset[$PickIndex] = 0;
	
	//generate combinations
	$LoopCounter=0;
	$BestScore = 0;
	$BestCraftable = 0;
	$BestMultiSet = 0;
	while( isset($CurGearset[$SlotCount]) == false )
	{
		//get score for this setup
		$CurScore = SetGearsetScore($CurGearset,0);
		if( $CurScore >= $BestScore )
		{
			$BestScore = $CurScore;
			$BestScoreSet = $CurGearset;
		}
		if( $CurScore >= $BestCraftable && IsCollectable($CurGearset) )
		{
			$BestCraftable = $CurScore;
			$BestCraftableSet = $CurGearset;
			$BestCraftableSets[$CurScore] = $CurGearset;
		}
		if( $CurScore >= $BestMultiSet )
		{
			$Sets = CountDifferentSets( $CurGearset );
			if( $Sets >= $SlotCount )
			{
				$BestMultiSet = $CurScore;
				$BestMultiSetSet = $CurGearset;
			}
		}
		//gen next setup
		$Ind = 0;
		$CurGearset[$Ind]++;
		while($CurGearset[$Ind]>=count($itemsCat[$SlotSetup[$Ind]]) && $Ind < $SlotCount )
		{
			//reset this index
			$CurGearset[$Ind] = 0;
			//increase next index
			$Ind++;
			$CurGearset[$Ind]++;
		}
		$LoopCounter++;
//		if($LoopCounter>10)			break;
	}
	echo "<br>Best score : <br>";
	SetGearsetScore($BestScoreSet,1);
	echo "<br>Best collectable : <br>";
	SetGearsetScore($BestCraftableSet,1);
	echo "<br>Best multiset : <br>";
	SetGearsetScore($BestMultiSetSet,1);
	
	asort($BestCraftableSets);
	echo "<br>Best craftables : <br>";	
	$i=0;
	foreach($BestCraftableSets as $key => $val )
	{
		SetGearsetScore($BestCraftableSets[$key],1);
		$i++;
		if($i>=10) break;
	}
}
?>