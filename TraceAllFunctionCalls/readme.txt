Add these files to an x32 c++ project ( for x64 you need to move the inline assembly in an asm file )
compile with /Gh flag
at some point ( after main ? ) enable logging with function : StartLogFunctionEntrances()