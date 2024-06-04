<?php
require_once('../NotHosted/header_include.php');

GenEndpointDefines();

if(!isset($_POST["SelectedRoleId"]))
{
	$roleOptions = executeMySQLQuery("Select RoleID,RoleName from UserRoleDefines order by RoleID asc");
	?>
    <h1>Role API Endpoint Rights Editor - internal use only !</h1>
    <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="POST">
        <label for="roleId">Role ID:</label>
        <select name="SelectedRoleId" required>
            <?php
            foreach ($roleOptions as $roleIndex => $row) {
				$roleId = $row['RoleID'];
				$roleName = $row['RoleName'];
                echo "<option value=\"$roleId\">$roleName</option>";
            }
            ?>
        </select><br>
        <input type="submit" value="Select role to edit">
    </form>
	<?php
}
else if(!isset($_POST["SaveChanges"]))
{
	$rightOptions = executeMySQLQuery("Select EndpointDefineID,EndpointDefineName,EndpointDefineDescription from ApiEndpointDefines order by EndpointDefineID asc");
	$endpointsSelected = executeMySQLQuery("Select EndpointDefineID from RoleEndpointRights where RoleID=".$_POST["SelectedRoleId"]);
	?>
    <h1>Role API Endpoint Rights Editor - internal use only !</h1>
    <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="POST">
        <label>API Endpoint Rights Assigned:</label><br>
        <?php
		//echo "Editing role id ".$_POST["SelectedRoleId"]."<br>";
        foreach ($rightOptions as $rowIndex => $row) {
			if(!isset($row['EndpointDefineID']))
			{
				continue;
			}
			$EndpointDefineID = $row['EndpointDefineID'];
			$EndpointDefineName = $row['EndpointDefineName'];
			$checked = "";
			if(IsRightSet($EndpointDefineID,$endpointsSelected))
			{
				$checked  = "checked=true";
			}
            echo '<label><input type="checkbox" name="r_'.$EndpointDefineID.'" value="' . $EndpointDefineID . '" '.$checked.' > ' . $EndpointDefineName . '</label><br>';
        }
		?>
        <br>
		<input type="hidden" name="SelectedRoleId" value="<?php echo $_POST["SelectedRoleId"]; ?>" >
		<input type="hidden" name="SaveChanges" value="1" >
        <input type="submit" value="Apply role right selection">
    </form>	
	<?php
}
else if(isset($_POST["SelectedRoleId"]) && isset($_POST["SaveChanges"]))
{
	// convert all post vars into simple vars. Internal use only for the sake of simpler code
	foreach($_POST as $key => $val) 
	{
		$$key = $val;
	}
	
	$rightOptions = executeMySQLQuery("Select EndpointDefineID from ApiEndpointDefines order by EndpointDefineID asc");
	$query_del = "";
	$query_add = "";

	foreach ($rightOptions as $rowIndex => $row) {
		if(!isset($row['EndpointDefineID']))
		{
			continue;
		}
		$EndpointDefineID = $row['EndpointDefineID'];
		$varName = "r_$EndpointDefineID";
		if( isset($$varName) )
		{
			$query_add .= ",($SelectedRoleId,$EndpointDefineID)";
		}
		else
		{
			$query_del .= ",$EndpointDefineID";
		}
	}
	
	if($query_del != "" )
	{
		$query_del = substr($query_del, 1);
		$query_del = "DELETE from RoleEndpointRights where RoleID = $SelectedRoleId and EndpointDefineID in ($query_del)";
		$delRes = executeMySQLQuery($query_del);
		if($delRes[0]['res'] === true)
		{
			echo "deleted rights using query : $query_del<br>";
		}
	}

	if($query_add != "" )
	{
		$query_add = substr($query_add,1);
		$query_add = "insert ignore into RoleEndpointRights values $query_add";
		$addRes = executeMySQLQuery($query_add);
		if($addRes[0]['res'] === true)
		{
			echo "added rights using query : $query_add<br>";
		}
	}
}

function IsRightSet($EndpointDefineID,$endpointsSelected)
{
	foreach($endpointsSelected as $rowIndex => $row )
	{
		if(!isset($row['EndpointDefineID']))
		{
			continue;
		}
		if($row['EndpointDefineID'] === $EndpointDefineID)
		{
			return true;
		}
	}
	return false;
}

function GenEndpointDefines()
{
	$phpFiles = glob('../*.php');
	$endpoints = "";
    foreach ($phpFiles as $phpFile) {
		$phpFile = str_replace("../","",$phpFile);
		$endpoints .= ",('$phpFile')";
    }
	$endpoints = substr($endpoints,1);
    executeMySQLQuery("insert ignore into ApiEndpointDefines (EndpointDefineName) values $endpoints");
}

require_once('../NotHosted/footer_include.php');
?>