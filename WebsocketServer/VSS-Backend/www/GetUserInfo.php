<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

// default values
$TargetUserId = "";

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("TargetUserId");
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
		USER_RIGHT_ID_VIEW_USER_INFO, ORGANIZATION_RIGHT_VIEW_USERS, 
		USER_RIGHT_VIEW_OTHER_USER, USER_RIGHT_VIEW_OTHER_ORGANIZATIONS );
}

// get the target user data. Will be used as return values
if($errId === NO_ERR)
{
	$TargetUserId = (int)$_POST["TargetUserId"];
	// UI application does not know it's own UserId. No problem we auto set it based on the session value
	if($TargetUserId === 0)
	{
		$TargetUserId = $session_UserId;
	}
	
	// Get all the user info, we might return only some
	$rows = executeMySQLQuery("Select 
		RoleID, OrganizationID, UserActiveID, UNIX_TIMESTAMP(ValidUntilTimestamp) as ValidUntilTimestamp, Email, 
		FirstName, LastName, Photo, UNIX_TIMESTAMP(DateOfBirth) as DateOfBirth, JobRole, 
		OfficePhoneNumber, MobilePhoneNumber
		from Users where UserID=$TargetUserId");
		
	// check if user exists
	if( !isset($rows[0]['RoleID']))
	{
		$err = "Could not find user";
		$errId = INVALID_USER;
	}
	// prepare data to be returned to the UI App
	else
	{
		$target_RoleId = (int)$rows[0]['RoleID'];
		$target_OrganizationID = (int)$rows[0]['OrganizationID'];
		// send back all the columns as JSON
		foreach($rows[0] as $key => $val )
		{
			if((int)$key==$key) // not sure what this is
			{
				continue;
			}
			$ret_if_no_error[$key] = $val;
		}
	}
}

// set returned data
if($errId === NO_ERR)
{
	// merge user info with other reply values
	$ret = array_merge($ret, $ret_if_no_error);
}

// log API usage 
LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "User viewed target user $TargetUserId. Err = $errId" );

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
