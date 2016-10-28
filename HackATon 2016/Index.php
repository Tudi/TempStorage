<?php
session_start();
include_once( "include/DBConnection.inc.php" );
include_once( "include/Functions.inc.php" );
?>

<script src="include/Functions.js"></script>

<table border="1">
	<tr>
		<td>
		</td>
		<td>
			<?php 
			if( !isset( $UserId ) || $UserId == 0 ) 
			{
			?>
			<a href="Login.php" target="VariableContent">Login</a>
			<?php 
			}
			else
			{
			?>
			<a href="Logout.php" target="VariableContent">Logout</a>
			<?php 
			}
			?>
			Search idea names : <input type="text" name="searchbox" id="searchbox" value="<?php echo $searchbox; ?>" onchange=FilterIdeaList() >
		</td>
	</tr>
	<tr>
		<td>
			<?php
			include( "list_idea_groups.php" );
			?>
		</td>
		<td>
			<iframe name="VariableContent" id="VariableContent" src="" width=1000 height=1000></iframe>
		</td>
	</tr>
</table>

<script>
	function FilterIdeaList()
	{
		var FilterTextbox = document.getElementById( "searchbox" );
		var ShowIdeaListFrame = document.getElementById( "VariableContent" );
		ShowIdeaListFrame.src = "list_ideas.php?SearchString="+FilterTextbox.value;
	}
</script>