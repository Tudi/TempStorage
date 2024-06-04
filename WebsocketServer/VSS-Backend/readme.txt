VSS-Backend is a cross platform tech stack

Requirements :
	- web server. Tested with Apache
	- PHP interpreter : tested with PHP 8.1.6
	- MySQL :  Ver 8.0.29 for Win64 (MySQL Community Server - GPL)

Instalation :
	- run "sql\CreateTables.sql"
	- run "sql\DefaultData.sql"
	- copy "www\*" into your hosted web folder
	- edit "www\NotHosted\config.php" to set your DB settings
	
Usage :
	- you can try http://yourdomain/GetAPIRedirection.php to check if you copied files to proper location ( does not output anything )
