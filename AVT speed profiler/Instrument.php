<?php

$SkipThisFile = "Benchmarking.sj";

//InstrumentFile( "CommonTools.sj" );
//InstrumentFile( ".\\Input\\Empower\\Empower\\Script\\AVTUtilities\\UserInterface.sj" );
SearchInstrumentAllSJfiles( $SkipThisFile );

echo "All done";

function SearchInstrumentAllSJfiles( $SkipThisFile )
{
	$it = new RecursiveDirectoryIterator(".\\Input");
	foreach(new RecursiveIteratorIterator($it) as $file)
		if( strpos( $file, "$SkipThisFile") == 0 )
		{
			$extension = strtolower( substr($file, strrpos( $file, '.', -1) + 1 ) );
//echo $extension."\n".strcmp( strtolower($extension ), "sj" );
			if( strcmp( $extension, "sj" ) == 0 )
				InstrumentFile( $file );
		}	
}

function InstrumentFile( $fileName )
{
	$content = file_get_contents_UTF16( $fileName );
	$ASCIIContent = iconv("UTF-16LE", "ISO-8859-1//IGNORE", $content);
	if( strlen( $ASCIIContent ) <= 10 )
	{
		$ASCIIContent = $content; //file is already ascii ?
		$ContentWasASCII = 1;
	}
	if( strlen( $ASCIIContent ) > strlen( $content ) )
	{
		$ASCIIContent = iconv("UTF-16", "ISO-8859-1//IGNORE", $content);
		$ContentWasUTF16 = 1;
	}
//	else
//		$ASCIIContent = iconv("UTF-16", "ISO-8859-1//IGNORE", substr( $content, 4 ));	//cause of the stupid 6 chars at the beggining of the file
	//parse it line by line
	$ASCIIContent = str_replace( "){", ")\n{\n", $ASCIIContent );	//special 1 line functions...
	$ASCIIContent = str_replace( ";}", ";\n}\n", $ASCIIContent );	//special 1 line functions...
	if( strpos( $line, "){" ) )
		die( "WTFT in $fileName " );
	$lines = explode( "\n", $ASCIIContent );
	$MaxLineCount = count( $lines );
//	echo "File has $MaxLineCount lines : $ASCIIContent <br>";
	$NewFileContent = "";
	$LinesAdded = 2;
	foreach( $lines as $LineNumber => $line )
	{
//		$line = $lines[$LineNumber];
		if( IsStartOfFunction( $line ) )
		{
			$FuncName = GetFunctionName($line);
			if( $LineNumber < $InsideAFunctionUntil )
			{
				$InsideAFunctionInFunctionFrom = $LineNumber;
				$FunctionInsideFunctionUntil = GetEndOfFunctionPos( $lines, $MaxLineCount, $LineNumber + 1 );
//echo "Found function inside function for file $fileName - until $LineNumber - $FunctionInsideFunctionUntil + $LinesAdded - $line\n".$lines[$FunctionInsideFunctionUntil]."\n";
//echo "Found function inside function for file $fileName - until $LineNumber - $FunctionInsideFunctionUntil + $LinesAdded - $line\n";
//				for( $i=$LineNumber;$i<=$FunctionInsideFunctionUntil+2;$i++)				echo $lines[$i]."\n";
				$InfoStr3 = GetFileName( $fileName )." - ".$FuncName." - Line :";
				$SkipThisFunction = 0;
			}
			else
			{
//				echo "could be a new function on line $LineNumber : $line \n";
//				echo "function name could be : ".$FuncName."\n";
				$InsideAFunctionFrom = $LineNumber;
				$InsideAFunctionUntil = GetEndOfFunctionPos( $lines, $MaxLineCount, $LineNumber + 1 );
				$InfoStr2 = GetFileName( $fileName )." - ".$FuncName." - Line :";
//				$InfoStr = GetFileName( $fileName )." - ".$FuncName." - Line : ".$InsideAFunctionFrom." - ".($InsideAFunctionUntil+$LinesAdded);
//				echo "InfoStr = $InfoStr \n";
				$SkipThisFunction = 0;
			}
			//skip instrumenting this function
			if( strpos( $line, "GetScriptName" ) != 0 )
				$SkipThisFunction = 1;				
		}
		
		if( $LineNumber <= $InsideAFunctionUntil && $FunctionInsideFunctionUntil < $LineNumber + 1 && $SkipThisFunction == 0 )
			InstrumentOneLine( $InsideAFunctionFrom, $line, $LineNumber, $NewFileContent, $LinesAdded, $InfoStr2, $InsideAFunctionUntil );
		else if( $LineNumber <= $FunctionInsideFunctionUntil && $SkipThisFunction == 0 )
			InstrumentOneLine( $InsideAFunctionInFunctionFrom, $line, $LineNumber, $NewFileContent, $LinesAdded, $InfoStr3, $FunctionInsideFunctionUntil );
		else
			$NewFileContent .= $line."\n";
		
//		echo $line[1]."\n";
//		echo $line[3]."\n";
//		echo $line."\n";
	}
	$H1 = pack("S",0xfeff);
	$H2 = pack("S",0xfffe);
	$OH = substr( $content, 0, 2 );
//echo "$H1 - $OH - $fileName\n ";
	$ToWrite = $NewFileContent;
	if( $OH == $H1 )
	{
		$ToWrite = iconv("ISO-8859-1", "UTF-16LE", $NewFileContent);
		$NH = substr( $ToWrite, 0, 2 );
		if( $NH != $H1 )
			$ToWrite = $H1.$ToWrite;
	}
	else if( $OH == $H2 )
	{
		$ToWrite = iconv("ISO-8859-1", "UTF-16", "UTF8".$NewFileContent);
		$NH = substr( $ToWrite, 0, 2 );
		if( $NH != $H2 )
			$ToWrite = $H2.$ToWrite;
	}
	
	if( strlen( $NewFileContent ) < 100 )
		echo "File $fileName seems strangely small. Investigate it \n";

	file_force_contents( ".\\Out\\". str_replace( ".\\Input\\","", $fileName), $ToWrite );

	file_force_contents( ".\\Backup\\". str_replace( ".\\Input\\","", $fileName), $content );

//	if( strpos( $ASCIIContent, "IndicatorChangeInfo" ) != 0 )
//		file_force_contents( ".\\ForLucia\\". str_replace( ".\\Input\\","", $fileName), $content );
}

