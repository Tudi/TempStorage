<?php
ini_set('memory_limit','16M');

//open the file
$f = fopen( "codec.cpp", "rt" );
//read the whole file
$content = fread( $f, 1024*1024*4 );
//no need to keep the file open anymore
fclose( $f );

//force function ends with "return" to signal function end
$ContentLenght = strlen( $content );
$index = 0;
do{
	//try to get the next function start by searching return
	$loc = strpos( $content, "return ", $index );
	
//if( strpos( $content, "int Environment::FetchHWProfile(CString& profile)")	<= $loc && strpos( $content, "bool Environment::HWProfilesEqual(CString profileA, CString profileB)" ) >= $loc )
//	echo "!asdfjlashdklfjhasdkj^#()*!^$@()#*$&(k`";
	if( $loc > $index && $loc > 0 )
	{
		//check if this return is commented. Skip if so
		if( IsLineCommented( $content, $loc ) == 1 )
		{
			$index = $loc + 1;
//if( strpos( $content, "int Environment::FetchHWProfile(CString& profile)")	<= $loc && strpos( $content, "bool Environment::HWProfilesEqual(CString profileA, CString profileB)" ) >= $loc )
//	echo "!^#()*!^$@()#*$&(*";
			continue;
		}
		
		//replace "return" with auto
		//get the end of the line for this return
		$loc2 = strpos( $content, "\n", $loc+1 );
		$content1 = substr( $content, $loc, $loc2 - $loc );
		$content1 = str_replace( "return ", "{\n auto ThisIsMyLocalVar=", $content1 );
		$content = substr( $content, 0, $loc ).$content1.substr( $content, $loc2 );
		$loc2 = strpos( $content, "\n", $loc+3 );
		
		//insert debug code
		$content = substr( $content, 0, $loc2 )."\n ProfileLine( __FILE__, __FUNCTION__, __LINE__, \"End\", 2 );\n return ThisIsMyLocalVar;\n }".substr( $content, $loc2 );
		$loc2 = strpos( $content, "}", $loc2 ) + 2;
		$index = $loc2;
		$ContentLenght += 100;
	}
	else
		$index+=100;	//make sure we do not deadlock
}while($index<$ContentLenght && $loc > 0 );


//add normal function exit instrumentation. This may add double instrumentations in some case. It will be simply not executed
$ContentLenght = strlen( $content );
$index = 0;
do{
	//try to get the next function start by searching ::
	$loc = strpos( $content, "wxCodec::", $index );
	
	if( $loc > $index && IsLineCommented( $content, $loc ) == 1 )
	{
		$index = $loc + 1;
		continue;
	}
	
	//try to jump after the opening bracket {
	$loc = strpos( $content, "{", $loc );
	
	if( $loc > $index && IsLineCommented( $content, $loc ) == 1 )
	{
		$index = $loc + 1;
		continue;
	}
	
	//try to find the matching }
	$loc2 = $loc+2;
	$BracketsOpen = 1;
	$BracketsClosed = 0;
	do {
		$loc_close = strpos( $content, "}", $loc2 );
		$loc_open = strpos( $content, "{", $loc2 );
		if( $loc_open < $loc_close )
		{
			$loc2 = $loc_open + 2;
			$BracketsOpen++;
		}
		else
		{
			$loc2 = $loc_close + 2;
			$BracketsClosed++;
		}
	}while( $BracketsOpen > $BracketsClosed && $loc2 > 2 );

	if( $BracketsOpen == $BracketsClosed && $loc2 > 2 )
	{
		$loc = $loc2 - 2;
		//insert debug code
		$AddedContent = "\n      ProfileLine( __FILE__, __FUNCTION__, __LINE__, \"End\", 2 );\n";
		$content = substr( $content, 0, $loc ).$AddedContent.substr( $content, $loc );
		$loc = strpos( $content, ");", $loc ) + 2;
		$index = $loc;
//		$ContentLenght += strlen($AddedContent)+1;
		$ContentLenght += 10;
	}
	else
		$index+=100;	//make sure we do not deadlock
}while($index<$ContentLenght && $loc > 0 );

