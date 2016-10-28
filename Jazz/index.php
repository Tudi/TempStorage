<?php
session_start();
print_r( $_SESSION );

$_SESSION['j_username'] = "roijoz";
$_SESSION['j_password'] = "waters2017";

//"https://clm1.waters.com:443/ccm/rpt/repository/foundation?fields=foundation/developmentLine[name=%27Acquisition%27]/currentIteration/(itemId|startDate|endDate|name)"
//$response = GetUrl( "https://clm1.waters.com:443/ccm/rpt/repository/foundation?fields=foundation/developmentLine[name=%27Acquisition%27]/currentIteration/(itemId|startDate|endDate|name)", "", 0 );
//var_dump($response);

$response = GetUrl( "https://clm1.waters.com/ccm/web/projects/j_security_check?j_username=roijoz&j_password=waters2017", "", 0 );
//$response = GetUrl( "https://clm1.waters.com/ccm/web/projects/j_security_check", "j_username=roijoz&j_password=waters2017", 1 );
var_dump($response);

//https://clm1.waters.com/ccm/service/com.ibm.team.repository.service.internal.webuiInitializer.IWebUIInitializerRestService/initializationData
//https://clm1.waters.com/ccm/rootservices

//$response = GetUrl( "https://clm1.waters.com:443/ccm/rpt/repository/foundation?fields=foundation/developmentLine[name=%27Acquisition%27]/currentIteration/(itemId|startDate|endDate|name)", "", 0, $response['coockies'] );
$response = GetUrl( "https://clm1.waters.com/ccm/web/projects/Empower#action=com.ibm.team.workitem.runSavedQuery&id=_eGFxwA0iEeae-L0ICLrO5A", "", 0, $response['coockies'] );
var_dump($response);

//$response = GetUrl( "https://clm1.waters.com/ccm/web/projects/Empower" );
//var_dump($response);

function UpdateCookie( $OldCoockies, $NewCookies )
{
	if( $OldCoockies == "" )
		return $NewCookies;
	if( $NewCookies == "" )
		return $OldCoockies;
	$parts1 = explode( "; ", $OldCoockies );
	$parts2 = explode( "; ", $NewCookies );
	foreach( $parts1 as $key => $val )
	{
		$splitpos = strpos( $val, "=" );
		$ckey = substr( $val, 0, $splitpos + 1 );
		$cval = substr( $val, $splitpos + 1 );
		$merged[$ckey] = $cval;
	}
	foreach( $parts2 as $key => $val )
	{
		$splitpos = strpos( $val, "=" );
		$ckey = substr( $val, 0, $splitpos + 1 );
		$cval = substr( $val, $splitpos + 1 );
		$merged[$ckey] = $cval;
	}
	$all = "";
	foreach( $merged as $key => $val )
	{
		if( $all != "" )
			$all .= "; ";
		$all .= $key.$val;
	}
	
	return $all;
}

function GetUrl( $url, $query = "", $IsPost = 0, $Cookies = "" )
{
//	global $ch;
	$ch = curl_init();
	
	if( $IsPost )
	{
		curl_setopt($ch, CURLOPT_POST, 1);
		if( strlen( $query ) > 0 )
			curl_setopt($ch, CURLOPT_POSTFIELDS, $query );		
	}
	else if( strlen( $query ) > 0 )
		$url .= "?".$query;
	
	if( strlen( $Cookies ) > 0 )
		curl_setopt( $ch, CURLOPT_COOKIE, $Cookies );
//	$tmpfname = dirname(__FILE__).'/'.$_COOKIE['PHPSESSID'].'.txt';
//	curl_setopt( $ch, CURLOPT_COOKIESESSION, true );
//	curl_setopt( $ch, CURLOPT_COOKIEJAR, $tmpfname );
//	curl_setopt( $ch, CURLOPT_COOKIEFILE, $tmpfname );
		
	curl_setopt($ch, CURLOPT_URL,$url);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
	curl_setopt($ch, CURLOPT_VERBOSE, true);
	curl_setopt($ch, CURLOPT_HEADER, 1);
//	curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.0.3705; .NET CLR 1.1.4322)');	
	curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
	curl_setopt($ch, CURLOPT_MAXREDIRS, 5);
	curl_setopt($ch, CURLOPT_TIMEOUT, 15);
	
	$headers = array
	(
		'Host: clm1.waters.com',
		'Connection: keep-alive',
		'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
		'Upgrade-Insecure-Requests: 1',
		'User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2623.112 Safari/537.36',
		'Accept-Language: en-US,en;q=0.8'
	); 

	curl_setopt($ch, CURLOPT_HTTPHEADER, $headers );
	curl_setopt($ch, CURLOPT_USERPWD, "roijoz:waters2017");
	curl_setopt($ch, CURLOPT_UNRESTRICTED_AUTH, 1);
	
	$server_output['page'] = curl_exec ($ch);
	$server_output['info'] = curl_getinfo($ch);
	$server_output['errno'] = curl_errno($ch);
	
	$header_size = curl_getinfo($ch, CURLINFO_HEADER_SIZE);
	$server_output['header'] = substr($server_output['page'], 0, $header_size);
	$server_output['page'] = substr($server_output['page'], $header_size);
	
	$pattern = "#Set-Cookie:\\s+(?<cookie>[^=]+=[^;]+)#m"; 
	preg_match_all($pattern, $server_output['header'], $matches); 
	$server_output['coockies'] = implode("; ", $matches['cookie']);
	
	$server_output['coockies'] = UpdateCookie( $Cookies, $server_output['coockies'] );
	
	curl_close ($ch);

	
	return $server_output;
}
?>