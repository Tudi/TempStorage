Description 
	Projects for generic licensing of software products
	
Usage :
	- make sure you have built "LicenseDLL". It is required by most projects
	- use project "ClientLicenseSeed" to generate a "ClientSeed.dat" file
	- use project "LicenseGenerate" to generate a "testLic.dat" file
	
	- optional : use project "UseLicenseDemo" to check how to use "testLic.dat" file
	- optional : use project "LicenseInfo" to check the content of a "testLic.dat" file
	
	
Subprojects :
		ClientLicenseSeed -> programatically generate a license seed
		ClientSeedLoadTest -> check the loading process of a license seed. For debug and demo purpuses
		GenerateClientFingerprintUI -> UI interface for the generate licese seed API calls
		GenerateLicenseUI2 -> very simple generate licesen UI interface
		GenerateLicenseUI3 -> Generate licesen interface with SQLite storage of clients, seeds
		LicenseDLL -> Core project for licensing. Should contain all API implementations for upper level projects
		LicenseGenerate -> generate a license programatically
		LicenseInfo -> example project. Dump license content using a bruteforce parsing.
		UseLicenseDemo -> Minimalistic project to demo the usage of the licensing API when consuming a license
		
Todo :
	- Generate license UI : Sort and filter client machines
	- Generate license UI : Implement versioning / logging for SQLite DB changes
	- Better logging of user actions when using license.dll
	- Popup messages (with feedback ) when using license.dll inside Siemens projects
	- More work on background process that checks for time consistency and license usage. Implement license change detection and handling
	- documentation
	- doxygen comments