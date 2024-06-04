<?php
require_once('../NotHosted/header_include.php');

if(!isset($_POST["SelectedRoleId"]))
{
	$roleOptions = executeMySQLQuery("Select RoleID,RoleName from UserRoleDefines order by RoleID asc");
	?>
    <h1>Role Rights Editor - internal use only !</h1>
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
	$rightOptions = executeMySQLQuery("Select RightID,RightName,RightDescription from RoleRightDefines order by RightID asc");
	$rightsSelected = executeMySQLQuery("Select RightID from RoleRights where RoleID=".$_POST["SelectedRoleId"]);
	?>
    <h1>Role Rights Editor - internal use only !</h1>
    <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="POST">
        <label>Rights assigned:</label><br>
        <?php
		//echo "Editing role id ".$_POST["SelectedRoleId"]."<br>";
        foreach ($rightOptions as $rowIndex => $row) {
			$RightID = $row['RightID'];
			$RightName = $row['RightName'];
			$checked = "";
			if(IsRightSet($RightID,$rightsSelected))
			{
				$checked  = "checked=true";
			}
            echo '<label><input type="checkbox" name="r_'.$RightID.'" value="' . $RightID . '" '.$checked.' > ' . $RightName . '</label><br>';
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
	
	$rightOptions = executeMySQLQuery("Select RightID from RoleRightDefines order by RightID asc");
	$query_del = "";
	$query_add = "";

	foreach ($rightOptions as $rowIndex => $row) {
		$RightID = $row['RightID'];
		$varName = "r_$RightID";
		if( isset($$varName) )
		{
			$query_add .= ",($SelectedRoleId,$RightID)";
		}
		else
		{
			$query_del .= ",$RightID";
		}
	}
	
	if($query_del != "" )
	{
		$query_del = substr($query_del, 1);
		$query_del = "DELETE from RoleRights where RoleID = $SelectedRoleId and RightID in ($query_del)";
		$delRes = executeMySQLQuery($query_del);
		if($delRes[0]['res'] === true)
		{
			echo "deleted rights using query : $query_del<br>";
		}
	}

	if($query_add != "" )
	{
		$query_add = substr($query_add,1);
		$query_add = "insert ignore into RoleRights values $query_add";
		$addRes = executeMySQLQuery($query_add);
		if($addRes[0]['res'] === true)
		{
			echo "added rights using query : $query_add<br>";
		}
	}
}

function IsRightSet($RightID,$rightsSelected)
{
	foreach($rightsSelected as $rowIndex => $row )
	{
		if($row['RightID'] === $RightID)
		{
			return true;
		}
	}
	return false;
}

require_once('../NotHosted/footer_include.php');
?>