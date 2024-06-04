<?php
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("uname", "pword", "stamp", "lic");
$genericRequiredFields=array("AppVer", "ClientEndpoint");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

if($errId === NO_ERR)
{
	$uname = $_POST['uname'];

	$rows = executeMySQLQuery("Select UserId, Password, UserActiveID, ValidUntilTimestamp, 
		FirstName, LastName, RoleID, Photo, OrganizationID, Email, JobRole,
		UNIX_TIMESTAMP(DateOfBirth) as DateOfBirth, MFATypeID, MobilePhoneNumber from users where Email=?", $uname);
	
	if(!isset($rows[0]['UserId']) || strlen($rows[0]['Password']) == 0)
	{
		$pword = $_POST['pword'];
		LogAPIUsage(LOG_SOURCE_LOGIN, LOG_SEVERITY_NORMAL, "Failed to log in : $uname / $pword" );
		
		$err = "Failed to log in (Bad user) : $uname / $pword";
		$errId = BAD_USERNAME_ERROR_CODE;
	}	
}

if($errId === NO_ERR)
{	
	$MFAType = $rows[0]['MFATypeID'];
	$UserId = $rows[0]['UserId'];
	$DBPwd = $rows[0]['Password'];
}

if($errId === NO_ERR)
{	
	$pword = $_POST['pword'];
	if (password_verify($pword, $DBPwd) == false) 
	{
		LogAPIUsage(LOG_SOURCE_LOGIN, LOG_SEVERITY_NORMAL, "Failed to log in : $uname / $pword" );
		
		$err = "Failed to log in (Bad passw) : $uname / $pword";
		$errId = BAD_PASSWORD_ERROR_CODE;
	}
}

// password was good. Depending on MFA type, send an email or SMS
$MFAToken = "";
$MFAExpires = 1;
if($errId === NO_ERR && $MFAType != MFA_TYPE_PWD_LOGIN)
{
	$MinValue = 10;
	for($i=2;$i<const_MFADigitCount;$i++)
	{
		$MinValue *= 10;
	}
	
	$MFAToken = rand($MinValue, $MinValue * 10 - 1);
	$MFAExpires = time() + const_MFAExpiresAfter;
}

if($errId === NO_ERR)
{
	// get rid of the previous session for this user 
	$delRes = executeMySQLQuery("delete from UserSessions where UserID=?", $UserId);
	
	$sessionId = GenSessionID($UserId);
	$ins_ret = executeMySQLQuery("insert into UserSessions 
			(SessionID,UserID,SessionIpAddress,MFAType,MFAToken,MFAExpires) values (?,?,?,?,?,FROM_UNIXTIME(?))", 
			$sessionId, $UserId, getPeerIP(), $MFAType, $MFAToken, $MFAExpires);
}
	
if($errId === NO_ERR && $MFAType == MFA_TYPE_EMAIL_TOKEN)
{
	$EmailContent = file_get_contents("./NotHosted/EmailMFATemplate.html");
	
	$EmailContent = str_replace("{TokenValue}", $MFAToken, $EmailContent);
	$EmailContent = str_replace("{TokenExpires}", date('Y-m-d H:i', $MFAExpires), $EmailContent);

	// Send the email
	$subject = "VSS Login Token";
	$headers = "From: ".const_MFASenderEmail."\r\n";
	$headers .= "MIME-Version: 1.0\r\n";
	$headers .= "Content-Type: text/html; charset=UTF-8\r\n";

	if (!mail($uname, $subject, $EmailContent, $headers)) 
	{
		$errId = FAILED_TO_SEND_EMAIL;
	}	
}

if($errId === NO_ERR && $MFAType == MFA_TYPE_SMS_TOKEN)
{
	$SMSContent = file_get_contents("./NotHosted/SMSMFATemplate.txt");
	
	$SMSContent = str_replace("{TokenValue}", $MFAToken, $SMSContent);
	$SMSContent = str_replace("{TokenExpires}", date('Y-m-d H:i', $MFAExpires), $SMSContent);

	// Send the SMS
	$SMSTo = $rows[0]['MobilePhoneNumber'];
	SendSMSSingleNumber($SMSTo, $SMSContent);
}

if($errId === NO_ERR && $MFAType == MFA_TYPE_PWD_LOGIN)
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

	// fetch the string representation of this role
//			$rows2 = executeMySQLQuery("Select RoleName, RoleDescription from UserRoleDefines where RoleID=?", $ret['RoleID']);
//			$ret['Role'] = $rows2[0]['RoleName'];
//			$ret['RoleDesc'] = $rows2[0]['RoleDescription'];
	
	// fetch rights for this user
	$rows3 = executeMySQLQuery("SELECT RightId FROM RoleRights where RoleID = ?", $ret['RoleID']);
	$ret['Rights'] = $rows3;

	$session_UserId = $UserId;
	
	// In case we wish to check which users are active on a specific computer
	// Or statistics how many computers a specific user is active on ?
	// Or maybe we need the User ID of some computer that refuses to log in. This is because we might want to ban a hacker before he logs in
	StoreUserClientEndpoint();
	
	LogAPIUsage(LOG_SOURCE_LOGIN, LOG_SEVERITY_NORMAL, "Successfully logged in. Got session id : $sessionId");

	$ret['stamp'] = time();
} 

// change the dymanic part of the session
$ret['MFAType'] = "".$MFAType."";

function GenSessionID($UserId)
{
	$filename = "sessionId.txt";
	$content = "";

	// Define the maximum number of retries
	$maxRetries = 5;

	$retryCount = 0;
	$fileCreated = false;

	while (!$fileCreated && $retryCount < $maxRetries) 
	{
		// Attempt to create the file
		$result = file_put_contents($filename, $content);

		if ($result !== false)
		{
			// File creation successful
			$fileCreated = true;

			// Get the file creation time
			$fileCreationTime = filectime($filename);

			// Delete the file
			unlink($filename);
		} 
		else 
		{
			// File creation failed, retry
			$retryCount++;
		}
	}

	if (!$fileCreated) 
	{
		$microtime = hrtime(false);
		$microtime[1] = (int)($microtime[0] / 100);
		$fileCreationTime = $fileCreationTime[0].$fileCreationTime[1];
	}
	
	$fileCreationTime = (int)$fileCreationTime & (int)0x7FFFFFFF;
	
	$microtime = hrtime(false);
	$microtime[1] = (int)($microtime[0] / 100);
	
	$sessionId = strlen($UserId).$UserId.$fileCreationTime.$microtime[0].$microtime[1];
	
	if(strlen($sessionId) >= 20)
		$sessionId = substr($sessionId,0,19);
	
	return $sessionId;
}

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>