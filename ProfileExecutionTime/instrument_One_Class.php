<?php
ini_set('memory_limit','16M');

//open the file
$FileName = "codec.cpp";
$ClassToSearch = "wxCodec::";
$ReturnTypes = array("int","bool","BOOL");

$f = fopen( $FileName, "rt" );
//read the whole file
$content = fread( $f, 1024*1024*4 );
//no need to keep the file open anymore
fclose( $f );

//do not change order. Try to find 1 matching begginning for each end
//get all the functions of the file
$ContentLenght = strlen( $content );
$index = 0;
do{
	//try to get the next function start by searching ::
	$loc = strpos( $content, $ClassToSearch, $index );
	
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
		$ContentToAdd="\n      AutoCloseFunctionProfiling autoProfileLine(__FILE__, __FUNCTION__, __LINE__, \"Start\", 1);\n";
		$content = substr( $content, 0, $loc ).$ContentToAdd.substr( $content, $loc );
		$loc = strpos( $content, ");", $loc ) + 2;
		$index = $loc;
		$ContentLenght += strlen($ContentToAdd);
	}
	else
		$index+=100;	//make sure we do not deadlock
}while($index<$ContentLenght && $loc > 0 );

$f = fopen( $FileName."2", "wt" );
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
	global $ClassToSearch;
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
	$t = strrpos( $content, "int ".$ClassToSearch, -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "BOOL ".$ClassToSearch, -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "bool ".$ClassToSearch, -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "void ".$ClassToSearch, -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "CString ".$ClassToSearch, -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "wxString ".$ClassToSearch, -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "PxConvName ".$ClassToSearch, -strlen($content) + $loc );
	if( $t > 0 && $t > $PrevFuncLoc )
		$PrevFuncLoc = $t;
	$t = strrpos( $content, "inline unsigned short ".$ClassToSearch, -strlen($content) + $loc );
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