<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

// default values
$TargetUserId = 0;

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("TargetUserId");
$genericRequiredFields=array("AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

// set session values like UserId, RoleId ...
CheckSessionAndUserIfCanUseAPI();

// check for action rights
if($errId === NO_ERR)
{
	$TargetUserId = (int)$_POST["TargetUserId"]; // if this is 0, it will be auto filled with session ID
	$TargetUserOrganizationId = 0; // if this is 0, it will be auto filled with organization ID from DB
echo "Target user id before rights call $TargetUserId";	
	CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
		USER_RIGHT_ID_UPDATE_USER_INFO, ORGANIZATION_RIGHT_UPDATE_USERS, 
		USER_RIGHT_UPDATE_OTHER_USER, USER_RIGHT_UPDATE_OTHER_ORGANIZATIONS );
}

// actually update fields
if($errId === NO_ERR)
{
	$updatableFields = array("FirstName", "LastName", "Email");

	$AllFuncParams = [];
	$AllFuncParams[] = "the query";
	
	$updateQueryPart = "";
	foreach($updatableFields as $index => $fieldname)
	{
		if(isset($_POST[$fieldname]))
		{
			$updateQueryPart .= $fieldname."=?,";
			$AllFuncParams[] = $_POST[$fieldname];
		}
	}
	
	// actually got fields to update ?
	if($updateQueryPart != "")
	{
		$updateQuery = "update Users set ".substr($updateQueryPart, 0, -1)." where UserID=".$TargetUserId;
		$AllFuncParams[0] = $updateQuery;
		$row = call_user_func_array("executeMySQLQuery", $AllFuncParams);
		if(!isset($row[0]['res']) || $row[0]['stmt_err'] !== "" || $row[0]['sql_err'] !== "" )
		{
			$err = "Failed to update user settings. Err statement ".$row[0]['stmt_err']." sql ".$row[0]['sql_err'];
			$errId = FAILED_TO_CREATE_DB_ENTRY;
		}
	}
}

// unless we return something, API call might be considered "lost" or errored
if($errId === NO_ERR)
{
	$ret[] = array("res","1");
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "Updated target user $TargetUserId. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
