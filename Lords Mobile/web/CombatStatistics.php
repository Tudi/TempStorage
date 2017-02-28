For now this is a static table. When i have enough data i will turn it into a combat simulator<br>
Below data contains ZERO combat stat changers. Infantry phalanx setup.<br>
<?php
$cnt = 0;

$struct["A"]="Infantry";
$struct["AIC"]=100;
$struct["AAC"]=0;
$struct["ACC"]=0;
$struct["ATC"]=0;
$struct["D"]="Infantry";
$struct["DIC"]=100;
$struct["DAC"]=0;
$struct["DCC"]=0;
$struct["DTC"]=0;

$struct["AICD"]=20;
$struct["AACD"]=0;
$struct["ACCD"]=0;
$struct["ATCD"]=0;
$struct["DICD"]=20;
$struct["DACD"]=0;
$struct["DCCD"]=0;
$struct["DTCD"]=0;
$StatisticsList[$cnt++] = $struct;

$struct["A"]="Infantry";
$struct["AIC"]=100;
$struct["AAC"]=0;
$struct["ACC"]=0;
$struct["ATC"]=0;
$struct["D"]="Cavalry";
$struct["DIC"]=0;
$struct["DAC"]=0;
$struct["DCC"]=100;
$struct["DTC"]=0;

$struct["AICD"]=54;
$struct["AACD"]=0;
$struct["ACCD"]=0;
$struct["ATCD"]=0;
$struct["DICD"]=0;
$struct["DACD"]=0;
$struct["DCCD"]=8;
$struct["DTCD"]=0;
$StatisticsList[$cnt++] = $struct;

$struct["A"]="Infantry";
$struct["AIC"]=100;
$struct["AAC"]=0;
$struct["ACC"]=0;
$struct["ATC"]=0;
$struct["D"]="Archer";
$struct["DIC"]=0;
$struct["DAC"]=100;
$struct["DCC"]=0;
$struct["DTC"]=0;

$struct["AICD"]=4;
$struct["AACD"]=0;
$struct["ACCD"]=0;
$struct["ATCD"]=0;
$struct["DICD"]=0;
$struct["DACD"]=64;
$struct["DCCD"]=0;
$struct["DTCD"]=0;
$StatisticsList[$cnt++] = $struct;

$struct["A"]="Infantry";
$struct["AIC"]=100;
$struct["AAC"]=0;
$struct["ACC"]=0;
$struct["ATC"]=0;
$struct["D"]="Trebuchet";
$struct["DIC"]=0;
$struct["DAC"]=0;
$struct["DCC"]=0;
$struct["DTC"]=100;

$struct["AICD"]=0;
$struct["AACD"]=0;
$struct["ACCD"]=0;
$struct["ATCD"]=0;
$struct["DICD"]=0;
$struct["DACD"]=0;
$struct["DCCD"]=0;
$struct["DTCD"]=56;
$StatisticsList[$cnt++] = $struct;

$struct["A"]="Archer";
$struct["AIC"]=0;
$struct["AAC"]=100;
$struct["ACC"]=0;
$struct["ATC"]=0;
$struct["D"]="Cavalry";
$struct["DIC"]=0;
$struct["DAC"]=0;
$struct["DCC"]=100;
$struct["DTC"]=0;

$struct["AICD"]=0;
$struct["AACD"]=8;
$struct["ACCD"]=0;
$struct["ATCD"]=0;
$struct["DICD"]=0;
$struct["DACD"]=0;
$struct["DCCD"]=54;
$struct["DTCD"]=0;
$StatisticsList[$cnt++] = $struct;

$struct["A"]="Archer";
$struct["AIC"]=0;
$struct["AAC"]=100;
$struct["ACC"]=0;
$struct["ATC"]=0;
$struct["D"]="Archer";
$struct["DIC"]=0;
$struct["DAC"]=100;
$struct["DCC"]=0;
$struct["DTC"]=0;

