<?php
// include constants, function and get an SQL connection
require_once('NotHosted/header_include.php');

$isDPSCall = 0;
// should also check that caller is allowed to access this API
if(isset($_POST["ThisIsADPSCall"]) && $_POST["ThisIsADPSCall"] == $_POST["AlertID"] )
{
	$isDPSCall = 1;
}

if($isDPSCall == 0)
{
	// this API call only makes sense if we received this param
	$EndpointSpecificColumns=array("UserID", "OrganizationID", "AlertID");
	$genericRequiredFields=array("AppVer", "ClientEndpoint", "SessionKey", "SessionSalt");
	CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

	// set session values like UserId, RoleId ...
	CheckSessionAndUserIfCanUseAPI();

	// check for action rights
	if($errId === NO_ERR)
	{
		$TargetUserId = (int)$_POST["UserID"]; // if this is 0, it will be auto filled with session ID
		$TargetUserOrganizationId = (int)$_POST["OrganizationID"]; // if this is 0, it will be auto filled with organization ID from DB
		CheckHasRightDB($TargetUserId, $TargetUserOrganizationId, 
			USER_RIGHT_ID_CREATE_ALERT, ORGANIZATION_RIGHT_CREATE_ALERT, 
			USER_RIGHT_UPDATE_OTHER_USER, USER_RIGHT_UPDATE_OTHER_ORGANIZATIONS );
	}
}

// fetch requested data
if($errId === NO_ERR)
{
	$AlertId = (int)$_POST["AlertID"];
	
	// Get all the user info, we might return only some
	$rows = executeMySQLQuery("Select AD.AlertDefinitionName, AD.AlertMobilePhoneNumber,
	AI.LocationId, GL.LocationName
	from Alerts AI
	INNER JOIN AlertDefintion AD ON AD.AlertDefinitionId = AI.AlertDefinitionId
	INNER JOIN GeographicLocations GL ON AI.LocationId = GL.LocationID
	where AI.AlertId=$AlertId and ( AlertStatusTypeId & 4 ) = 0
	and AD.Disabled=0 and AD.AlertActionTypeId in (1,3)
	");
		
	// check if user exists
	if( !isset($rows[0]['AlertMobilePhoneNumber']) || strlen($rows[0]['AlertMobilePhoneNumber']) < 3)
	{
		$err = "Could not find Alert";
		$errId = INVALID_USER;
	}
}

if($errId === NO_ERR)
{
	$AlertDefinitionName = $rows[0]['AlertDefinitionName'];
	$TargetLocationName = $rows[0]['LocationName'];
	
	$SMSContent = file_get_contents("./NotHosted/SMSNotificationTemplate.txt");
	
	$SMSContent = str_replace("{AlertDefinitionName}", $AlertDefinitionName, $SMSContent);
	$SMSContent = str_replace("{LocationName}", $TargetLocationName, $SMSContent);

	if(strpos($rows[0]['AlertMobilePhoneNumber'],";"))
	{
		$to_array = explode(";", $rows[0]['AlertMobilePhoneNumber']);
		foreach($to_array as $key => $toNumber)
		{		
			SendSMSSingleNumber($to, $SMSContent);
			
			if($errId !== NO_ERR)
			{
				break;
			}
		}
	}
	else
	{
		SendSMSSingleNumber($rows[0]['AlertMobilePhoneNumber'], $SMSContent);
	}
}

if($errId === NO_ERR)
{
	// update DB marking this Alert having it's SMS sent
	$rows = executeMySQLQuery("Update Alerts set AlertStatusTypeId = AlertStatusTypeId | 4 where AlertId=$AlertId");	
}

// log API usage 
if($isDPSCall == 0)
{
	LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "$TargetUserId sent an SMS. Err = $errId" );
}

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
