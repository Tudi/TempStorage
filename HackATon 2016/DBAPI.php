<?php
session_start();
include_once( "include/DBConnection.inc.php" );
include_once( "include/Functions.inc.php" );

// ex : http://pc371/Hack_A_Ton_2016_02_15/DBAPI.php?q=login&u=a&p=a
//	ret : SESSION_ID / NULL
if( isset( $q ) && stripos( "#login", $q ) == 1 && strlen( $u ) > 0 && strlen( $p ) )
{
	$ret = Login( $u, $p );
	if( $Error == "" )
		$ret = json_encode( array( $ret ) );
	echo $ret;
}

// ex : http://pc371/Hack_A_Ton_2016_02_15/DBAPI.php?q=ideas&GroupId=1
if( isset( $q ) && stripos( "#ideas", $q ) == 1 && $GroupId > 0 )
{
	$ret = GetIdeasList( $GroupId, $from, $to );
	$ret = json_encode( $ret );
	echo $ret;
}

// ex : http://pc371/Hack_A_Ton_2016_02_15/DBAPI.php?q=idea&id=7
if( isset( $q ) && stripos( "#idea", $q ) == 1 && $id > 0 )
{
	$ret = GetIdea( $id );
	$ret = json_encode( $ret );
	echo $ret;
}
?>