//do not change order. Try to find 1 matching begginning for each end
//get all the functions of the file
$ContentLenght = strlen( $content );
$index = 0;
do{
	//try to get the next function start by searching ::
	$loc = strpos( $content, "wxCodec::", $index );
	
	if( $loc > $index && ( IsLineCommented( $content, $loc ) == 1 || IsCodeAtMainLevel( $content, $loc ) == 0 ) )
	{
		$index = $loc + 1;
		continue;
	}
	
	//try to jump after the opening bracket {
	$loc = strpos( $content, "{", $loc );
	
	if( $loc > $index && IsLineCommented( $content, $loc ) == 1 )
	{
		$index = $loc + 1;
		continue;
	}
	
	
	//make our code pretty
	$loc = strpos( $content, "\n", $loc );
	
	if( $loc > $index )
	{	
		//insert debug code
		$content = substr( $content, 0, $loc )."\n      ProfileLine( __FILE__, __FUNCTION__, __LINE__, \"Start\", 1 );".substr( $content, $loc );
		$loc = strpos( $content, ");", $loc ) + 2;
		$index = $loc;
		$ContentLenght += 100;
	}
	else
		$index+=100;	//make sure we do not deadlock
}while($index<$ContentLenght && $loc > 0 );

$f = fopen( "codec2.cpp", "wt" );
//echo $content;
fwrite( $f, $content );
fclose( $f );

//check if this line is commented. If it is, skip processing it
function IsLineCommented( $content, $loc )
{
	$PrevLineEnd = strrpos( $content, "\n", -strlen($content) + $loc ) + 1;
	$CommentPos = strpos( $content, "//", $PrevLineEnd );
	if( $PrevLineEnd <= $CommentPos && $CommentPos <= $loc )
		return 1;
	//check if there is /* before us and */ after us
	$CommentStart = strrpos( $content, "/*", -strlen($content) + $loc ) + 1;
	$CommentEnd = strrpos( $content, "*/", -strlen($content) + $loc ) + 1;
	if($CommentEnd < $CommentStart)
		return 1;
	return 0;
}

//do not add start function code if we are already inside a function that has start function code
//pray that we do not have random non closed brackets in the file ( maybe in comments )
function IsCodeAtMainLevel( $content, $loc )
{
	$content = str_replace( "\t", " ", $content );
	//does it have strange characters before it ?
	$PrevLineEnd = strrpos( $content, "\n", -strlen($content) + $loc );
	$ThisLine = substr( $content, $PrevLineEnd, $loc - $PrevLineEnd );
	if( strpos( $ThisLine, "/" ) != 0 || strpos( $ThisLine, "(" ) != 0 || strpos( $ThisLine, ")" ) != 0 || strpos( $ThisLine, "=" ) != 0 || strpos( $ThisLine, "!" ) != 0 )
		return 0;
	
	//try to find a previous function = a } at the beggining of the line
	$PrevFuncLoc = 0;
	$t = strrpos( $content, "\"End\", 2 );", -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "int wxCodec::", -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "BOOL wxCodec::", -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "bool wxCodec::", -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "void wxCodec::", -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "CString wxCodec::", -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "wxString wxCodec::", -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "PxConvName wxCodec::", -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "inline unsigned short wxCodec::", -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "\n}", -strlen($content) + $loc ) + 2;
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$CodeSoFar = substr( $content, $PrevFuncLoc, $loc - $PrevFuncLoc );
	$NrOpenBrackets = strlen( $CodeSoFar) - strlen( str_replace( "{", "", $CodeSoFar ) );
	$NrCloseBrackets = strlen( $CodeSoFar) - strlen( str_replace( "}", "", $CodeSoFar ) );
//	echo " match for loc $PrevFuncLoc - $loc => $NrOpenBrackets == $NrCloseBrackets <br>";
	if( $NrOpenBrackets == $NrCloseBrackets )
		return 1;
	echo "$NrOpenBrackets != $NrCloseBrackets => ".substr( $content, $PrevLineEnd, strpos( $content, "\n", $loc) - $PrevLineEnd)." <br>";
	echo "$NrOpenBrackets != $NrCloseBrackets => $PrevFuncLoc - $loc ".substr( $content, $PrevFuncLoc, $loc - $PrevFuncLoc)." <br>";
	return 0;
}

?>