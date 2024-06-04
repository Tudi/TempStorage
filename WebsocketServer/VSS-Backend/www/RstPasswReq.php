<?php
require_once('NotHosted/header_include.php');

// this API call only makes sense if we received this param
$EndpointSpecificColumns=array("email");
$genericRequiredFields=array("AppVer", "ClientEndpoint");
CheckRequiredFields($_POST,array_merge($EndpointSpecificColumns, $genericRequiredFields));

// curl -X POST -d "email=jozsab1@gmail.com" http://localhost:8081/RstPasswReq.php

if($errId === NO_ERR)
{
    // Get the user's email address from the form
    $email = $_POST["email"];
	
    // Generate a random temporary password
    $newPassw = bin2hex(random_bytes(8));
    $callbackId = bin2hex(random_bytes(8));

	// check if this user exists
	$rows = executeMySQLQuery("Select UserId,FirstName,LastName from Users where RecoveryEmail=?", $email);
	if(isset($rows[0]['UserId']))
	{	
		// create a queue entry
		$UserId = $rows[0]['UserId'];
		$FullName = $rows[0]['FirstName']." ".$rows[0]['LastName'];
		$rows = executeMySQLQuery("insert into PsswRstQueue (Email,UserID,CallbackId,NewPassw) values (?,$UserId,'$callbackId','$newPassw')", $email);
		if(isset($rows[0]['id']))
		{
			$reqId = $rows[0]['id'];
			// Load the HTML email content from an HTML file
			$htmlContent = file_get_contents("./NotHosted/PasswResetTemplate.html");
			$rstConfirmLink = $serverHost."/RstPasswConf.php?id=$reqId&cid=$callbackId&uid=$UserId";

			// Replace placeholders in the HTML content with actual values
			$htmlContent = str_replace("{new_password}", $newPassw, $htmlContent);
			$htmlContent = str_replace("{user_name}", $FullName, $htmlContent);
			$htmlContent = str_replace("{passw_reset_link}", $rstConfirmLink, $htmlContent);

			// Send the email
			$to = $email;
			$subject = "Password Reset";
			$headers = "From: ".const_passwResetEmailSender."\r\n";
			$headers .= "MIME-Version: 1.0\r\n";
			$headers .= "Content-Type: text/html; charset=UTF-8\r\n";

			if (mail($to, $subject, $htmlContent, $headers)) 
			{
				LogMsg("Passw reset email sent");
				$ret['res'] = 1;
			} 
			else 
			{
				$ret['ErrorId'] = FAILED_TO_SEND_EMAIL;
			}
		}
		else
		{
			$ret['ErrorId'] = FAILED_TO_CREATE_DB_ENTRY;
		}
	}
	else
	{
		$ret['ErrorId'] = INVALID_RECOVERY_EMAIL;
	}
}
else
{
	$ret['ErrorId'] = MISSING_EMAIL_FIELD;
}

// disconnect sql, format and send reply
require_once('NotHosted/footer_include.php');
?>
