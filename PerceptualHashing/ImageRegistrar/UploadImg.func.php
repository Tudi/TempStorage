<?php

function uploadImageToIPFS($srcFileName)
{
	global $PINATA_API_KEY,$PINATA_API_SECRET_KEY,$IPFS_BASE_URL;
	$target_url = "https://api.pinata.cloud/pinning/pinFileToIPFS";
	$file_name_with_full_path = $srcFileName;

    $cFile = new CURLFile($file_name_with_full_path, "image/png", basename($file_name_with_full_path));
	$headers = array('pinata_api_key: '.$PINATA_API_KEY,'pinata_secret_api_key: '.$PINATA_API_SECRET_KEY,'Content-Type: multipart/form-data');
	$post = array("pinataOptions"=>"{\"cidVersion\": 1}",
		'pinataMetadata'=>"{\"name\": \"".basename($file_name_with_full_path)."\", \"keyvalues\": {\"company\": \"Rev3al\"}}",
		'file'=> $cFile);

	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $target_url);
	curl_setopt($ch, CURLOPT_POST, 1);
	curl_setopt($ch, CURLOPT_POSTFIELDS, $post);
	curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0);
	curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
	curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	
	$curl_output = curl_exec ($ch);
	$curl_err = curl_error($ch);
	
	if($curl_err > 0)
	{
		DebugPring( curl_errno($ch).":".curl_error($ch) );
		$ret = "";
	}
	else
	{
		$IPFS_reply_array = json_decode($curl_output, true);
		if( !isset($IPFS_reply_array['IpfsHash']) || strlen($IPFS_reply_array['IpfsHash'])==0)
		{
			DebugPring( "Could not parse Pinata reply : ".$curl_output );
			return "";
		}
		$hash = $IPFS_reply_array['IpfsHash'];
		$file_url = $IPFS_BASE_URL.$hash;
		$timestamp = $IPFS_reply_array['Timestamp'];
		
		// yeah, it's the same
		$ret['IpfsHash'] = $hash;
		$ret['URL'] = $file_url;
		$ret['Timestamp'] = $timestamp;
	}
	
	curl_close ($ch);
	
	return $ret;
}

function getUploadedRealFileName()
{
	return $_FILES["ImgFile"]["name"];
}

function getUploadedFileName()
{
	global $HASH_MAX_FILE_SIZE, $HASH_MAX_RESOLUTION;
	global $LastErrorReason;
	// Check if the form was submitted
	if($_SERVER["REQUEST_METHOD"] == "POST")
	{
		// Check if file was uploaded without errors
		if(isset($_FILES["ImgFile"]))
		{
			if($_FILES["ImgFile"]["error"] == 0)
			{
				$allowed = array("jpg" => "image/jpg", "jpeg" => "image/jpeg", "gif" => "image/gif", "png" => "image/png");
				$filename = $_FILES["ImgFile"]["name"];
				$filetype = $_FILES["ImgFile"]["type"];
				$filesize = $_FILES["ImgFile"]["size"];
				if( $filesize > $_FILES["ImgFile"]["size"] )
				{
					$LastErrorReason = "File too large. Received ".$filesize." max allowed ".$HASH_MAX_FILE_SIZE;
					DebugPring($LastErrorReason);
					return "";
				}
				list($width, $height, $type, $attr) = getimagesize($_FILES["ImgFile"]["tmp_name"]);				
				if($width * $height >= $HASH_MAX_RESOLUTION)
				{
					$LastErrorReason = "File resolution too large. Received ".$width."*".$height." max allowed ".$HASH_MAX_RESOLUTION;
					DebugPring($LastErrorReason);
					return "";
				}
				// Validate file extension
				$ext = pathinfo($filename, PATHINFO_EXTENSION);
				if(!array_key_exists($ext, $allowed)) 
				{
					$LastErrorReason = "Not a valid file format. Allowed : jpg, jpeg, gif, png";
					DebugPring($LastErrorReason);
					return "";
				}
				
				return $_FILES["ImgFile"]["tmp_name"];
			}
			else
			{
				$LastErrorReason = "File upload error : ".$_FILES["ImgFile"]["error"];
				DebugPring( "Error ".$_FILES["ImgFile"]["error"]." happened while uploading image file." );
			}
		}
		else
		{
			$LastErrorReason = "Missing 'ImgFile' POST param. Expecting 'multipart/form-data' file.";
			DebugPring( $LastErrorReason );
		}
	}
	else
	{
		$LastErrorReason = "Expecting POST method, received ".$_SERVER["REQUEST_METHOD"];
		DebugPring( $LastErrorReason );
	}
	return "";
}

function moveUploadedToMountedVolume($tmpFileName, $NewFileName)
{
	global $EBFS_PATH;
	// Check whether file exists before uploading it
	$file_Path = $EBFS_PATH . $NewFileName;
	if(file_exists($file_Path))
	{
		DebugPring($file_Path . " already exists. Abort overwrite on mounted volume");
		return;
	} 
	if(move_uploaded_file($tmpFileName, $file_Path))
	{
		DebugPring("File was moved successfully to mounted volume.");
		return $file_Path;
	}
	else
	{
		DebugPring("Failed to move uploaded file to mounted volume");
	}
	return "";
}

function getFileContentAsBase64($srcFile)
{
	$data = file_get_contents($srcFile);
	return base64_encode($data);
}

?>