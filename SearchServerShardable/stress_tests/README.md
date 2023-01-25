Content
===============================================================================
1. Brief description
2. Requirements
3. Instalation
4. Usage
6. Examples



1. Brief description.
-------------------------------------------------------------------------------
	Stress tester tool is designed to test the capabilities of a certain ferrari-c
instalation. It should generate reports about data insert, data fetch limits.
	Stresser tool is unable to test more than 1 server shard.
	Stresser tool is unable to test search capabilities of a server shard.

2. Requirements.
-------------------------------------------------------------------------------
	Stress tester tool is dependent on the ferrari-c project. In case ferrari-c protocol
changes are made, stress tester tool should automatically get updated

3. Instalation.
-------------------------------------------------------------------------------
	No special instalation is required. When you build ferrar-c project, it 
will also build the stress tester tool
	- checkout development branch on a new PC
	- make sure to install dependencies for ferrari-c. See doc
	- make sure to compile 3rd party libs
	- run : make
	- cd ./stress_tests

4. Usage.
-------------------------------------------------------------------------------
	You should run the stress tester tool from a different PC than the one
ferrari-c is running on. This is to not slow down the ferrari-c and get lower
than real limit statistics.
	Depending on the test you are performing, you will need to edit "stresser.conf".
	You need to set the "ServerIP" and "ServerPort" to point to the tested shard.
	"WorkerThreadCount" should not be larger than the available core count on the PC.
How to :
a) check number of connections per second ferrari-c is able to handle :
	- set RequestType=0
	- run : ./bin/stress_tool
b) check the number of parallel company inserts ( non overlapping ) :
	- set InputFile=entry_company.json
	- set RequestType=7
	- set OverWriteEntryIdThreadIncrease to a value so that every worker thread will 
			write to a different file
	- run : ./bin/stress_tool
c) check the number of overlapping company inserts :
	- set InputFile=entry_company.json
	- set RequestType=7
	- set OverWriteEntryIdThreadIncrease=0
	- run : ./bin/stress_tool
d) check the number of parallel company inserts ( non overlapping ) :
	- set InputFile=entry_profile.json
	- set RequestType=2
	- set OverWriteEntryIdThreadIncrease to a value so that every worker thread will 
			write to a different file
	- run : ./bin/stress_tool
e) check the number of overlapping company inserts :
	- set InputFile=entry_profile.json
	- set RequestType=2
	- set OverWriteEntryIdThreadIncrease=0
	- run : ./bin/stress_tool
f) check the number of parallel company requests ( non overlapping ) :
	- set RequestType=6
	- set OverWriteEntryIdThreadIncrease to a value so that every worker thread will 
			write to a different file
	- run : ./bin/stress_tool
g) check the number of overlapping company requests :
	- set RequestType=6
	- set OverWriteEntryIdThreadIncrease=0
	- run : ./bin/stress_tool
h) check the number of parallel profile requests ( non overlapping ) :
	- set RequestType=1
	- set OverWriteEntryIdThreadIncrease to a value so that every worker thread will 
			write to a different file
	- run : ./bin/stress_tool
i) check the number of overlapping profile requests :
	- set RequestType=1
	- set OverWriteEntryIdThreadIncrease=0
	- run : ./bin/stress_tool
