<?php
session_start();
include_once( "include/DBConnection.inc.php" );
include_once( "include/Functions.inc.php" );

$query = "select Id,Name from IdeaGroups";
$result = mysql_query($query,$dbi) or die("201602151000".$query);
while( list( $Id,$Name ) = mysql_fetch_row( $result ) )
{
?>
	<input type="button" id="GroupSelect_<?php echo $Id; ?>" value="<?php echo $Name; ?>" onclick=SelectGroup(<?php echo $Id; ?>) /><br>
<?php 
}
?>

<script>
	function SelectGroup(id)
	{
		var ShowIdeaListFrame = document.getElementById( "VariableContent" );
		ShowIdeaListFrame.src = "list_ideas.php?GroupId="+id;
	}
</script>
