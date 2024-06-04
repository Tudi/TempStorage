<?php
// Create a connection
if(isset($conn))
{
	LogMsg("SQL header is getting included more than once.");
}
else
{
	$conn = new mysqli(const_DB_servername, const_DB_username, const_DB_password, const_DB_database);

	// Check the connection
	if ($conn->connect_error) {
		die("Connection failed: " . $conn->connect_error);
	}
}


function executeMySQLQuery($query) {
	global $conn;
	
    // Get the number of arguments passed to the function
    $numArgs = func_num_args();
    
    if ($numArgs < 2) {
		// Initialize an array to store the results
		$resultArray = array();
		try {
			$result = $conn->query($query);
			
			if(is_bool($result) === true)
			{
				$resultArray[] = ['res' => $result, 'id' => $conn->insert_id ];
			}			
			else if ($result) {

				// Fetch the results into an associative array
				while ($row = mysqli_fetch_assoc($result)) {
					$resultArray[] = $row;
				}

				// Close the result set
				$result->close();
			} else {
				// Handle query errors. Should never get errors
				LogMsg("Error: " . $conn->error ." for query ".$query);
				$resultArray[] = ['res' => false];
			}
		} catch (Exception $e) {
			// Log the exception to a file or other destination
			LogMsg("Exception: " . $e->getMessage()." while executing the query ".$query);
			// Optionally, you can handle the exception gracefully here if needed
			$resultArray[] = ['res' => false];
		}
		
		return $resultArray;
    }
    
    // The first argument is the query, so we start binding variables from the second argument
    $params = array_slice(func_get_args(), 1);

    // Prepare the query
	try 
	{
		$stmt = $conn->prepare($query);
	} 
	catch (Exception $e) 
	{
		// Log the exception to a file or other destination
		LogMsg("Exception: " . $e->getMessage()." while executing the query ".$query);
		// Optionally, you can handle the exception gracefully here if needed
		$resultArray[] = ['res' => false];
		return $resultArray;
	}

    if ($stmt === false) {
        LogMsg("Error preparing the query: " . $conn->error);
		return [];
    }

   // Dynamically bind parameters based on the number of variables
    $types = "";
    $paramsToBind = [];

    foreach ($params as $param) {
        if (is_int($param)) {
            $types .= "i"; // Integer type
        } elseif (is_float($param)) {
            $types .= "d"; // Double (float) type
        } else {
            $types .= "s"; // String type (default)
        }
        $paramsToBind[] = $param;
    }

    // Use call_user_func_array to bind parameters dynamically
    $bind_params = array_merge([$types], $paramsToBind);
	// Create an array of references
	$bind_params_refs = array();
	foreach ($bind_params as $key => $value) {
		$bind_params_refs[$key] = &$bind_params[$key];
	}

    call_user_func_array([$stmt, 'bind_param'], $bind_params_refs);

	// assume something went wrong
	$result = false;
    // Fetch results as an associative array
    $data = [];
    // Execute the query
	try {
		if (!$stmt->execute()) {
			LogMsg("Error ". $stmt->error." executing the query: ". $stmt->queryString);
		}
		else
		{
			// Get the result set
			$result = $stmt->get_result();
			// if this was an insert / update, than afected rows is probably higher than 0
			if($result === false)
			{
				if( $stmt->affected_rows > 0 || (($stmt->error == "" || !isset($stmt->error) || $stmt->error === 0) && ($conn->error == "" || !isset($conn->error) || $conn->error === 0)))
				{
					$result = true;
				}
			}
		}
		// update queries do not return a result
		if($result === null)
		{
			$result = true;
		}
		
		// select query will have non boolean result
		if(is_bool($result) === false)
		{
			while ($row = $result->fetch_assoc()) {
				$data[] = $row;
			}
		}	
		
	} catch (Exception $e) {
		// Log the exception to a file or other destination
		LogMsg("Exception: " . $e->getMessage()." while executing the query ".$query);
	}

	// success or failure, we let the caller know of the result
	// !! update query result is false, but there are no errors !
	if(is_bool($result) === true)
	{
		$data[] = [ 'res' => $result, 'id' => $conn->insert_id, 'stmt_err' => $stmt->error, 'sql_err' => $conn->error];
	}
	
    // Close the statement and the database connection
    $stmt->close();
	
	return $data;
}

?>