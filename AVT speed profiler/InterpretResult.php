<?php
ini_set('memory_limit','1G');

//test complete 9
//ParseFile( ".\\results\\result.txt" );
//ParseFile( ".\\results\\AVTBenchmark_05_05_16_17_12_50.txt" );
//ParseFile( ".\\results\\AVTBenchmark_05_06_16_09_37_07.txt" );
//ParseFile( ".\\results\\AVTBenchmark_05_09_16_09_33_42.txt" );
//ParseFile( ".\\results\\AVTBenchmark_05_09_16_13_44_43.txt" );
//ParseFile( ".\\results\\AVTBenchmark_05_09_16_14_23_58.txt" );

// test complete 11
//ParseFile( ".\\results\\AVTBenchmark_05_09_16_17_32_41.txt" );
//ParseFile( ".\\results\\AVTBenchmark_05_10_16_09_50_25.txt" );	// 26 minutes run
//ParseFile( ".\\results\\AVTBenchmark_05_10_16_12_37_27.txt" );	// 15.75975 minutes run
//ParseFile( ".\\results\\AVTBenchmark_05_10_16_17_04_32.txt" );	// 13.3503 minutes run
//ParseFile( ".\\results\\AVTBenchmark_05_10_16_17_47_20.txt" );		// 11.47361 minutes run
//ParseFile( ".\\results\\AVTBenchmark_05_13_16_17_24_49.txt" );		// 11.47361 minutes run
//ParseFile( ".\\results\\AVTBenchmark_05_31_16_14_29_28_event_wait_DLL_calls_dictionary.txt" );		// 8.39 minutes run
//ParseFile( ".\\results\\AVTBenchmark_06_01_16_11_40_23_event_wait_DLL_calls_dictionary_lang_settext.txt" );		// 8.06 minutes run
//ParseFile( ".\\results\\AVTBenchmark_06_01_16_12_16_01_event_wait_DLL_calls_dictionary_lang_findchild.txt" );		// 6.53 minutes run
//ParseFile( ".\\results\\AVTBenchmark_06_01_16_12_32_51_event_wait_DLL_calls_dictionary_lang_findchild_findmethod.txt" );		// 6.60 minutes run
//ParseFile( ".\\results\\AVTBenchmark_06_01_16_15_53_22_event_wait_DLL_calls_dictionary_lang_findchild_findmethod.txt" );		// 
/*{
	// logs with findid + GUI comp optimizations : 5526179 = 92 mins
	ParseFile( ".\\results\\AVTBenchmark_06_10_16_12_25_12_setup.txt" );		
	ParseFile( ".\\results\\AVTBenchmark_06_10_16_11_11_43_suit.txt" );		
	ParseFile( ".\\results\\AVTBenchmark_06_10_16_10_37_32_cust_fields.txt" );		
	ParseFile( ".\\results\\AVTBenchmark_06_10_16_10_25_02_suit_unk.txt" );		
}/**/
/*{
	// logs with inputfiledriver only optimizations : 5526179 = 92 mins
	ParseFile( ".\\results\\AVTBenchmark_06_10_16_15_24_57_cust_fields.txt" );		
	ParseFile( ".\\results\\AVTBenchmark_06_10_16_15_40_54_suit_unk.txt" );		
}/**/
/*{
	// logs with findid + GUI comp optimizations array + inputfiledriver optimizations : 
	ParseFile( ".\\results\\AVTBenchmark_06_10_16_16_57_24_cust_fields.txt" );		
	ParseFile( ".\\results\\AVTBenchmark_06_10_16_16_44_43_suit_unk.txt" );		
}/**/
/*{
	// logs with findid array + GUI comp optimizations array + inputfiledriver + GetXmlObjectByTag optimizations : 
	ParseFile( ".\\results\\AVTBenchmark_06_13_16_12_29_33_unk_fields.txt" );		
	ParseFile( ".\\results\\AVTBenchmark_06_13_16_12_17_00_suit_unk.txt" );		
	ParseFile( ".\\results\\AVTBenchmark_06_13_16_13_47_46_suit_cal.txt" );		
	ParseFile( ".\\results\\AVTBenchmark_06_13_16_12_48_58_suit_ver.txt" );		
}/**/
	ParseFile( ".\\results\\AVTBenchmark_06_13_16_12_29_33_unk_fields.txt" );		

