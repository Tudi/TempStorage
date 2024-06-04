<?php
require_once('../NotHosted/header_include.php');

if(!isset($_POST["username"]))
{
	$roleOptions = executeMySQLQuery("Select RoleID,RoleName from UserRoleDefines order by RoleID asc");
	$organizationOptions = executeMySQLQuery("Select OrganizationID,Name from Organizations order by Name asc");
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>User Create User Form</title>
</head>
<body>
    <h1>User Registration - internal use only !</h1>
    <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="POST">

        <label for="roleId">Role ID:</label>
        <select name="roleId" required>
            <?php
            foreach ($roleOptions as $roleIndex => $row) {
				$roleId = $row['RoleID'];
				$roleName = $row['RoleName'];
                echo "<option value=\"$roleId\">$roleName</option>";
            }
            ?>
        </select><br>
		
        <label for="organizationId">Organization ID:</label>
        <select name="organizationId">
            <?php
            foreach ($organizationOptions as $rowIndex => $row) {
				$orgId = $row['OrganizationID'];
				$orgName = $row['Name'];
                echo "<option value=\"$orgId\">$orgName</option>";
            }
            ?>
        </select><br>

        <label for="username">Username:</label>
        <input type="text" name="username" required><br>

        <label for="firstName">First Name:</label>
        <input type="text" name="firstName" required><br>

        <label for="lastName">Last Name:</label>
        <input type="text" name="lastName" required><br>

        <label for="password">Password:</label>
        <input type="password" name="password" required><br>

        <label for="photo">Photo:</label>
        <input type="text" name="photo"><br>

        <label for="dateOfBirth">Date of Birth:</label>
        <input type="datetime-local" name="dateOfBirth"><br>

        <label for="jobRole">Job Role:</label>
        <input type="text" name="jobRole"><br>

        <label for="email">Email:</label>
        <input type="email" name="email"><br>

        <label for="recoveryEmail">Recovery Email:</label>
        <input type="email" name="recoveryEmail"><br>

        <label for="officePhoneNumber">Office Phone Number:</label>
        <input type="tel" name="officePhoneNumber"><br>

        <label for="mobilePhoneNumber">Mobile Phone Number:</label>
        <input type="tel" name="mobilePhoneNumber"><br>

        <label for="timezoneSeconds">Timezone Seconds:</label>
        <input type="number" name="timezoneSeconds"><br>
		
        <input type="submit" value="Register">
    </form>
</body>
</html>

<?php
}
else
{
	// convert all post vars into simple vars. Internal use only for the sake of simpler code
	foreach($_POST as $key => $val) 
	{
		$$key = $val;
	}
	
	$hashedPassword = password_hash(const_DB_password, PASSWORD_BCRYPT);
	
	$ret = executeMySQLQuery("Insert into users 
		(roleId, organizationId, username, firstName, lastName,
		password, photo, dateOfBirth, jobRole, email,
		officePhoneNumber, mobilePhoneNumber, timezoneSeconds, recoveryEmail) values 
		(?,?,?,?,?,
		?,?,?,?,?,
		?,?,?, ?)",
		$roleId, $organizationId, const_DB_username, $firstName, $lastName, 
		$hashedPassword, $photo, $dateOfBirth, $jobRole, $email,
		$officePhoneNumber, $mobilePhoneNumber, $timezoneSeconds, $recoveryEmail);
		
	if($ret[0]['res'] === true)
	{
		echo "User created successfully";
	}
	else
	{
		echo "There was an error while creating the user";
	}
}

require_once('../NotHosted/footer_include.php');
?>