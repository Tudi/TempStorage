VSS-Dashboard is a windows application project

VisualStudio 2022 project to be opened : project\VSS-UI.sln

Available build options :
	x64-Debug : Will allow usage of a console window to quickly inspect log messages
	x64-Release : Used for release builds
	
Prebuilt libraries are provided. If you wish to use a different compiler version / CRT .. you will need to rebuild these libraries
		- ImGUI
		- CURL + ZLIB

Output binary files are in : 
	\bin\Debug
	\bin\Release
	
Ini files might not be up to date in release folder. Might need to copy them from debug folder.