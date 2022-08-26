<?php
ini_set('display_errors', 1); ini_set('display_startup_errors', 1); error_reporting(E_ALL);
require '../vendor/autoload.php';
use Aws\S3\S3Client;
use Aws\Lambda\LambdaClient;

// Instantiate an Amazon S3 client.
$s3Client = new S3Client([
	'version' => 'latest',
	'region'  => 'us-east-1',
	'credentials' => [
	'key'    => 'asdf',
	'secret' => 'asdfa/E7+Bvg2+3pNfjUMezULFE8s1'
	]
	]);
$bucket = 'originalnftimages';

// Check if the form was submitted
if($_SERVER["REQUEST_METHOD"] == "POST")
{
	// Check if file was uploaded without errors
	if(isset($_FILES["anyfile"]) && $_FILES["anyfile"]["error"] == 0)
	{
		$allowed = array("jpg" => "image/jpg", "jpeg" => "image/jpeg", "gif" => "image/gif", "png" => "image/png");
		$filename = $_FILES["anyfile"]["name"];
		$filetype = $_FILES["anyfile"]["type"];
		$filesize = $_FILES["anyfile"]["size"];
		// Validate file extension
		$ext = pathinfo($filename, PATHINFO_EXTENSION);
		if(!array_key_exists($ext, $allowed)) 
			die("Error: Please select a valid file format.");
		// Validate file size - 10MB maximum
		$maxsize = 10 * 1024 * 1024;
		if($filesize > $maxsize) 
			die("Error: File size is larger than the allowed limit.");
		// Validate type of the file
		if(in_array($filetype, $allowed))
		{
			// Check whether file exists before uploading it
//			if(file_exists("upload/" . $filename))
//			{
//				echo $filename . " is already exists.";
//			} 
//			else
			{
				if(move_uploaded_file($_FILES["anyfile"]["tmp_name"], "upload/" . $filename))
				{
					$file_Path = __DIR__ . '/upload/'. $filename;
//					$key = basename($file_Path);
					$key = createDBRow(basename($file_Path));
					if($key<0)
					{
						echo "Failed to create entry in DB";
					}
					else
					{
						try 
						{
							$result = $s3Client->putObject([
								'Bucket' => $bucket,
								'Key'    => $key,
								'Body'   => fopen($file_Path, 'r'),
								'ACL'    => 'public-read', // make file 'public'
								]);
							echo "Image uploaded successfully. Image path is: ". $result->get('ObjectURL');
						} 
						catch (Aws\S3\Exception\S3Exception $e) 
						{
							echo "There was an error uploading the file.\n<br>";
							echo $e->getResponse()->getBody()->getContents();
							echo "====\n<br>";
							echo $e->getMessage();
							die();
						}
						if(processImageFromS3($key)==0)
						{
							echo "final result is : { RequestCallbackParams : \"\", Registered : True }";
						}
						else
						{
							echo "final result is : { RequestCallbackParams : \"\", Registered : True }";
						}
					}  
//					echo "Your file was uploaded successfully.";
				}
				else
				{
					echo "File is not uploaded";
				}
			} 
		} 
		else
		{
			echo "Error: There was a problem uploading your file. Please try again."; 
		}
	} 
	else
	{
		echo "Error: " . $_FILES["anyfile"]["error"];
	}
}
else 
{
//	phpinfo();
}

function getIP()
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

function createDBRow($filename)
{
	$ret = -1;
	$con=mysqli_connect("icom", "sdfg", "asdf.Rev3al", "imageinfo");
	// Check connection
	if (mysqli_connect_errno())
	{
		echo "Failed to connect to MySQL: " . mysqli_connect_error();
		return -1;
	}
	
	$sql = "insert into RegisteredImages (FileName,PeerIp) values (
	'".mysqli_real_escape_string($con, $filename)."',
	'".mysqli_real_escape_string($con, getIP())."')";
	$result = mysqli_query($con, $sql);
	if($result != false)
	{
		$ret = mysqli_insert_id($con);
	}
	else
	{
		echo "There was an sql error :".mysqli_error($con);
		$ret = -2;
	}
	
	mysqli_close($con);
	
	return $ret;
}

function processImageFromS3($imgName)
{
	$client = LambdaClient::factory(
		array(
			'credentials' => array(
				'key' => 'asdf',
				'secret' => 'asdf/E7+Bvg2+3pNfjUMezULFE8s1'
			),
			'version' => 'latest',
			'region'  => 'us-east-1'
		)
	);
		
	$lambda_params = "{
	  \"Records\": [
		{
		  \"s3\": {
			\"bucket\": {
			  \"name\": \"originalnftimages\"
			},
			\"object\": {
			  \"key\": \"".$imgName."\"
			}
		  }
		}
	  ]
	}";
	//$lambda_params = json_encode($lambda_params);

//	echo "json param : ".$lambda_params."<br>";
		
	$result = $client->invoke([
	  'FunctionName' => 'HashOriginalNFTImage',
	  'Payload' => $lambda_params,
	  'InvokeArgs' => $lambda_params,
	]);

	if($result['StatusCode'] == 200)
	{
//		echo "Lambda run";
		if($result['FunctionError'] == "")
		{
//			echo "values should be updated in DB";
			return 0;
		}
	}
//	echo "result is ".$result."<br>";
	/*	
	$result = $client->invokeAsync(array(
		// FunctionName is required
		'FunctionName' => 'HashOriginalNFTImage',
		// InvokeArgs is required
		'InvokeArgs' => $lambda_params,
		'Payload' => $lambda_params,
	));*/
//	echo "all done<br>";
	return 1;
}

?>
