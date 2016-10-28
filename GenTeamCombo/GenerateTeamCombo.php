<?php
set_time_limit(60 * 30); //in seconds
ini_set('memory_limit','2048M');
ob_start();

// galben = 1, rosu = 2, verde = 3, alb = 4
//load the data if we have not done it yet
if( $UserCount == 0 )
{
	$f = fopen( "echipe.txt", "rt" );
	if( $f )
	{
		//load user roles
		$UserCount = 0;
		while( $line = fgets( $f ) )
		{
//			echo $line."<br>";
			if(strpos($line,"DEV")==0 && strpos($line,"EVAL")==0 )
				break;
			$parts = explode( "\t", $line );
			$users_Role[$parts[0]] = $parts[1];
			if(strpos($line,"DEV")!=0)
				$users_Role2[$parts[0]] = 1;
			if(strpos($line,"EVAL")!=0)
				$users_Role2[$parts[0]] = 2;
			$UserCount++;
		}
		
		//load user matches
		$row = 0;
		do
		{
//			echo $line."<br>";
			$parts = explode( "\t", $line );
			$UserColumns = 0;
			foreach( $parts as $key => $val )
			{
				$UserColumns++;
				$users_UserName[$row][$key] = $parts[$key];
			}
			$row++;
		}while( $line = fgets( $f ) );
		fclose( $f );
	}
}

PrintInitialTable();

$ReqDevs[0] = 3;
$ReqEvals[0] = 2;
$ReqDevs[1] = 3;
$ReqEvals[1] = 2;
$ReqDevs[2] = 4;
$ReqEvals[2] = 3;

$MaxTeamMembers = $ReqDevs + $ReqEvals;
$MaxTeamCount = 3;
for($i=0;$i<$UserCount;$i++)
	$ProposedMemberTeam[$i]=0;

function MyEcho( $what )
{
//	echo $what;
//	return;
	
	$f = fopen( "t2.txt", "at" );
	if($f)
	{
		fputs( $f, $what );
		fclose( $f );
	}
}

//get the best combo
$SmallestRed = 9999;
$SmallestRedIndex = 9999;
$BiggestGreen = 0;
$BiggestGreenIndex = 9999;
$Cycles = 0;
$SolutionCount = 0;
while( $ProposedMemberTeam[$UserCount] == 0 )
{
	if( IsValidCombo() == 1)
	{
		$score = GetScore();
		
		$ShouldPrint = 0;
		if( $score['r']==$SmallestRed )
		{
			$what .= " Similar best solution for minimal red score <br>\n";
			$ShouldPrint = 1;
		}
		if( $score['g']==$BiggestGreen )
		{
			$what .= " Similar best solution for maximal green score <br>\n";
			$ShouldPrint = 1;
		}
		if( $score['r']<$SmallestRed )
		{
			$SmallestRed = $score['r'];
			$SmallestRedIndex = $SolutionCount;
			$what .= " New best solution for minimal red score <br>\n";
			$ShouldPrint = 1;
		}
		if( $score['g']>$BiggestGreen )
		{
			$BiggestGreen = $score['g'];
			$BiggestGreenIndex = $SolutionCount;
			$what .= " New best solution for maximal green score <br>\n";
			$ShouldPrint = 1;
		}
		
//		PrintSolution($Solutions[$SolutionCount]);
		if( $ShouldPrint==1 )
		{
			$what = "Possible Solution : ";
			for( $i=0;$i<$UserCount;$i++)
				$what .= "".($i+1)."->".($ProposedMemberTeam[$i]+1)." ";
			$what .= "$SolutionCount ) Score is : red ".$score['r']." green ".$score['g']."<br><br>\n\n";
			
			$Solutions[$SolutionCount]['score'] = $score;
			$Solutions[$SolutionCount]['team'] = $ProposedMemberTeam;
			PrintSolutionTabDelimited($Solutions[$SolutionCount]);
			MyEcho( $what );
		}
		$SolutionCount++;
//		if($Cycles>100)		
//			die();
	}
/*
//	else die();
	if($Cycles % $UserCount == 0)
	{
		if( $Cycles %100 == 0 )
		{
			print_r( $ProposedMemberTeam );
			echo "<br>";
		}
		if( $Cycles / $UserCount > 8000 )
			die();
	}/**/
	GenNextTeamCombo();
	$Cycles++;
}
echo "Finished generating combinations<br>";

