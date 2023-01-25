<?php

function hashImageOnEC2($FileNamePath)
{
	global $HASH_APP_PATH, $_REQUEST;
	$OriginalFilename = $_FILES["ImgFile"]["name"];
	$cmd = $HASH_APP_PATH." 1 ".$FileNamePath." \"".$OriginalFilename."\"";
	DebugPring("Executing : ".$cmd);
	$ret = shell_exec($cmd);
	DebugPring("Exec result : ".$ret);
	return $ret;
}

function getImageHashes()
{
	$tempFileName = getUploadedFileName();
	if($tempFileName == "")
	{
		DebugPring("Failed to obtain uploaded image." );
		return "";
	}
	$hashesJSON = hashImageOnEC2($tempFileName);
	if($hashesJSON == "" || strpos($hashesJSON, "PHASH") == false)
	{
		DebugPring("Failed to obtain image hash." );
		return "";
	}
	$hashes = json_decode($hashesJSON, true);
	return $hashes;
}
?>