$struct["AICD"]=0;
$struct["AACD"]=20;
$struct["ACCD"]=0;
$struct["ATCD"]=0;
$struct["DICD"]=0;
$struct["DACD"]=20;
$struct["DCCD"]=0;
$struct["DTCD"]=0;
$StatisticsList[$cnt++] = $struct;

$struct["A"]="Archer";
$struct["AIC"]=0;
$struct["AAC"]=100;
$struct["ACC"]=0;
$struct["ATC"]=0;
$struct["D"]="Trebuchet";
$struct["DIC"]=0;
$struct["DAC"]=0;
$struct["DCC"]=0;
$struct["DTC"]=100;

$struct["AICD"]=0;
$struct["AACD"]=8;
$struct["ACCD"]=0;
$struct["ATCD"]=0;
$struct["DICD"]=0;
$struct["DACD"]=0;
$struct["DCCD"]=0;
$struct["DTCD"]=44;
$StatisticsList[$cnt++] = $struct;

$struct["A"]="Cavalry";
$struct["AIC"]=0;
$struct["AAC"]=0;
$struct["ACC"]=100;
$struct["ATC"]=0;
$struct["D"]="Trebuchet";
$struct["DIC"]=0;
$struct["DAC"]=0;
$struct["DCC"]=0;
$struct["DTC"]=100;

$struct["AICD"]=0;
$struct["AACD"]=0;
$struct["ACCD"]=2;
$struct["ATCD"]=0;
$struct["DICD"]=0;
$struct["DACD"]=0;
$struct["DCCD"]=0;
$struct["DTCD"]=58;
$StatisticsList[$cnt++] = $struct;

$struct["A"]="Infantry + Cavalry + Archer";
$struct["AIC"]=100;
$struct["AAC"]=100;
$struct["ACC"]=100;
$struct["ATC"]=0;
$struct["D"]="Infantry + Cavalry + Archer";
$struct["DIC"]=100;
$struct["DAC"]=100;
$struct["DCC"]=100;
$struct["DTC"]=0;

$struct["AICD"]=66;
$struct["AACD"]=0;
$struct["ACCD"]=14;
$struct["ATCD"]=0;
$struct["DICD"]=66;
$struct["DACD"]=0;
$struct["DCCD"]=14;
$struct["DTCD"]=0;
$StatisticsList[$cnt++] = $struct;

$struct["A"]="Infantry ";
$struct["AIC"]=100;
$struct["AAC"]=0;
$struct["ACC"]=0;
$struct["ATC"]=0;
$struct["D"]="Infantry + Archer";
$struct["DIC"]=50;
$struct["DAC"]=50;
$struct["DCC"]=0;
$struct["DTC"]=0;

$struct["AICD"]=9;
$struct["AACD"]=0;
$struct["ACCD"]=0;
$struct["ATCD"]=0;
$struct["DICD"]=27;
$struct["DACD"]=0;
$struct["DCCD"]=0;
$struct["DTCD"]=0;
$StatisticsList[$cnt++] = $struct;

$struct["A"]="Infantry ";
$struct["AIC"]=100;
$struct["AAC"]=0;
$struct["ACC"]=0;
$struct["ATC"]=0;
$struct["D"]="Cavalry + Archer";
$struct["DIC"]=0;
$struct["DAC"]=50;
$struct["DCC"]=50;
$struct["DTC"]=0;