echo "Best combo with most green<br>";
print_r( $Solutions[$BiggestGreenIndex]['team'] );
echo "Score is : red ".$Solutions[$BiggestGreenIndex]['score']['r']." green ".$Solutions[$BiggestGreenIndex]['score']['g']."<br>";

echo "Best combo with least red<br>";
print_r( $Solutions[$SmallestRedIndex]['team'] );
echo "Score is : red ".$Solutions[$SmallestRedIndex]['score']['r']." green ".$Solutions[$SmallestRedIndex]['score']['g']."<br>";

//Combination score is number of greens and number of reds mixing 3 devs and 3 evals
//a combination contains 3 devs and 3 evals

function GenNextTeamCombo()
{
	global $ProposedMemberTeam,$UserCount,$MaxTeamCount;
	$ProposedMemberTeam[0]++;
	for($i=0;$i<$UserCount;$i++)
		if($ProposedMemberTeam[$i] >= $MaxTeamCount )
		{
			$ProposedMemberTeam[$i] = 0;
			$ProposedMemberTeam[$i+1]++;
		}
}

function IsValidCombo()
{
	global $ProposedMemberTeam,$UserCount,$users_Role2,$ReqDevs,$ReqEvals,$MaxTeamCount;

	for($i=0;$i<$UserCount;$i++)
	{
		$UserTeam = $ProposedMemberTeam[$i];
		$u = $i + 1;
		if($users_Role2[$u]==1)
			$DevCount[$UserTeam]++;
		else if($users_Role2[$u]==2)
			$EvalCount[$UserTeam]++;
	}
	//3 devs + 3 evals for each team
	for($j=0;$j<$MaxTeamCount;$j++)
	{
		//make sure the team has enough members
		if( $DevCount[$j] < $ReqDevs[$j] )
		{
//echo "BAD: Not enough devs $j ${DevCount[$j]}";
			return 0;
		}
		if( $EvalCount[$j] < $ReqEvals[$j] )
		{
//echo "BAD: Not enough evals $j ${EvalCount[$j]}";
			return 0;
		}
	}
	return 1;
}

function GetScore()
{
	global $ProposedMemberTeam,$UserCount,$users_UserName;
	$GreenCount = 0;
	for($i=0;$i<$UserCount;$i++)
	{
		$u = $i + 1; //this is the username
		$team = $ProposedMemberTeam[$i];
		//find our desired matchlist
		for($row=0;$row<$UserCount;$row++)
			if($users_UserName[$row][0]==$u)
				break;
		if( $row < $UserCount )
			for($j=1;$j<8;$j++)
			{
				$UserMatchesWithName = $users_UserName[$row][$j];
				//check if this user is in our group
				if(	$UserMatchesWithName > 0 && $ProposedMemberTeam[$UserMatchesWithName-1] == $team )
				{
//					echo "$u -> $UserMatchesWithName ($team) is green <br>";
					$GreenCount++;
				}
			}
	}
	$GreenCount -= 2;
	$ret['r'] = $UserCount * 7 - 22 - $GreenCount;
	$ret['g'] = $GreenCount;
	return $ret;
}

function IsUserIndexCompatibleWith($i,$j)
{
	global $ProposedMemberTeam,$UserCount,$users_UserName;
	$u = $i + 1; //this is the username
	$team = $ProposedMemberTeam[$i];
	//find our desired matchlist
	for($row=0;$row<$UserCount;$row++)
		if($users_UserName[$row][0]==$u)
			break;
	if( $row < $UserCount )
		for($j=1;$j<8;$j++)
		{
			$UserMatchesWithName = $users_UserName[$row][$j];
			//check if this user is in our group
			if(	$UserMatchesWithName == $j + 1 && $ProposedMemberTeam[$UserMatchesWithName-1] == $team )
			{
//					echo "$u -> $UserMatchesWithName ($team) is green <br>";
				return 1;
			}
		}
	return 0;
}

