<?php
session_start();

// in case login form is required
include "LoginForm.php";

//perform SQT actions
foreach($_REQUEST as $key => $val )
{
//echo $key."<br>";
	if( strpos( "#".$key, "PerformSQT_Name_" ) == 1 )
	{
		$NodeName = $val;
		$DoIQ = $_REQUEST[ $NodeName."_IQ" ];
		$DoOQ = $_REQUEST[ $NodeName."_OQ" ];
		$DoOQMVM = $_REQUEST[ $NodeName."_OQMVM" ];
		if( $DoIQ == "" )
			$DoIQ = 0;
		if( $DoOQ == "" )
			$DoOQ = 0;
		if( $DoOQMVM == "" )
			$DoOQMVM = 0;
		$SQTResult = file_get_contents( "http://localhost:8081/PerformSQT?NodeName=$NodeName&DoIQ=$DoIQ&DoOQ=$DoOQ&DoOQMVM=$DoOQMVM" );
		//echo "DEBUG : $SQTResult<br>";
		$SQTResult_Array = json_decode( $SQTResult, true );
		//var_dump( $SQTResult_Array );	echo "<br>";
		$operation = 0;
		if( $DoIQ == 1 )
			$SQTResults[$NodeName]['DoIQ'] = $SQTResult_Array['Errors'][$operation++]['Error'];
		if( $DoOQ == 1 )
			$SQTResults[$NodeName]['DoOQ'] = $SQTResult_Array['Errors'][$operation++]['Error'];
		if( $DoOQMVM == 1 )
			$SQTResults[$NodeName]['DoOQMVM'] = $SQTResult_Array['Errors'][$operation++]['Error'];		
	}
}




//get the list of available projects and show "restore" option if no projects are present
$SQTCapability = file_get_contents( "http://localhost:8081/ProjectList" );
$SQTCapabilityObj = json_decode( $SQTCapability, true );
//echo "<br>SQT capabilities supported : $SQTCapability ";
//var_dump( $SQTCapabilityObj );
if( $SQTCapabilityObj['CanDoIQ'] == 0 || $SQTCapabilityObj['CanDoOQ'] == 0 || $SQTCapabilityObj['CanDoOQMVM'] == 0 )
{
	?>
		<a href="http://localhost:8081/Restore" target="blank" > Restore projects from CD </a>
	<?php
}







//get the list of available nodes
$SQTNodes = file_get_contents( "http://localhost:8081/NodeList" );
$SQTNodes_Array = json_decode( $SQTNodes, true );
//echo "<br>DEBUG : SQT nodelist : $SQTNodes ";

//create a list of "nodes" we wish to process
?>
<form name="PerformSQT" id="PerformSQT" action="<?php echo $_SERVER['PHP_SELF']; ?>">
	<table border=1>
		<tr>
			<td>Node Name</td>
			<td>IQ</td>
			<td>OQ</td>
			<td>OQMVM</td>
		</tr>
<?php
foreach( $SQTNodes_Array['NodeNameList'] as $key => $val )
{
	$NodeHasIQCap="";
	if( $SQTCapabilityObj['CanDoIQ'] == 0 )
		$NodeHasIQCap="disabled";
	$NodeHasOQCap="";
	if( $SQTCapabilityObj['CanDoOQ'] == 0 )
		$NodeHasOQCap="disabled";
	$NodeHasOQMVMCap="";
	if( $SQTCapabilityObj['CanDoOQMVM'] == 0 )
		$NodeHasOQMVMCap="disabled";
?>
		<tr>
			<td><?php echo $val;?><input type="hidden" name="PerformSQT_Name_<?php echo $val;?>" value="<?php echo $val;?>" ></td>
			<td><input type="checkbox" name="<?php echo $val;?>_IQ" value="1" <?php echo $NodeHasIQCap;?>><?php echo $SQTResults[$val]['DoIQ'];?></td>
			<td><input type="checkbox" name="<?php echo $val;?>_OQ" value="1" <?php echo $NodeHasOQCap;?>><?php echo $SQTResults[$val]['DoOQ'];?></td>
			<td><input type="checkbox" name="<?php echo $val;?>_OQMVM" value="1" <?php echo $NodeHasOQMVMCap;?>><?php echo $SQTResults[$val]['DoOQMVM'];?></td>
		</tr>
<?php
}
?>
		<tr>
			<td>
				<input type=submit value="Perform SQT" >
			</td>
		</tr>
	</table>
</form>
<?php
//var_dump( $SQTNodes_Array );
?>