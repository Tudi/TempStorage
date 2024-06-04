<?php

//$ErrorStack = [];
function LogMsg($msg)
{
	global $DebugExecution, $ret;
	if(isset($DebugExecution) && $DebugExecution === 1)
	{
//		echo $msg."<br>";
		$ret["_comment"] = $msg;
		
		// Add a timestamp to the log message
		$logEntry = date('Y-m-d H:i:s').'-'.$_SERVER['PHP_SELF'].'-'.$msg."-Callstack:".getPHPCallerFunctionInfo(4)."\n";

		// Open the log file in append mode
		if ($fileHandle = fopen("./NotHosted/DebugLogs.txt", 'a')) 
		{
			// Write the log entry to the file
			fwrite($fileHandle, $logEntry);
			fclose($fileHandle);
		}
	}
/*	$numArgs = func_num_args();
	if($numArgs > 1)
	{
		$params = func_get_args();
		$ErrorStack[] = $params[1];
	}*/
}

function getPeerIP()
{
	if(!empty($_SERVER['HTTP_CLIENT_IP'])) {  
        return $_SERVER['HTTP_CLIENT_IP'];  
    }  
    //if user is from the proxy  
    elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR'])) {  
        return $_SERVER['HTTP_X_FORWARDED_FOR'];  
    }  
    //if user is from the remote address  
    else{  
        return $_SERVER['REMOTE_ADDR'];  
    }
	return "";
}

