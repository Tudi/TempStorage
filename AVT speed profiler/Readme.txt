Sections :
	- Description
	- Usage
	- Interpret results

===============================================================================	
Description	:
Smartbear profiling tools are hardly existent. Because the script runs in a virtual machine, hooking events is even harder that doing it on a real oeprating system.
One of the few solutions is code instrumentation. This project is aimed to parse the SJ files, detect function starts and exits, add instrumentations to measure time spent in functions
After running AVT test suit, you should collection the timing information, parse it, draw conclusions where AVT slacks the most

===============================================================================	
Usage:
- copy the whole content of AVT project into the input directory
- run "Instrument.php"
- copy back the content of the output directory

===============================================================================	
Interpret results:
In "Benchmarking.sj" file there is a definition : var OutputPath = "c:\\temp\\AVTBenchmark";
After running AVT testsuite a new file should get created ( Ex : "AVTBenchmark_05_05_16_17_12_50.txt" ). You should copy this file in "Results" directory.
Open the page "InterpretResult.php" in a browser and view page source. You should see a list of functions that 
http://localhost/instrumentjs/interpretresult.php
	.......
  ["UIFinder.sj - FindMyChild"]=>
  array(4) {
    ["dur"]=>
    int(82007)
    ["count"]=>
    int(702)
    ["mindif"]=>
    int(15)
    ["timeshare"]=>
    float(18.57665159519)
  }
  ......
  ["UIFinder.sj - FindMyComponent"]=>
  array(4) {
    ["dur"]=>
    int(77621)
    ["count"]=>
    int(249)
    ["mindif"]=>
    int(110)
    ["timeshare"]=>
    float(17.583112093727)
  }  
  ...............

Yyou should look at the "timeshare" field for functions. Look for base functions with large timeshares. Making a small change in base functions should have a global impact on AVT runtime.