<?php
function Time_ms()
{
    $mt = explode(' ', microtime());
    return ((int)$mt[1]) * 1000 + ((int)round($mt[0] * 1000));
}

function defaultRegisterErrorHandler($cond, $action, $errMsg, $dbgMsg = "")
{
	global $LastErrorReason;
	if($cond == 0 || $cond == false)
	{
		return;
	}
	if($LastErrorReason == "")
	{
		$LastErrorReason = $errMsg;
	}
	if($dbgMsg != "" )
	{
		DebugPring($dbgMsg);
	}
	echo "{ \"$action\":\"False\", \"ErrorReason\":\"$LastErrorReason\"}";
	closeGlobalDBConnection();
	die();
}

function DebugPring($msg)
{
	global $_REQUEST, $ShowDebugMessages;
	if(isset($_REQUEST['ShowDebugMessages']) || (isset($ShowDebugMessages) && $ShowDebugMessages == 1))
	{
		echo $msg."<br>\n";
	}
}
?>