InterpretResults();

function ParseFile($name)
{
	global $CallTree, $Results, $TotalScriptTime, $LogFilesParsed;
	$LogFilesParsed++;
	$content = file_get_contents( $name );
//	$ASCIIContent = iconv("UTF-16", "ISO-8859-1//IGNORE", $content);
	$ASCIIContent = $content;
//echo $ASCIIContent;
	$lines = explode( "\n", $ASCIIContent );
	$FirstTime = 0;
	$LastTime = 0;
	foreach( $lines as $LineNumber => $line )
	{
		$time = (int)trim( substr( $line, 0, strpos( $line, ":" ) ) );
		if( $time < $FirstTime )
			$FirstTime = $time;
		if( $time > $LastTime )
			$LastTime = $time;
		$startpos = strpos( $line, ": Start :" );
		if( $startpos > 0 )
		{
			$startpos += strlen(": Start :") + 1;
			$CutEnd = strpos( $line, " - Line : " );
			$funcName = substr( $line, $startpos, $CutEnd - $startpos);
			$tResults[$funcName]["start"] = $time;
			$UniqueFunctions[$funcName] = 1;
			$CallTree = UpdateCallTree( $funcName, $UniqueFunctions, $CallTree, $CallTreeInside );
			if( $CallTreeInside[$funcName] == 1 && !isset($ReportedRecursiveCalls[$funcName]) )
			{
				$ReportedRecursiveCalls[$funcName] = 1;
				echo "!!Alert $funcName - $line : recursive function calls will mess up function times. \n";
			}
			$CallTreeInside[$funcName] = 1;
		}
		else if( $endpos = strpos( $line, ": End :" ) )
		{
			$endpos += strlen(": End :") + 1;
			$CutEnd = strpos( $line, " - Line : " );
			$funcName = substr( $line, $endpos, $CutEnd - $endpos);
//			$tResults[$funcName]["end"] = $time;
			if( isset( $tResults[$funcName]["start"] ) && $CallTreeInside[$funcName] == 1 )
				$diff = $time - $tResults[$funcName]["start"];
			else 
			{
				$diff = 0;
				if( !isset($ReportedRecursiveCalls[$funcName]) )
				{
					$ReportedRecursiveCalls[$funcName] = 1;
					echo "!!Alert $funcName - $line : missing Start timer. \n";
				}
			}
if( "Custom_Fields_Verification.sj - CustomFieldsVerification_Init" == $funcName )
	echo $tResults[$funcName]["start"]."->$time = $diff\n";
			$Results[$funcName]['dur'] += $diff;
			$Results[$funcName]['count']++;
			if( $diff < $Results[$funcName]['mindif'] || !isset( $Results[$funcName]['mindif'] ) )
				$Results[$funcName]['mindif'] = $diff;
			$CallTreeInside[$funcName] = 0;
		}
//echo $time." ".$funcName."\n";		
	}
	$TotalScriptTime += $LastTime - $FirstTime;
	echo "Script total runtime : $TotalScriptTime - $name\n";
}

