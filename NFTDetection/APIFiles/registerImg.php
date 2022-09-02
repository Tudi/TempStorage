<?php
ini_set('display_errors', 1); ini_set('display_startup_errors', 1); error_reporting(E_ALL);
header("Access-Control-Allow-Origin: *");
require '../vendor/autoload.php';
require 'DebugMsg.func';
require 'settings.cfg';
require 'UploadImg.func';
require 'DB.func';
require 'lambda.func';

// Check if we got all the required params to process this request
if(!isset($_REQUEST['OriginalImgUrl']) || strlen($_REQUEST['OriginalImgUrl'])==0)
{
	DebugPring("Expected OriginalImgUrl param is missing value.\n<br>" );
	die();
}

if(!isset($_REQUEST['ImgArtist']) || strlen($_REQUEST['ImgArtist'])==0)
{
	DebugPring("Expected ImgArtist param is missing value.\n<br>" );
	die();
}

if(!isset($_REQUEST['ImgTitile']) || strlen($_REQUEST['ImgTitile'])==0)
{
	DebugPring("Expected ImgTitile param is missing value.\n<br>" );
	die();
}

if(!isset($_REQUEST['BlockChainHash']) || strlen($_REQUEST['BlockChainHash'])==0)
{
	DebugPring("Expected BlockChainHash param is missing value.\n<br>" );
	die();
}

if(!isset($_REQUEST['APIAccessToken']) || strlen($_REQUEST['APIAccessToken'])==0)
{
	DebugPring("Expected APIAccessToken param is missing value.\n<br>" );
	phpinfo();
	die();
}

// store the file locally
$uploadedFile = moveUploadedToLocalDir();
if( $uploadedFile == "" )
{
	die();
}
$fileNameOnly = basename($uploadedFile);

// create a register row in DB
$dbRowId = createDBRowForRegister($fileNameOnly, $_REQUEST['OriginalImgUrl'], $_REQUEST['BlockChainHash'], $_REQUEST['APIAccessToken'], $_REQUEST['ImgArtist'], $_REQUEST['ImgTitile']);
if($dbRowId <= 0)
{
	die();
}

// in order to process an image, lambda needs to have access to it
if( uploadImageToS3($uploadedFile, $dbRowId) != 0)
{
	die();
}

// obtain the hashes for the image using lambda
$hashesJSON = processImageFromS3($dbRowId);
if($hashesJSON == "")
{
	die();
}
$hashes = json_decode($hashesJSON, true);

// update DB row with the hash values
$rowAccessToken=updateDBRowHashes($dbRowId, $hashes["AHASH"], $hashes["PHASH"]);

echo "{ \"Registered\":\"True\",\"RequestCallbackParams\":".$_REQUEST['CallbackUrl']."\",\"TransactionId\":\"".$rowAccessToken."\"}";
?>