function CheckSessionAndUserIfCanUseAPI()
{
	global $_SERVER,$_POST;
	global $session_key,$session_tUserId,$session_UserId,$session_salt,$errId,$err;
	global $session_RoleId,$session_OrganizationId,$session_UserActiveID,$session_CreatedAt;
	global $err, $errId;	

	// do not overwrite previous reported error. Stop execution at first error
	if($errId !== NO_ERR)
	{
		return;
	}
	
	$session_key = "";
	$session_tUserId = 0;
	$session_UserId = 0;
	$session_salt = "";
	$session_RoleId = 0;
	$session_OrganizationId = 0;
	$session_UserActiveID = 0;
	$session_CreatedAt = 0;
	
	// do not overwrite previous reported error. Stop execution at first error
	if($errId !== NO_ERR)
	{
		return;
	}
	
	// client should know our time with X seconds accuracy or something is wrong
	if($errId === NO_ERR )
	{
		if(isset($_POST["SessionSalt"]))
		{
			$session_salt = (int)$_POST["SessionSalt"];
			
			$baseVal = (int)(time() / 3);
			$SaltMatched = 0;
			$accepted = "";
			for($i=-2;$i<=2;$i++) // extra 2 second latency compensation
			{
				$serverSalt = (int)(( $baseVal + $i ) % 37);
				$accepted .= "$serverSalt,";
				if($serverSalt === $session_salt)
				{
					$SaltMatched = 1;
					break;
				}
			}
			if( $SaltMatched === 0 )
			{
//				$err = "Session salt mismatch. Got $session_salt, expected $accepted";
				$err = "Session salt mismatch. Got $session_salt";
				$errId = SESSION_SALT_MISMATCH;
			}
		}
		else
		{
			$err = "Session info missing";
			$errId = MISSING_PARAMETERS;
		}		
	}
	
	// get session related data
	if($errId === NO_ERR)
	{
		if(isset($_POST["SessionKey"]))
		{
			// parse provided data
			$session_key = $_POST["SessionKey"];
			$idlen = (int)$session_key[0];
			$session_tUserId = (int)substr($session_key,1,$idlen);
			
			// get userID for this session
			$rows = executeMySQLQuery("Select UserId, UNIX_TIMESTAMP(SessionCreatedAt) as SessionCreatedAt
				from UserSessions where SessionID=? and (MFAType = 0 or MFAType = 1) limit 1", $session_key);
			if(isset($rows[0]['UserId']) && (int)$rows[0]['UserId'] === (int)$session_tUserId)
			{
				$session_UserId = (int)$rows[0]['UserId'];
				$session_CreatedAt = (int)$rows[0]['SessionCreatedAt'];
			}
			else
			{
				$err = "Could not find session with id $session_key";
				$errId = INVALID_SESSION;
			}
		}
		else
		{
			$err = "Session info missing";
			$errId = MISSING_PARAMETERS;
		}		
	}

	// check if session expired
	if($errId == NO_ERR)
	{
		if(const_loginSessionExpiresAfter > 0 
			&& $session_CreatedAt + const_loginSessionExpiresAfter < time())
		{	
			$err = "Session expired : $ReqCreatedTimestamp ".($session_CreatedAt + const_loginSessionExpiresAfter)."< ".time();
			$errId = SESSION_EXPIRED;
		}
	}
	
	// get user related data
	if($errId == NO_ERR)
	{
		// check if user is still active
		$rows = executeMySQLQuery("Select RoleID, OrganizationID, UserActiveID, UNIX_TIMESTAMP(ValidUntilTimestamp) as ValidUntilTimestamp
			from Users where UserID=$session_UserId");
		if(isset($rows[0]['RoleID']))
		{
			$session_RoleId = (int)$rows[0]['RoleID'];
			$session_OrganizationId = (int)$rows[0]['OrganizationID'];
			$session_UserActiveID = (int)$rows[0]['UserActiveID'];
			$session_ValidUntil = (int)$rows[0]['ValidUntilTimestamp'];
		}
		else
		{
			$err = "Could not find user";
			$errId = INVALID_USER;
		}
	}
	
	// check if user is disabled/banned/deleted...
	if($errId == NO_ERR)
	{
		if($session_UserActiveID != USER_IS_ACTIVE_USABLE)
		{
			$err = "User is not active";
			$errId = NON_ACTIVE_USER;
		}
	}
	if($errId == NO_ERR)
	{
		if($session_ValidUntil != 0 && $session_ValidUntil < time())
		{
			$err = "User time expired";
			$errId = NON_ACTIVE_USER;
		}
	}
	
	// check if user has access to this API
	if($errId == NO_ERR)
	{
		$fileName = basename($_SERVER['PHP_SELF']);
		// check if user is still active
		$rows = executeMySQLQuery("SELECT COUNT(*) as hasAccess FROM ApiEndpointDefines AS a
			INNER JOIN RoleEndpointRights AS r ON a.EndpointDefineID = r.EndpointDefineID
			WHERE r.RoleID = $session_RoleId AND a.EndpointDefineName = ?", $fileName);
		if(!isset($rows[0]['hasAccess']) || (int)$rows[0]['hasAccess'] === 0)
		{
			$err = "No API access for this role";
			$errId = NO_API_ACCESS;
		}
	}	
}

function CheckHasRight($NeededRight,$RightsArray)
{
	global $session_rights, $session_RoleId;
	// no dedicated array has been given
	if(is_array($RightsArray)==false)
	{
		// user has no rights ?
		if(!isset($session_RoleId))
		{
			return false;
		}
		// try to cache rights for multiple uses 
		if(is_array($session_rights)==false)
		{
			$session_rights = executeMySQLQuery("SELECT RightId FROM RoleRights where RoleID = $session_RoleId" );
		}
		$RightsArray = $session_rights;
	}
	// actually check rights
	foreach($RightsArray as $rowIndex => $row)
	{
		if((int)$row['RightId'] === $NeededRight)
		{
			return true;
		}
	}
	return false;
}

function CheckHasRightDB($TargetUserId, $Target_OrganizationID, 
	$NeededUserRight, $NeededOrganizationRight,
	$NeededUserRightOthers, $NeededOrganizationRightOthers)
{
	global $_SERVER,$_POST;
	global $session_key,$session_tUserId,$session_UserId,$session_salt,$errId,$err;
	global $session_RoleId,$session_OrganizationId,$session_UserActiveID,$session_CreatedAt;
	
	// make sure these are numeric
	$TargetUserId = (int)$TargetUserId;
	$Target_OrganizationID = (int)$Target_OrganizationID;
	
	// fill in data that was not provided for the function
	if($TargetUserId == 0)
	{
		$TargetUserId = $session_UserId;
	}

	// fill in data that was not provided for the function
	if($Target_OrganizationID == 0)
	{
		if($TargetUserId != $session_UserId)
		{
			$targetOrganizationRow = executeMySQLQuery("SELECT OrganizationID FROM Users where UserID=$TargetUserId" );
			if(isset($targetOrganizationRow[0]["OrganizationID"]))
			{
				$Target_OrganizationID = $targetOrganizationRow[0]["OrganizationID"];
			}
			else
			{
				$err = "Organization not found for user $TargetUserId";
				$errId = INVALID_USER;
				return;
			}
		}
		else
		{
			$Target_OrganizationID = $session_OrganizationId;
		}
	}
	
	// check if we have the rights to view user info of others
	if($TargetUserId === $session_UserId && 
			$NeededUserRight != 0)
	{
		if(CheckHasRight($NeededUserRight, "") === false )
		{
			$err = "Not enough self rights";
			$errId = NOT_ENOUGH_RIGHTS;
			return;
		}
	}
	else if($TargetUserId !== $session_UserId &&
			$NeededUserRight != 0 && $NeededUserRightOthers != 0)
	{
		if(CheckHasRight($NeededUserRight, "") === false ||
			CheckHasRight($NeededUserRightOthers, "") === false)
		{
			$err = "Not enough other rights";
			$errId = NOT_ENOUGH_RIGHTS;
			return;
		}
	}
	
	// if we should check for organization rights
	if($Target_OrganizationID === $session_OrganizationId && 
			$NeededOrganizationRight != 0)
	{
		if(CheckHasRight($NeededOrganizationRight, "") === false )
		{
			$err = "Not enough self organization rights";
			$errId = NOT_ENOUGH_RIGHTS;
			return;
		}
	}
	else if($Target_OrganizationID !== $session_OrganizationId && 
			$NeededOrganizationRight != 0 && $NeededOrganizationRightOthers != 0)
	{
		if(CheckHasRight($NeededOrganizationRight, "") === false ||
			CheckHasRight($NeededOrganizationRightOthers, "") === false)
		{
			$err = "Not enough other organization rights";
			$errId = NOT_ENOUGH_RIGHTS;
			return;
		}
	}
}

function StoreUserClientEndpoint()
{
	global $session_UserId;
	if(!isset($_POST["ClientEndpoint"]))
	{
		return;
	}
	if(!isset($session_UserId) || (int)$session_UserId === (int)0)
	{
		return;
	}
	executeMySQLQuery("insert ignore into UserClientEndpoints (ClientEndpoint,UserId,ClientIP)values(?,".$session_UserId.",?)", $_POST["ClientEndpoint"], getPeerIP());
}

function StoreUserEndpointActivity()
{
	global $session_UserId;
	$UserId = 0;
	$IsGuessed = 0;
	if(!isset($session_UserId) || $session_UserId === 0)
	{
		$UserId = GuessUserId();
		if($UserId == 0)
		{
			return;
		}
		$IsGuessed = 1;
	}
	else
	{
		$UserId = $session_UserId;
	}
	$SeparatorToken="!@#$%";
	$params = "";
	foreach($_REQUEST as $key => $value)
	{
		$params .= $key.$SeparatorToken.$value.$SeparatorToken;
	}
	if($params != "")
	{
		$params = substr($params,0,strlen($params)-strlen($SeparatorToken));
	}
	executeMySQLQuery("insert into UserAPIActivity (UserId, ApiUrl, ApiParams, IsGuessedUserId) values ($UserId, ?, ?, $IsGuessed)", $_SERVER['PHP_SELF'], $params);
}

function GuessUserId()
{
	if(!isset($_REQUEST["ClientEndpoint"]) || strlen($_REQUEST["ClientEndpoint"]) == 0)
	{
		return 0;
	}
	
	// Try to guess if this ClientEnpoint and IP combination is unique to this user
	$rows = executeMySQLQuery("SELECT UserId from UserClientEndpoints where ClientEndpoint=? and ClientIP=? limit 0,2", $_REQUEST["ClientEndpoint"], getPeerIP());
	
	// Client has never logged in from this computer with this IP
	if(!isset($rows[0]['UserId']) || (int)$rows[0]['UserId'] === 0)
	{
		// maybe he is using dynamic IP
		$rows = executeMySQLQuery("SELECT UserId from UserClientEndpoints where ClientEndpoint=? limit 0,2", $_REQUEST["ClientEndpoint"]);
	}

	// Still nothing about this computer ?
	if(!isset($rows[0]['UserId']) || (int)$rows[0]['UserId'] === 0)
	{
		return 0;
	}	
	
	// more than 1 clients use this computer. Can't guess which one would be using this API
	if(count($rows) > 1)
	{
		return 0;
	}
		
	// looks like there is one user registered to this computer
	return $rows[0]['UserId'];
}

// changes Salt of a session key. The dynamic part of a sessionID
/*function GenNewAccountSalt()
{
	global $session_key,$session_salt;
	
	$AddSalt = mt_rand(0, 9);
	if($AddSalt>4)
	{
		$rows = executeMySQLQuery("Update UserSessions set SessionSalt=SessionSalt + $AddSalt where SessionID=? and SessionSalt=?", $session_key, $session_salt);
		if( $rows[0]['res'] == false )
		{
			$AddSalt = 0;
		}
	}
	else
	{
		$AddSalt = 0;
	}
	
	return $AddSalt;
}*/

function AddGenericDBLog($ClientStamp, $LogSourceGroup, $LogSource, $LogSeverityID, $LogDetails)
{
	global $_SERVER, $session_UserId;

	if($ClientStamp != 0)
	{
		$rows = executeMySQLQuery("Insert into logs 
		(UserID, LogSourceGroup, LogSource, LogSeverityID, LogDetails, LogClientStamp) values
		(?, ?, ?, ?, ?, FROM_UNIXTIME(?))", $session_UserId, $LogSourceGroup, $LogSource, $LogSeverityID, $LogDetails, $ClientStamp);
	}
	else
	{
		$rows = executeMySQLQuery("Insert into logs 
		(UserID, LogSourceGroup, LogSource, LogSeverityID, LogDetails) values
		(?, ?, ?, ?, ?)", $session_UserId, $LogSourceGroup, $LogSource, $LogSeverityID, $LogDetails);
	}
}

function LogAPIUsage($LogSource, $LogSeverityID, $msg)
{
	global $_SERVER, $session_UserId;
	
	$LogDetails = getPHPCallerFunctionInfo(3)."$msg";
	AddGenericDBLog(0, LOG_GROUP_BACKEND, $LogSource, $LogSeverityID, $LogDetails);
}


function getPHPCallerFunctionInfo($depth) {
	global $_SERVER;
	
	$ret = "";
	$backtrace = debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS, $depth);
	
	for($i=1;$i<$depth;$i++)
	{
		if(!isset($backtrace[$i]))
		{
			continue;
		}
		$callerInfo = $backtrace[$i];

		if (isset($callerInfo['file'])) {
			$ret .= basename($callerInfo['file']).":";
		}
		else if (isset($_SERVER['PHP_SELF'])) {
			$ret .= basename($_SERVER['PHP_SELF']).":";
		}


		if (isset($callerInfo['line'])) {
			$ret .= $callerInfo['line'].":";
		}

		if (isset($callerInfo['function'])) {
			$ret .= $callerInfo['function'].":";
		}
	}
    
	return $ret;
}

function CheckRequiredFields($RequestType, $FieldNameArray)
{
	global $err, $errId;
	
	// do not overwrite previous reported error. Stop execution at first error
	if($errId !== NO_ERR)
	{
		return;
	}
	if(!is_array($FieldNameArray))
	{
//echo "It's  not and array\n";
		return;
	}
	foreach($FieldNameArray as $index => $fieldName)
	{
		if(!isset($RequestType[$fieldName]))
		{
//echo "It's not set : $fieldName\n";
			$err = "Required field is missing : $fieldName";
			$errId = MISSING_PARAMETERS;
			return;
		}
		else
		{
//echo "It's set : $fieldName = ".$RequestType[$fieldName]."\n";
		}
	}
}

function crc31_hash($seed, $data) {
    $crc = $seed & 0x7FFFFFFF; // Initialize with all bits set to 1 except the sign bit
    $s = unpack('C*', $data);

//foreach ($s as $value) {
//    echo $value . ' ';
//} echo "<br>\n";

    foreach ($s as $byte) {
        $crc = $crc ^ $byte;
        for ($j = 0; $j < 8; $j++) {
            $crc = ($crc >> 1) ^ (0x4C11DB7 & (-($crc & 1)));
//echo $crc.' ';			
        }
    }
//echo "<br>\n";

    return $crc & 0x7FFFFFFF; // Mask to keep only the lower 31 bits
}

function PlainPasswToDBPassw($str)
{
	$hashedPassword1 = crc31_hash(0xFEED, $str);
//	echo "got seed str= $str hash1=$hashedPassword1 <br>\n";
	$hashedPassword2 = crc31_hash(0xBABE, $str);
//	echo "got seed str= $str hash2=$hashedPassword2 <br>\n";
	$hashedPassword = $hashedPassword1.$hashedPassword2;
//	echo "got seed h1h2=$hashedPassword <br>\n";
	$hashedPassword = password_hash($hashedPassword, PASSWORD_BCRYPT);
//	echo "final = $hashedPassword <br>\n";
	
	return $hashedPassword;
}


function SendSMSSingleNumber($to, $message)
{
	global $err, $errId, $DebugExecution;

	// Initialize cURL session
	$ch = curl_init('https://api.twilio.com/2010-04-01/Accounts/' . const_twilioAccountSid . '/Messages.json');

	// Set cURL options
	curl_setopt($ch, CURLOPT_POST, 1);
	curl_setopt($ch, CURLOPT_POSTFIELDS, http_build_query([
		'To' => $to,
		'From' => const_twillioPhoneNumber,
		'Body' => $message,
	]));
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_USERPWD, const_twilioAccountSid . ':' . const_twilioAuthToken);
	curl_setopt($ch, CURLOPT_CAINFO, realpath(dirname(__FILE__)).'\\curl-ca-bundle.crt');
//	curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
//	curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);
	
	if(isset($DebugExecution) && $DebugExecution === 1)
	{
		curl_setopt($ch, CURLOPT_VERBOSE, true);
	}

	// Execute the cURL session
	$response = curl_exec($ch);
	if ($response === false) 
	{
		$err = 'cURL error: ' . curl_error($ch) . ' (' . curl_errno($ch) . ')';
		$errId = CURL_ERROR_GENERIC;
	}	

	if($errId === NO_ERR)
	{
		//echo "response : ". var_dump($response); echo "<br>";

		// Check the HTTP response code to determine success
		$httpStatus = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		if ($httpStatus != 201) {
			$err = 'Failed to send SMS. HTTP Status: ' . $httpStatus;
			$err .= 'Response: ' . $response;
			$errId = CURL_ERROR_GENERIC;
		}

		// Close the cURL session
		curl_close($ch);
	}	
}

function force_gzip_page_content()
{
    // Ensures only forced if the Accept-Encoding header contains "gzip"
    if (!isset($_SERVER['HTTP_ACCEPT_ENCODING']) || // client does not care about encoding ?
			substr_count($_SERVER['HTTP_ACCEPT_ENCODING'], 'gzip'))
    {
        header('Content-Encoding: gzip');
        ob_start('ob_gzhandler');
    }
}
?>