function InstrumentOneLine( &$InsideAFunctionFrom, &$line, &$LineNumber, &$NewFileContent, &$LinesAdded, &$InfoStr2, $LastLine )
{
	$SimpleAdd = 1;
	if($InsideAFunctionFrom + 1 == $LineNumber && trim($line)[0]=='{')
	{
		$NewFileContent .= $line."\n";
		$NewFileContent .= "  "."Runner.CallMethod(\"Benchmarking.LogTimeDiff\", \"Start : $InfoStr2 ".($LineNumber+$LinesAdded)." - ".($InsideAFunctionUntil+$LinesAdded)."\" );\n";
		$LinesAdded++;
		$SimpleAdd = 0;
	}
	$retpos = strpos($line, "return");
	$commentpos = 0;
	if( $retpos != 0 )
	{
		$commentpos = strpos( "#".$line, "//");
		if( $commentpos > $retpos )
			$commentpos = 0;
		//maybe it's inside a comment
		$BeforeReturn = substr( $line, 0, $retpos );
		$count = substr_count( $BeforeReturn, "\"" );
		if( $count % 2 == 1 )
			$commentpos = 1;
		//maybe it's a function inside a function -> example CustomFieldsVerification_Init()
	}
	if( $retpos != 0 && $commentpos == 0 )
	{
		$spacing = "";
		for($t=0;$t<$retpos;$t++)
			$spacing .= " ";
		//everything that is before the "return". It could be an "if" or an "else"
		if( strpos( trim( $line) , "return" ) != 0 )
		{
			$NewFileContent .= substr( $line, 0, strpos( $line, "return") )."\n";
			$LinesAdded++;					
		}
		$NewFileContent .= $spacing."{\n";
		$NewFileContent .= $spacing."  "."Runner.CallMethod(\"Benchmarking.LogTimeDiff\", \"End : $InfoStr2 ".($LineNumber+$LinesAdded)."\" );\n";
		$NewFileContent .= "  ".substr( $line, strpos( $line, "return") )."\n";
		$NewFileContent .= $spacing."}\n";
		$LinesAdded += 3;
		$SimpleAdd = 0;				
	}
	if( $SimpleAdd == 1 )
	{
		if( $LineNumber == $LastLine - 1 )
		{
			$NewFileContent .= $spacing."  "."Runner.CallMethod(\"Benchmarking.LogTimeDiff\", \"End : $InfoStr2 ".($LineNumber+$LinesAdded)."\" );\n";
			$LinesAdded += 1;				
		}
		$NewFileContent .= $line."\n";
	}	
}

function file_get_contents_UTF16( $filename )
{
	$handle = fopen($filename, "rb");
	$contents = fread($handle, filesize($filename));
	fclose($handle);
	return $contents;
}

function file_force_contents($dir, $contents)
{
//echo $dir."\n";	
	$parts = explode('\\', $dir);
	$file = array_pop($parts);
	$dir = '.';
	foreach($parts as $key => $part)
	if( $key > 0 )
	{
//		echo $dir."\n";
		if(!is_dir($dir .= "/$part"))
			mkdir($dir);
	}
	file_put_contents("$dir/$file", $contents);
}

function IsStartOfFunction( $line )
{
//	if( $line[0] == 'f' && strpos( "#".$line, "function") == 1 )	
//		return 1;
	$TrimLine = trim( $line );
	if( $TrimLine[0] == 'f' && strpos( "#".$TrimLine, "function") == 1 )
		return 1;
	return 0;
}

function GetEndOfFunctionPos( $lines, $MaxLines, $StartLine )
{
	$OpenBrackets = 0;
	$CloseBrackets = 0;
	for( $i = $StartLine; $i<$MaxLines; $i++ )
	{
		$OpenBrackets += substr_count( $lines[$i], "{" );
		$CloseBrackets += substr_count( $lines[$i], "}" );
//echo $lines[$i][0]."\n";
		if( $OpenBrackets == $CloseBrackets  )
		{
			if( trim( $lines[$i] )[0] == '}' )
				return $i + 1;
			if( IsStartOfFunction($lines[$i]) )
				return $StartLine;
		}
	}
	return $MaxLines;
}

function GetFileName( $path )
{
	$end = strlen( $path );
	$start = strrpos( $path, '\\', -1 ) + 1;
	return substr( $path, $start, $end - $start );
}
function GetFunctionName( $line )
{
	$end = strpos( $line, '(' );
	$start = strrpos( $line, ' ', $end - strlen( $line ) ) + 1;
	return substr( $line, $start, $end - $start );
}
?>