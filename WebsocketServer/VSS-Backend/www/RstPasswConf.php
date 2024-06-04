<?php
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
CheckRequiredFields($_GET,array("id","cid","uid"));

if($errId === NO_ERR)
{
    // Get the user's email address from the form
    $reqId = $_GET["id"];
    $secId = $_GET["cid"];
    $tUserId = $_GET["uid"];
}

if($errId === NO_ERR)
{
	// check if this user exists
	$rows = executeMySQLQuery("Select UserId,CallbackId,NewPassw,IsConsumed,UNIX_TIMESTAMP(ReqCreatedTimestamp) as ReqCreatedTimestamp 
		from PsswRstQueue where ResetID=? and IsConsumed=0 order by ReqCreatedTimestamp desc limit 1 ", $reqId);
	if(isset($rows[0]['UserId']))
	{	
		// create a queue entry
		$UserId = (int)$rows[0]['UserId'];
		$CallbackId = $rows[0]['CallbackId'];
		$NewPassw = $rows[0]['NewPassw'];
		$IsConsumed = $rows[0]['IsConsumed'];
		$ReqCreatedTimestamp = $rows[0]['ReqCreatedTimestamp'];
//		print_r($rows);
	}
	else
	{
		$err = "Reset request no longer available";
		$errId = INVALID_SESSION;
		$UserId = "";
		$CallbackId = "";
		$NewPassw = "";
		$IsConsumed = "";
		$ReqCreatedTimestamp = "";
	}
}

if($errId === NO_ERR)
{
	if($tUserId != $UserId)
	{	
		$err = "User ID mismatch : $tUserId != $UserId";
		$errId = INVALID_USER;
	}
}

if($errId === NO_ERR)
{
	if($CallbackId !== $secId)
	{	
		$err = "Callback ID mismatch";
		$errId = INVALID_SESSION;
	}
}

if($errId === NO_ERR)
{
	if($IsConsumed !== 0)
	{	
		$err = "Already reset";
		$errId = INVALID_SESSION;
	}
}

if($errId === NO_ERR)
{
	if($ReqCreatedTimestamp + const_passwResetLinkAvailableSec < time())
	{	
		$err = "Reset expired : $ReqCreatedTimestamp ".($ReqCreatedTimestamp + const_passwResetLinkAvailableSec)."< ".time();
		$errId = INVALID_SESSION;
	}
}

if($errId === NO_ERR)
{
	// actually reset the password
	$hashedPassword = PlainPasswToDBPassw($NewPassw);
	$rows = executeMySQLQuery("Update Users set password=? where UserID=$UserId", $hashedPassword);
	if($rows[0]['res'] !== true)
	{
		$err = "Failed to set password ";
		$errId = FAILED_TO_CREATE_DB_ENTRY;
	}
	// consider this reset req consumed
	$rows = executeMySQLQuery("Update PsswRstQueue set IsConsumed=1 where ResetID=?", $reqId);
	if($rows[0]['res'] !== true)
	{
		$err = "Failed to consume request";
		$errId = FAILED_TO_CREATE_DB_ENTRY;
	}
}

if($errId === NO_ERR)
{
	$htmlContent = file_get_contents("./NotHosted/PasswResetConfirmTemplate.html");

	$rows = executeMySQLQuery("Select FirstName, LastName from Users where UserId=$UserId");
	if(isset($rows[0]['FirstName']))
	{	
		$FullName = $rows[0]['FirstName']." ".$rows[0]['LastName'];
		$htmlContent = str_replace("{user_name}", $FullName, $htmlContent);
	}
	// Replace placeholders in the HTML content with actual values
	$htmlContent = str_replace("{new_password}", $NewPassw, $htmlContent);
	
	echo $htmlContent;
}
else
{
	$htmlContent = file_get_contents("./NotHosted/PasswResetFailedTemplate.html");
	echo $htmlContent;
}

//do not show empty brackets on this page. Ever
unset($res);

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
