<?php
$line="";
$ret="missing";
foreach($_REQUEST as $key => $val)
{
	$line .= $key ."=".$val.",";
	if(strcmp($key,"FP")==0)
		$FP = DecodeLicense($val);
}

//check if the license key exists

//send back the server a response so he knows the license is valid
$ret = EncodeLicense( $FP['fp'], $FP['k'] );

//this is just for the sake of debuging. Running the driver for a long period and checking consistency
$f=fopen("r.txt","at");
fputs($f,$line."\n\r");
if(isset($FP))
{
	fputs($f,"fp=".$FP['fp']."\n\r");
	fputs($f,$FP['k']."\n\r");
	fputs($f,$ret."\n\r");
}
fputs($f,"\n\r");
fclose($f);

echo $ret;

function DecodeLicense($str)
{
	$ret = "";
	$str=str_replace(' ','+',$str);
	$unc = base64_decode($str);
	$key = substr($unc,0,12);
	$key = (int)$key;
	$ret = substr($unc,12);
	$len= strlen($ret);
	$ret2['k']=$key;
	$keys[0] = ($key >> 0) & 0xFF;
	$keys[1] = ($key >> 8) & 0xFF;
	$keys[2] = ($key >> 16) & 0xFF;
	$keys[3] = ($key >> 24) & 0xFF;

/*	echo "str=$str<br>";
	echo "key=$key<br>";
	echo "unc=$unc<br>";
	echo "ret=$ret<br>";
	for($i=0;$i<$len;$i++)
		echo ord($ret[$i])." ";
	echo "<br>";
	print_r($keys);
	echo "<br>";*/

	for($i=0;$i<$len;$i++)
	{
		$key = $keys[$i % 4];
		$Byte = ord($ret[$i]);
		$Out = $Byte ^ $key;
		$c = chr($Out);
//echo "$Byte ^ $key = $Out = $c<br>";		
		$ret[$i] = $c;
	}
//	echo "ret=$ret<br>";
	$ret2['fp']=$ret;
	return $ret2;
}

function EncodeLicense($str, $k)
{
	$key = $k - 0x00001234;
	$keys[0] = ($key >> 0) & 0xFF;
	$keys[1] = ($key >> 8) & 0xFF;
	$keys[2] = ($key >> 16) & 0xFF;
	$keys[3] = ($key >> 24) & 0xFF;
	$len = strlen($str);
	
//	echo "key=$k<br>";
//	echo "str=$str<br>";
	
	$ret = "";
	for($i=0;$i<$len;$i++)
	{
		$key = $keys[$i % 4];
		$Byte = ord($str[$i]);
		$Out = $Byte ^ $key;
		$c = chr($Out);
//echo "$Byte ^ $key = $Out = $c<br>";		
		$ret .= $c;
	}
//	$str[$i] = $str[$i] ^ $key;
	$ret = base64_encode($ret);
//	echo "ret=$ret<br>";
	return $ret;
}
?>