function InterpretResults()
{
	global $CallTree, $Results, $TotalScriptTime;
//	var_dump( $Results );
	foreach( $Results as $FuncName => $val )
		$Results[ $FuncName ]['timeshare'] = $Results[$FuncName]['dur'] * 100 / $TotalScriptTime;
	
	//create a calltree 2 with times also
	foreach( $CallTree as $fname1 => $fnamelist )
	{
		$CallTree[$fname1]['total_precise'] = $Results[$fname1]['dur'];
		foreach( $fnamelist as $fname2 => $callcount)
		{
//echo "$fname1 -> $fname2 ($callcount)".$Results[$fname2]['count']."\n";
			$CallCountTotal = $Results[$fname2]['count'];
			if( $CallCountTotal == 0 )
			{
				$AvgCallTimeSubfunc = 0;
//				var_dump( $fnamelist );
//				var_dump( $Results[$fname2]) ;
			}
			else
				$AvgCallTimeSubfunc = $Results[$fname2]['dur'] / $CallCountTotal * $callcount['count'];
			$CallTree[$fname1]['total_calculated'] += $AvgCallTimeSubfunc;
			$CallTree[$fname1][$fname2]['time'] = $AvgCallTimeSubfunc;
		}
	}
	uasort( $CallTree, CallTreeCompare );
	foreach( $CallTree as $fname1 => $fnamelist )
		uasort( $CallTree[$fname1], CallTreeCompareSubcall );
	
/*	
	//keep replacing functions that we know their time
	$FoundUnknownFunctionTime = 1;
	while( $FoundUnknownFunctionTime == 1 )
	{
		$FoundUnknownFunctionTime = 0;
		
		foreach( $UniqueFunctions as $key => $val )
			if( !isset( $FuncRawAvgCallTimes[$key] ) )
			{
				$KnowAllSubTreeTimes = 1;
				$SumSubFuncTimes = 0;
				$FunctionCallCount = $Results[$key]['count'];
				if( isset( $CallTree[$key] ) )
					foreach( $CallTree[$key] as $key2 => $val2 )
	//					if( strcmp( $key2, "inside" ) != 0 )
						{
							if( !isset( $FuncRawAvgCallTimes[$key2] ) )
							{
								$KnowAllSubTreeTimes = 0;
								break;
							}
							else
							{
								$SumSubFuncTimes += ( $FuncRawAvgCallTimes[$key2] * $FunctionCallCount );
							}
						}
				if( $KnowAllSubTreeTimes == 1 )
				{
					$AvgDuration = $Results[$key]['dur'] / $Results[$key]['count'];
					if( $Results[$key]['mindif'] <= 50 )
						$AvgDuration = 0.00001;
					$FuncRawCallTimes[$key] = $Results[$key]['dur'] - $SumSubFuncTimes;
					$FuncRawAvgCallTimes[$key] = $AvgDuration;
					$FoundUnknownFunctionTime = 1;
				}
			}
//		break;
	}

	//get timeshare for functions
	$SumOfTimes = 0;
	foreach( $FuncRawCallTimes as $key => $val )
		$SumOfTimes += $val;
		
	foreach( $FuncRawCallTimes as $key => $val )
		$TimeShare[$key] = $FuncRawCallTimes[$key] * 100 / $SumOfTimes;
	arsort( $TimeShare );

	arsort( $FuncRawCallTimes );
	arsort( $FuncRawAvgCallTimes );
	
//	var_dump( $TimeShare );
//	echo "Dumping function times without subcalls<br>";
//	var_dump( $FuncRawCallTimes );
//	echo "Dumping function times without subcalls avg <br>";
//	var_dump( $FuncRawAvgCallTimes );
*/

	uasort( $Results, CallCompareFuncTimes );
	echo "Dumping function times <br>";
	var_dump( $Results );
	
	echo "Dumping calltree <br>";
	var_dump( $CallTree );
}

function CallCompareFuncTimes( $a, $b )
{
/*	
	echo "lofassz";
	var_dump( $a );
	var_dump( $b );
	die("aaargh");
*/
	return ($a['dur'] < $b['dur']);
}

function CallTreeCompareSubcall( $a, $b )
{
/*	
	echo "lofassz";
	var_dump( $a );
	var_dump( $b );
	die("aaargh");
*/
	return ($a['time'] < $b['time']);
}

function CallTreeCompare( $a, $b )
{	
	return ($a['total_precise'] < $b['total_precise']);
}

function UpdateCallTree( $funcName, $UniqueFunctions, $CallTree, $CallTreeInside )
{
	//mark all running functions that they are using this function also
	foreach( $UniqueFunctions as $key => $val )
		if( $CallTreeInside[$key] == 1 
//			&& strcmp( $key, $funcName ) != 0 
			)
		{
			$CallTree[$key][$funcName]['count']++; //owner function called this function N times. Marking it once would be enough for us
		}
	return $CallTree;
}
?>