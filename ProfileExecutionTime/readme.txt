Sections :
	- Description
	- How to use
	- How to use on large scale
	- Examples
	- How to interpret result

===============================================================================	
Description :
There are lots of professional profiling tools. Ex : Intel VTune, AMD catalyst, Visual Studio.... The gneric issue is that these tools either provide too much info, or they slow down the execution too much.
In many cases you will be forced to measure the time yourself. 

===============================================================================	
How to use :
// Inside a cpp file
#include "Debug.cpp"
// in some cases you want to add stacktrace while measuring a function. 
#include "StackTrace.h"
// replace function name with the stacktrace. This is important when you want to measure a specific module and ignore non important calls. Ex: Loading huge SQL queries for tables compared to loading 1 line queries comming from login
	char CallTrace[2000];
	printStack( CallTrace, sizeof( CallTrace ) );
    ProfileLine( __FILE__, CallTrace, __LINE__, "Start", 1 );

	
===============================================================================	
How to use on large scale :
instrument_Environment_Class.php is an example how you could automatically instrument a specific class in empower to generate timing info

You could generalize and instrument all classes for a generic time consume estimation


===============================================================================	
Examples :
In generic case you want to measure the time spent in a code segment.
Put a line at the beginning like this ( the 1 at the end signals you want to start a new measurement ):
    ProfileLine( __FILE__, __FUNCTION__, __LINE__, "Start", 1 );
Put a line at the end of the segment like this ( the 2 at the end signals you want to close the measurement ):
    ProfileLine( __FILE__, __FUNCTION__, __LINE__, "End", 2 );
	
	
The strings "Start" / "End" are just examples. You can add any string that helps you in the process of measuring. Example :
    ProfileLine( __FILE__, __FUNCTION__, __LINE__, "Start profiling", 1 );
    ProfileLine( __FILE__, __FUNCTION__, __LINE__, "how much time did we spend since start ?", 0 );
    ProfileLine( __FILE__, __FUNCTION__, __LINE__, "How much time did we spend since start ? Close sessions.", 2 );


===============================================================================	
How to interpret result:	
perf_report_12828.txt is an example of output you would get after profiling. You should focus on the functions with highest timeshare! The column "time ms" should not be taken as a very precise measurement.	