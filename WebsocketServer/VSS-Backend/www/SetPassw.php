<?php
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("NewPssw", "TargetUserId");
$genericRequiredFields=array("AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

// check for action rights
if($errId === NO_ERR && isset($_POST["TargetUserId"]))
{
	$TargetUserId = (int)$_POST["TargetUserId"]; // if this is 0, it will be auto filled with session ID
	$TargetUserOrganizationId = 0; // if this is 0, it will be auto filled with organization ID from DB
	CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
		USER_RIGHT_ID_UPDATE_USER_INFO, ORGANIZATION_RIGHT_UPDATE_USERS, 
		USER_RIGHT_UPDATE_OTHER_USER, USER_RIGHT_UPDATE_OTHER_ORGANIZATIONS );
}

// update data
$NewPssw = $_POST["NewPssw"];
if($errId === NO_ERR)
{
	// actually reset the password
	$hashedPassword = password_hash($NewPssw, PASSWORD_BCRYPT);
	$row = executeMySQLQuery("Update Users set Password=? where UserID=$TargetUserId", $hashedPassword);
	if(!isset($row[0]['res']) || $row[0]['stmt_err'] !== "" || $row[0]['sql_err'] !== "" )
	{
		$err = "Failed to set password";
		$errId = FAILED_TO_CREATE_DB_ENTRY;
	}
}

// unless we return something, API call might be considered "lost" or errored
if($errId === NO_ERR)
{
	$ret[] = array("res","1");
}

// Log activity
LogAPIUsage(LOG_SOURCE_PASSWCHANGE, LOG_SEVERITY_NORMAL, "Passw changed to $NewPssw" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