function PrintSolutionTabDelimited($sol)
{
	global $UserCount;
	$f = fopen( "ExcelTeams.csv", "at" );
	if($f)
	{
		//build teams
		for($j=0;$j<3;$j++)
			$TeamCounter[ $j ] = 0;
		for( $i=0;$i<$UserCount;$i++)
		{
			$UserTeam = $sol['team'][$i];
			$UserTeamCounter = $TeamCounter[ $UserTeam ];
			$Team[$UserTeam][$UserTeamCounter] = $i+1;
//echo "team $UserTeam - index $UserTeamCounter = ".($Team[$UserTeam][$UserTeamCounter])."<br>";
			$TeamCounter[ $UserTeam ]++;
		}

		$table = "";
		$table .= "Red score : ".$sol['score']['r']." Green score ".$sol['score']['g']."\n";
		for($i=0;$i<7;$i++)
		{
			for($j=0;$j<3;$j++)
				$table .= $Team[$j][$i]."\t";
			$table .= "\n";
		}
		$table .= "\n";
		fputs($f,$table);
		fclose($f);
	}
}

function PrintSolution($sol)
{
	global $UserCount;
//		$Solutions[$SolutionCount]['score'] = GetScore();
//		$Solutions[$SolutionCount]['team'] = $ProposedMemberTeam;
	print_r($sol['team']);
	//build teams
	for($j=0;$j<3;$j++)
		$TeamCounter[ $j ] = 0;
	for( $i=0;$i<$UserCount;$i++)
	{
		$UserTeam = $sol['team'][$i];
		$UserTeamCounter = $TeamCounter[ $UserTeam ];
		$Team[$UserTeam][$UserTeamCounter] = $i+1;
//echo "team $UserTeam - index $UserTeamCounter = ".($Team[$UserTeam][$UserTeamCounter])."<br>";
		$TeamCounter[ $UserTeam ]++;
	}
	?>
<table border="1" >
	<tr>
		<td>Team1</td>
		<td>Team2</td>
		<td>Team3</td>
	</tr>
	<?php
		for($i=0;$i<7;$i++)
		{
			?>
	<tr>
			<?php
			for($j=0;$j<3;$j++)
			{
				$color = "";
/*				if(IsUserIndexCompatibleWith($i,$Team[$j][$i]-1) )
					$color = " bgcolor=\"#00FF00\"";
				else
					$color = " bgcolor=\"#FF0000\""; */
			?>
			<td<?=$color;?>><?=$Team[$j][$i];?></td>
			<?php
			}
			?>
	</tr>
	<?php
		}
	?>
</table>
<?php	
}

function PrintInitialTable()
{
	global $UserCount, $UserColumns, $users_Color, $users_UserName, $users_Role;
?>
Initial Table
<table border="1" >
	<?php
		for($i=0;$i<$UserCount;$i++)
		{
			?>
	<tr>
			<td><?=$users_Role[$users_UserName[$i][0]];?></td>
			<?php
			for($j=0;$j<$UserColumns;$j++)
			{
				$color = "";
/*				if($users_Color[$i][$j]==1 || $users_Color[$i][$j]=='y')
					$color = "bgcolor=\"#00FFFF\"";
				else if($users_Color[$i][$j]==2 || $users_Color[$i][$j]=='r')
					$color = "bgcolor=\"#FF0000\"";
				else if($users_Color[$i][$j]==3|| $users_Color[$i][$j]=='g')
					$color = "bgcolor=\"#00FF00\"";
				*/
				?>
				<td <?=$color;?>><?=$users_UserName[$i][$j];?></td>
				<?php
			}
			?>
	</tr>
	<?php
		}
	?>
</table>
<?php
}
$htmlStr = ob_get_contents();
ob_end_clean(); 
file_put_contents( "t.txt", $htmlStr);
//echo $htmlStr;
?>