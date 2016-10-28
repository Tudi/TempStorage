<?php
session_start();
include( "include/DBConnection.inc.php" );
include( "include/Functions.inc.php" );

//this page can increase / decrease vote count
if( isset( $id ) && isset( $v ) )
{
	//check if we already voted
	$query1 = "select VoteResult from uservotes where sessionid=".$_SESSION['SessionId']." and ideaid=$id";
	$result1 = mysql_query($query1,$dbi) or die("201602101600".$query1);
	list( $VoteResult ) = mysql_fetch_row( $result1 );
	if( $VoteResult == 0 || $VoteResult == "" )
	{
		//check if we can vote this idea
		$query1 = "select id from ideas where id=$id and AllowVote=1 and ( RegisteredVotes = 0 or EditToken in ( select token from usertokens where sessionid =".$_SESSION['SessionId']."))";
		$result1 = mysql_query($query1,$dbi) or die("201602101626".$query1);
		list( $CanVote ) = mysql_fetch_row( $result1 );
		if( $CanVote )
		{
			$query1 = "insert into uservotes ( sessionid, ideaid, VoteResult ) values ( ".$_SESSION['SessionId'].",$id, $v )";
			$result1 = mysql_query($query1,$dbi) or die("201602101600".$query1);
			
			if( $v > 0 )
				$query1 = "update ideas set UpVoteCount=UpVoteCount+1 where id=$id";
			else
				$query1 = "update ideas set DownVoteCount=DownVoteCount+1 where id=$id";
			$result1 = mysql_query($query1,$dbi) or die("201602101620".$query1);
		}
	}
}
?>