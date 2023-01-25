<?php
$LastErrorReason = "";

setUserIdFromAPIAccessToken();

function setUserIdFromAPIAccessToken()
{
	global $_REQUEST, $SESSION_USER_ID;
	$SESSION_USER_ID = 0;
	
	$APIAccessToken = "";
	if(isset($_REQUEST['APIAccessToken']))
	{
		$APIAccessToken = $_REQUEST['APIAccessToken'];
	}
	else if(isset($_SERVER['HTTP_APIACCESSTOKEN']))
	{
		$APIAccessToken = $_SERVER['HTTP_APIACCESSTOKEN'];
	}
	if(strlen($APIAccessToken)==0)
	{
		DebugPring("Missing access token : APIAccessToken");
		return;
	}
	$SESSION_USER_ID = getUserId($APIAccessToken);
	DebugPring("Assigned session ID ". $SESSION_USER_ID );
}
?>