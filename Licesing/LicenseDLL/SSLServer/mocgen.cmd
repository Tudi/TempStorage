@ECHO OFF

rem SET MYPATH="d:\Projects\QSSL Example\"
SET MYPATH="./"
echo Path to process : %MYPATH%

REM QT is added to environment PATH variable so we can use it from anywhere
SET "QT_PATH=D:\Projects\_3p\qt4"
SET "PATH=%QT_PATH%\bin;%PATH%"

REM Generate a list of all files
set TempFilename=filenames.txt
PUSHD %MYPATH%
dir /a-d /b /s *.h> %TempFilename%
for /F "tokens=*" %%A in (%TempFilename%) do (
	REM echo Checking:%%A
	call :process_h "%%A"
)
del %TempFilename%
POPD

exit /b

:process_h
	set OldFile=%1
	set NewName=%OldFile:~1,-3%_moc.hxx
	FOR %%i IN (%OldFile%) DO SET DATE1=%%~ti
	FOR %%i IN ("%NewName%") DO SET DATE2=%%~ti
	IF "%DATE1%"=="%DATE2%" ( echo Skipping:%OldFile%) else (
		echo Generating:%NewName%
		REM Touch the src file to update date to current time
		COPY /B %OldFile% +,, %OldFile%
		moc.exe -i %OldFile% -o "%NewName%"
	)
	exit /b