$struct["AICD"]=24;
$struct["AACD"]=0;
$struct["ACCD"]=0;
$struct["ATCD"]=0;
$struct["DICD"]=0;
$struct["DACD"]=0;
$struct["DCCD"]=12;
$struct["DTCD"]=0;
$StatisticsList[$cnt++] = $struct;
?>						 
<table border=1>
	<tr>
		<td>Attacker</td>
		<td>Defender</td>
		<td>Atackers dead</td>
		<td>Defenders dead</td>
		<td>Attackers survived</td>
		<td>Defenders survived</td>
	</tr>
	<?php
	for($i=0;$i<$cnt;$i++)
	{
		$Attacker = "";
		$AttackerA = "";
		$AttackerD = "";
		if( $StatisticsList[$i]["AIC"] )
		{
			$Attacker .= $StatisticsList[$i]["AIC"]." Infantry ";
			if( $StatisticsList[$i]["AICD"] > 0 )
				$AttackerD .= $StatisticsList[$i]["AICD"]." Infantry ";
			$AttackerA .= ($StatisticsList[$i]["AIC"]-$StatisticsList[$i]["AICD"])." Infantry ";
		}
		if( $StatisticsList[$i]["AAC"] )
		{
			$Attacker .= $StatisticsList[$i]["AAC"]." Archer";
			if($StatisticsList[$i]["AACD"]>0)
				$AttackerD .= $StatisticsList[$i]["AACD"]." Archer";
			$AttackerA .= ($StatisticsList[$i]["AAC"]-$StatisticsList[$i]["AACD"])." Archer ";
		}
		if( $StatisticsList[$i]["ACC"] )
		{
			$Attacker .= $StatisticsList[$i]["ACC"]." Cavalry";
			if($StatisticsList[$i]["ACCD"]>0)
				$AttackerD .= $StatisticsList[$i]["ACCD"]." Cavalry";
			$AttackerA .= ($StatisticsList[$i]["ACC"]-$StatisticsList[$i]["ACCD"])." Cavalry ";
		}
		if( $StatisticsList[$i]["ATC"] )
		{
			$Attacker .= $StatisticsList[$i]["ATC"]." Trebuchet";
			if($StatisticsList[$i]["ATCD"]>0)
				$AttackerD .= $StatisticsList[$i]["ATCD"]." Trebuchet";
			$AttackerA .= ($StatisticsList[$i]["ATC"]-$StatisticsList[$i]["ATCD"])." Trebuchet ";
		}
	
		$Defender = "";
		$DefenderA = "";
		$DefenderD = "";
		if( $StatisticsList[$i]["DIC"] )
		{
			$Defender .= $StatisticsList[$i]["DIC"]." Infantry ";
			if($StatisticsList[$i]["DICD"]>0)
				$DefenderD .= $StatisticsList[$i]["DICD"]." Infantry ";
			$DefenderA .= ($StatisticsList[$i]["DIC"]-$StatisticsList[$i]["DICD"])." Infantry ";
		}
		if( $StatisticsList[$i]["DAC"] )
		{
			$Defender .= $StatisticsList[$i]["DAC"]." Archer ";
			if($StatisticsList[$i]["DACD"]>0)
				$DefenderD .= $StatisticsList[$i]["DACD"]." Archer ";
			$DefenderA .= ($StatisticsList[$i]["DAC"]-$StatisticsList[$i]["DACD"])." Archer ";
		}
		if( $StatisticsList[$i]["DCC"] )
		{
			$Defender .= $StatisticsList[$i]["DCC"]." Cavalry ";
			if($StatisticsList[$i]["DCCD"]>0)
				$DefenderD .= $StatisticsList[$i]["DCCD"]." Cavalry ";
			$DefenderA .= ($StatisticsList[$i]["DCC"]-$StatisticsList[$i]["DCCD"])." Cavalry ";
		}
		if( $StatisticsList[$i]["DTC"] )
		{
			$Defender .= $StatisticsList[$i]["DTC"]." Trebuchet ";
			if($StatisticsList[$i]["DTCD"]>0)
				$DefenderD .= $StatisticsList[$i]["DTCD"]." Trebuchet ";
			$DefenderA .= ($StatisticsList[$i]["DTC"]-$StatisticsList[$i]["DTCD"])." Trebuchet ";
		}
	?>
	<tr>
		<td><?php echo $Attacker; ?></td>
		<td><?php echo $Defender; ?></td>
		<td><?php echo $AttackerD;?></td>
		<td><?php echo $DefenderD;?></td>
		<td><?php echo $AttackerA;?></td>
		<td><?php echo $DefenderA;?></td>
	</tr>
	<?php
	}
	?>
</table>
