<?php
// Database configuration
const const_DB_servername = "127.0.0.1"; 
const const_DB_username = "vss_dashboard";
const const_DB_password = "vss_dashboard";
const const_DB_database = "vss_dashboard";

const const_loginSessionExpiresAfter = 60*60*23; // how many seconds is a user session valid ?
const const_passwResetEmailSender = "1";
const const_EmailNotificationSender = "1";
const const_passwResetLinkAvailableSec = 60*10; // how many seconds is a passw reset link valid ?
//const minPasswLen = 4; // 4 characters
const const_MFASenderEmail = "1";
const const_MFAExpiresAfter = 60*2;
const const_MFADigitCount = 6;

const const_twillioPhoneNumber = '+1'; 
const const_twilioAccountSid = '1';
const const_twilioAuthToken = '1';
	
$protocol = isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? 'https' : 'http';
$domain = $_SERVER['HTTP_HOST'];// based on PHP conf this might not be available automatically
$serverHost = $protocol . '://' . $domain;

$DebugExecution = 1;
//$DebugForcedLatency = 0; // force X seconds latency on API calls to see how the UI behaves
?>