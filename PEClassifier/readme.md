1. Install
=============
Not required

2. Dependencies
=============
None 

3. Build
=============
	Open PEClassifier.sln with Visual Studio 2022
	Select Debug configuration and x64 platform
	Build
	
4. Running
=============
	a ) you can use test_Debugx64.bat with the conditions :
		bat file is in the main directory ( where the sln is located )
		input files ( f00... f09 are expected to be in the main directory, where the sln is located )
		project has been built in debug mode
		
	b) build your prefered configuration
		go to the build directory. Ex : x64\Release
		make sure input file paths are updated in the command
		only redirect application output if you want it in the results.txt
		execute : PEClassifier.exe f00 f01 f02 f03 f04 f05 f06 f07 f08 f09 > results.txt