<?php
function GenerateUniqueToken( $Salt = "" )
{
	return hash( 'tiger192,3', session_id().time().$Salt );
}
function PrintStatus()
{
	global $Error, $Success;
	if( $Error )
		echo $Error;
	else if( $Success )
		echo $Success;	
}
function IsLoggedIn()
{
	return $_SESSION['UserId'] > 0;
}
function CreateUser( $UserName, $UserPassword, $Email )
{
	global $Error,$Success,$dbi;
	$Error = "";
	$Success = "";
	$query = "select count(*) from users where email='".mysql_real_escape_string( $UserEmail )."' or Name='".mysql_real_escape_string( $UserName )."'";
	$result=mysql_query($query,$dbi) or die("201602091236".$query);
	list( $IsInUse ) = mysql_fetch_row($result);
	if( $IsInUse )
		$Error = "This username or email is already in use. Can not register account to it<br/>";
	else
	{
		//make sure we have a valid user / passw
		if( isset( $UserName ) && isset( $UserPassword ) && strlen( $UserName ) > 0 && strlen( $UserPassword ) > 0 )
		{
			$query = "insert into users ( name, password, Email ) values ( '".mysql_real_escape_string( $UserName )."', '".mysql_real_escape_string( $UserPassword )."','".mysql_real_escape_string( $UserEmail )."')";
			$result = mysql_query($query,$dbi) or die("201602091237".$query);
			$Success = "User created successfully. To verify email, click verification linky<br/>";
		}
		else
			$Error = "Can not register new user. Use another username / passw combination<br/>";
	}
	return Login( $UserName, $UserPassword );
}
function Login( $UserName, $UserPassword )
{
	global $Error,$Success,$dbi;
	$Error = "";
	$Success = "";
	$query = "select id from users where name='".mysql_real_escape_string( $UserName )."' and password='".mysql_real_escape_string( $UserPassword )."'";
	$result=mysql_query($query,$dbi) or die("201602091248".$query);
	list( $UserId ) = mysql_fetch_row($result);
	$_SESSION['UserId'] = $UserId;
	//update session table. Link session ID to user ID
	$ret = "";
	if( $UserId > 0 )
	{
		$ret = CreateSessionId( 1 );
		$Success = "User logged in succesfully<br>";
	}
	else
		$Error = "Wrong Username or Password<br>";
	return $ret;
}
function CreateSessionId( $UpdateDB = 0 )
{
	if( !isset( $_SESSION['UserToken'] ) )
		$_SESSION['UserToken'] = GenerateUniqueToken();
	if( $UpdateDB == 1 )
	{
		global $dbi;
		//
		$query = "select id from sessions where token='".mysql_real_escape_string( $_SESSION['UserToken'] )."' or userid=".$_SESSION['UserId']."";
		$result=mysql_query($query,$dbi) or die("201602091236".$query);
		list( $SessionId ) = mysql_fetch_row($result);
		if( $SessionId > 0 )
		{		
			//register in DB user token. Might be used to remember last viewed idea or last viewed comment
			$query = "update sessions set UserId=".$_SESSION['UserId'].", token='".mysql_real_escape_string( $_SESSION['UserToken'] )."' where token='".mysql_real_escape_string( $_SESSION['UserToken'] )."' or userid=".$_SESSION['UserId']."";
			$result = mysql_query($query,$dbi) or die("201602091456".$query);
		}
		else
		{
			//register in DB user token. Might be used to remember last viewed idea or last viewed comment
			$query = "insert into sessions ( Token, UserId ) values ('".mysql_real_escape_string( $_SESSION['UserToken'] )."', ".$_SESSION['UserId']." )";
			$result=mysql_query($query,$dbi) or die("201602091226".$query);

			$query = "select id from sessions where token='".mysql_real_escape_string( $_SESSION['UserToken'] )."'";
			$result=mysql_query($query,$dbi) or die("201602091236".$query);
			list( $SessionId ) = mysql_fetch_row($result);
		}
		$_SESSION['SessionId'] = $SessionId;
	}
	return $_SESSION['UserToken'];
}
function Alert( $Msg )
{
	include("alert.inc.php");	
}
function jumpto($jumpto)
{
	include("jumpto.inc.php");
}
function DeleteIdea( $id )
{
	global $dbi;
	$id = mysql_real_escape_string( $id );
	
	//get edit token
	$query = "select EditToken from ideas where id='$id'";
	$result=mysql_query($query,$dbi) or die("201602091236".$query);
	list( $EditToken ) = mysql_fetch_row($result);
	
	//delete token
	$query = "delete from usertokens where Token='$EditToken'";
	$result=mysql_query($query,$dbi) or die("201602151428".$query);
	
	//delete votes
	$query = "delete from uservotes where ideaid='$id'";
	$result=mysql_query($query,$dbi) or die("201602151429".$query);

	//delete comments
	$query = "delete from comments where ideaid='$id'";
	$result=mysql_query($query,$dbi) or die("201602151430".$query);

	//delete idea
	$query = "delete from ideas where id='$id'";
	$result=mysql_query($query,$dbi) or die("201602151431".$query);
}
function DeleteComment( $id )
{
	global $dbi;
	$id = mysql_real_escape_string( $id );
	
	//get edit token
	$query = "select EditToken from comments where id='$id'";
	$result=mysql_query($query,$dbi) or die("201602091511".$query);
	list( $EditToken ) = mysql_fetch_row($result);
	
	//delete token
	$query = "delete from usertokens where Token='$EditToken'";
	$result=mysql_query($query,$dbi) or die("201602091512".$query);
	
	//delete comments
	$query = "delete from comments where id='$id'";
	$result=mysql_query($query,$dbi) or die("201602091513".$query);
}
function GetIdeasList( $GroupId, $from = 0, $to = 0, $SearchString = "" )
{
	global $dbi;
	$query = "select * from ideas where GroupId='$GroupId'";
	if( isset( $SearchString ) && strlen( $SearchString ) > 0 )
		$query .= " and title like '%".mysql_real_escape_string( $SearchString )."%'";
	if( $from < $to && $from > 0 )
		$query .= " limit $from, $to";
	$result = mysql_query($query,$dbi) or die("201602101451".$query);
	$ret = "";
	$retCounter = 0;
	while( $row = mysql_fetch_assoc( $result ) )	
		$ret[ $retCounter++ ] = $row;
	return $ret;
}
function GetIdea( $Id )
{
	global $dbi;
	$query = "select * from ideas where id='$Id'";
	$result = mysql_query($query,$dbi) or die("201602101451".$query);
	$ret = mysql_fetch_assoc( $result );
	return $ret;
}
?>