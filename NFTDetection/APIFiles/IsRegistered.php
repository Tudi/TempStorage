<?php
ini_set('display_errors', 1); ini_set('display_startup_errors', 1); error_reporting(E_ALL);
header("Access-Control-Allow-Origin: *");
require '../vendor/autoload.php';
require 'DebugMsg.func';
require 'settings.cfg';
require 'UploadImg.func';
require 'DB.func';
require 'lambda.func';

// store the file locally
$uploadedFile = moveUploadedToLocalDir();
if( $uploadedFile == "" )
{
	die();
}
$fileNameOnly = basename($uploadedFile);

$temp_name = "hash_temp";

// in order to process an image, lambda needs to have access to it
if( uploadImageToS3($uploadedFile, $temp_name) != 0)
{
	die();
}

// obtain the hashes for the image using lambda
$hashesJSON = processImageFromS3($temp_name);
if($hashesJSON == "")
{
	die();
}
$hashes = json_decode($hashesJSON, true);

$registerdJSON = checkRegistered($hashes["AHASH"], $hashes["PHASH"], 80, 5);

echo $registerdJSON;
?>