# Microsoft Developer Studio Generated NMAKE File, Based on OQ_BackEnd.dsp
!IF "$(CFG)" == ""
CFG=OQ_BackEnd - Win32 Debug
!MESSAGE No configuration specified. Defaulting to OQ_BackEnd - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "OQ_BackEnd - Win32 Release" && "$(CFG)" != "OQ_BackEnd - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OQ_BackEnd.mak" CFG="OQ_BackEnd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OQ_BackEnd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "OQ_BackEnd - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "OQ_BackEnd - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\ReleaseBin\OQ_BackEnd.exe"


CLEAN :
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\OQ_BackEnd.obj"
	-@erase "$(INTDIR)\OQ_BackEnd.pch"
	-@erase "$(INTDIR)\OQ_BackEnd.res"
	-@erase "..\ReleaseBin\OQ_BackEnd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\resus" /I "..\mil0" /I "..\mil1" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_AFXDLL" /Fp"$(INTDIR)\OQ_BackEnd.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\OQ_BackEnd.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\OQ_BackEnd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\mil0\release\mil0.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\mil0\release\mil0.lib ..\mil1\release\mil1.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\OQ_BackEnd.pdb" /machine:I386 /out:"..\ReleaseBin\OQ_BackEnd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\OQ_BackEnd.obj" \
	"$(INTDIR)\OQ_BackEnd.res"

"..\ReleaseBin\OQ_BackEnd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "OQ_BackEnd - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\DebugBin\OQ_BackEnd.exe"


CLEAN :
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\OQ_BackEnd.obj"
	-@erase "$(INTDIR)\OQ_BackEnd.pch"
	-@erase "$(INTDIR)\OQ_BackEnd.res"
	-@erase "$(OUTDIR)\OQ_BackEnd.pdb"
	-@erase "..\DebugBin\OQ_BackEnd.exe"
	-@erase "..\DebugBin\OQ_BackEnd.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\resus" /I "..\mil0" /I "..\mil1" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_AFXDLL" /Fp"$(INTDIR)\OQ_BackEnd.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\OQ_BackEnd.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\OQ_BackEnd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\mil0\debug\mil0.lib ..\mil1\debug\mil1.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\OQ_BackEnd.pdb" /debug /machine:I386 /out:"..\DebugBin\OQ_BackEnd.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\OQ_BackEnd.obj" \
	"$(INTDIR)\OQ_BackEnd.res"

"..\DebugBin\OQ_BackEnd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("OQ_BackEnd.dep")
!INCLUDE "OQ_BackEnd.dep"
!ELSE 
!MESSAGE Warning: cannot find "OQ_BackEnd.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "OQ_BackEnd - Win32 Release" || "$(CFG)" == "OQ_BackEnd - Win32 Debug"
SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "OQ_BackEnd - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "..\resus" /I "..\mil0" /I "..\mil1" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_AFXDLL" /Fp"$(INTDIR)\OQ_BackEnd.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\OQ_BackEnd.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "OQ_BackEnd - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\resus" /I "..\mil0" /I "..\mil1" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_AFXDLL" /Fp"$(INTDIR)\OQ_BackEnd.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\OQ_BackEnd.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\OQ_BackEnd.cpp

"$(INTDIR)\OQ_BackEnd.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\OQ_BackEnd.pch"


SOURCE=.\OQ_BackEnd.rc

"$(INTDIR)\OQ_BackEnd.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

