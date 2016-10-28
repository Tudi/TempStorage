set TargetDir="d:\temp\sjfiles"
set SourceDir="d:\E3 - AVT - GIT\"

echo %TargetDir%
rmdir /S /Q %TargetDir%
mkdir %TargetDir%
xcopy %SourceDir%*.sj %TargetDir% /sy
