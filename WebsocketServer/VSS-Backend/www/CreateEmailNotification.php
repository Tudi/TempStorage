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
	$rows = executeMySQLQuery("Select AD.AlertDefinitionName, AD.AlertEmail,
	AI.LocationId, GL.LocationName
	from Alerts AI
	INNER JOIN AlertDefintion AD ON AD.AlertDefinitionId = AI.AlertDefinitionId
	INNER JOIN GeographicLocations GL ON AI.LocationId = GL.LocationID
	where AI.AlertId=$AlertId and ( AlertStatusTypeId & 2 ) = 0
	and AD.Disabled=0 and AD.AlertActionTypeId in (2,3)
	");
		
	// check if user exists
	if( !isset($rows[0]['AlertEmail']) || strlen($rows[0]['AlertEmail']) < 3)
	{
		$err = "Could not find Alert";
		$errId = INVALID_USER;
	}
}

if($errId === NO_ERR)
{
	$AlertDefinitionName = $rows[0]['AlertDefinitionName'];
	$TargetLocationName = $rows[0]['LocationName'];
	
	$EmailContent = file_get_contents("./NotHosted/EmailNotificationTemplate.html");
	
	$EmailContent = str_replace("{AlertDefinitionName}", $AlertDefinitionName, $EmailContent);
	$EmailContent = str_replace("{LocationName}", $TargetLocationName, $EmailContent);

	if(strpos($rows[0]['AlertEmail'],";"))
	{
		$to_array = explode(";", $rows[0]['AlertEmail']);
		foreach($to_array as $key => $toNumber)
		{		
			SendEmailSingleDest($to, $EmailContent);
			
			if($errId !== NO_ERR)
			{
				break;
			}
		}
	}
	else
	{
		SendEmailSingleDest($rows[0]['AlertEmail'], $EmailContent);
	}
}

if($errId === NO_ERR)
{
	// update DB marking this Alert having it's Email sent
	$rows = executeMySQLQuery("Update Alerts set AlertStatusTypeId = AlertStatusTypeId | 2 where AlertId=$AlertId");		
}

// log API usage 
if($isDPSCall == 0)
{
	LogAPIUsage(LOG_SOURCE_USER_ACTION, LOG_SEVERITY_NORMAL, "$TargetUserId sent an Email notification. Err = $errId" );
}

function SendEmailSingleDest($to, $htmlContent)
{
	global $err, $errId;
	// Send the email
	$subject = "VSS Alert Notification";
	$headers = "From: ".const_EmailNotificationSender."\r\n";
	$headers .= "MIME-Version: 1.0\r\n";
	$headers .= "Content-Type: text/html; charset=UTF-8\r\n";

	if (mail($to, $subject, $htmlContent, $headers)) 
	{
		$ret['res'] = 1;
	} 
	else 
	{
		$errId = FAILED_TO_SEND_EMAIL;
	}
	
	return $ret;
}

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
