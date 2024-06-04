<?php
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("uname", "token");
$genericRequiredFields=array("AppVer", "ClientEndpoint");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));


if($errId === NO_ERR)
{
	$uname = $_POST['uname'];

	$rows = executeMySQLQuery("Select UserId, Password, UserActiveID, ValidUntilTimestamp, 
		FirstName, LastName, RoleID, Photo, OrganizationID, Email, JobRole,
		UNIX_TIMESTAMP(DateOfBirth) as DateOfBirth, MFATypeID from users where Email=?", $uname);
	
	if(!isset($rows[0]['UserId']))
	{
		$pword = $_POST['token'];
		LogAPIUsage(LOG_SOURCE_LOGIN, LOG_SEVERITY_NORMAL, "Failed to MFA log in : $uname / $pword" );
		
		$err = "Failed to MFA log in (Bad user) : $uname / $pword";
		$errId = BAD_USERNAME_ERROR_CODE;
	}
	else
	{
		$UserId = $rows[0]['UserId'];
	}
}

if($errId === NO_ERR)
{	
	$MFAToken = $_POST['token'];
	$rows2 = executeMySQLQuery("Select SessionID from UserSessions where UserID=? and MFAToken=? and MFAExpires>NOW()", $UserId, $MFAToken);
	
	if(!isset($rows2[0]['SessionID']))
	{
		LogAPIUsage(LOG_SOURCE_LOGIN, LOG_SEVERITY_NORMAL, "Failed to MFA log in : $uname / $MFAToken" );
		
		$err = "Failed to MFA log in (Bad token) : $uname / $MFAToken";
//echo "Select SessionID from UserSessions where UserID=$UserId and MFAToken=$MFAToken and MFAExpires>NOW()";var_dump($rows2);
		$errId = BAD_TOKEN_ERROR_CODE;
	}
	else
	{
		$sessionId = $rows2[0]['SessionID'];
	}
}
	
if($errId === NO_ERR)
{
	$ret['SessionId'] = $sessionId;
	$ret['UserId'] = $UserId;
	$ret['FirstName'] = $rows[0]['FirstName'];
	$ret['LastName'] = $rows[0]['LastName'];
	$ret['RoleID'] = $rows[0]['RoleID'];
	$ret['Photo'] = $rows[0]['Photo'];
	$ret['OrganizationID'] = $rows[0]['OrganizationID'];
	$ret['Email'] = $rows[0]['Email'];
	$ret['JobRole'] = $rows[0]['JobRole'];
	$ret['DateOfBirth'] = $rows[0]['DateOfBirth'];

	// fetch rights for this user
	$rows3 = executeMySQLQuery("SELECT RightId FROM RoleRights where RoleID = ?", $ret['RoleID']);
	$ret['Rights'] = $rows3;

	$session_UserId = $UserId;
	
	$rows4 = executeMySQLQuery("update UserSessions set MFAType=0 where SessionID=?", $sessionId);
	
	// In case we wish to check which users are active on a specific computer
	// Or statistics how many computers a specific user is active on ?
	// Or maybe we need the User ID of some computer that refuses to log in. This is because we might want to ban a hacker before he logs in
	StoreUserClientEndpoint();
	
	LogAPIUsage(LOG_SOURCE_LOGIN, LOG_SEVERITY_NORMAL, "Successfully logged in with MFA. Got session id : $sessionId");

	// change the dymanic part of the session
	$ret['stamp'] = time();
} 

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>