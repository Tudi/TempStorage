<?php

// this only does anything if you set debug option in configs
if($err !== "")
{
	LogMsg($err);
}

// format reply 
if($errId != NO_ERR)
{
	$ret['ErrorId'] = $errId;
}

// If user is logged in, store activity
StoreUserEndpointActivity();

// actually send reply in the response body
if(isset($ret) && is_array($ret) && count($ret) > 0)
{
	$retString = json_encode($ret);
	echo $retString;
}

// Close the database connection
if( isset($conn) && $conn->connect_error === false)
{
	$conn->close();
	unset($conn);
}

?>