# Microsoft Developer Studio Project File - Name="PsiSiGen" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PsiSiGen - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PsiSiGen.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PsiSiGen.mak" CFG="PsiSiGen - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PsiSiGen - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PsiSiGen - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PsiSiGen - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "PsiSiGen - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /Zp1 /MDd /W3 /Gm /GX /Zi /Od /I "..\PsiLib\src" /I "..\usrlib\h" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 PsiLib.lib UsrLibD.lib version.lib /nologo /subsystem:windows /incremental:no /pdb:"..\bin\PsiSiGen.pdb" /debug /machine:I386 /out:"..\bin\PsiSiGen.exe" /pdbtype:sept /libpath:"..\bin" /opt:ref
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "PsiSiGen - Win32 Release"
# Name "PsiSiGen - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Sentinel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\server\DvbSentinel\Src\DVBSentinel.cpp

!IF  "$(CFG)" == "PsiSiGen - Win32 Release"

!ELSEIF  "$(CFG)" == "PsiSiGen - Win32 Debug"

# ADD CPP /I "..\UsrLib\h"
# SUBTRACT CPP /I "..\PsiLib\src" /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\server\DvbSentinel\Src\grid.cpp

!IF  "$(CFG)" == "PsiSiGen - Win32 Release"

!ELSEIF  "$(CFG)" == "PsiSiGen - Win32 Debug"

# ADD CPP /I "..\UsrLib\h"
# SUBTRACT CPP /I "..\PsiLib\src" /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\Src\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\PsiSiGen.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\StdAfx.cpp

!IF  "$(CFG)" == "PsiSiGen - Win32 Release"

!ELSEIF  "$(CFG)" == "PsiSiGen - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Src\TablesFreqDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Src\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Src\PsiSiGen.h
# End Source File
# Begin Source File

SOURCE=.\Src\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Src\TablesFreqDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\PsiLib\Src\PsiSiDlg.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\hlp\PsiSiGen.hpj

!IF  "$(CFG)" == "PsiSiGen - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__PSISI="hlp\AfxCore.rtf"	"hlp\$(TargetName).hm"	
# Begin Custom Build - Making help file...
OutDir=.\Release
TargetName=PsiSiGen
InputPath=.\hlp\PsiSiGen.hpj
InputName=PsiSiGen

"$(OutDir)\$(InputName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	start /wait hcw /C /E /M "hlp\$(InputName).hpj" 
	if errorlevel 1 goto :Error 
	if not exist "hlp\$(InputName).hlp" goto :Error 
	copy "hlp\$(InputName).hlp" $(OutDir) 
	goto :done 
	:Error 
	echo hlp\$(InputName).hpj(1) : error: 
	type "hlp\$(InputName).log" 
	:done 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "PsiSiGen - Win32 Debug"

# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\res\PsiSiGen.ico
# End Source File
# Begin Source File

SOURCE=.\PsiSiGen.rc
# End Source File
# Begin Source File

SOURCE=.\res\PsiSiGen.rc2
# End Source File
# Begin Source File

SOURCE=..\server\DvbSentinel\WinNT\RC\Sentinel.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter "cnt;rtf"
# Begin Source File

SOURCE=.\hlp\AfxCore.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AppExit.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Bullet.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw2.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw4.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurHelp.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCopy.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCut.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditPast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditUndo.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileNew.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileOpen.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FilePrnt.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileSave.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpSBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpTBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\PsiSiGen.cnt

!IF  "$(CFG)" == "PsiSiGen - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying contents file...
OutDir=.\Release
InputPath=.\hlp\PsiSiGen.cnt
InputName=PsiSiGen

"$(OutDir)\$(InputName).cnt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "hlp\$(InputName).cnt" $(OutDir)

# End Custom Build

!ELSEIF  "$(CFG)" == "PsiSiGen - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying contents file...
OutDir=.\Debug
InputPath=.\hlp\PsiSiGen.cnt
InputName=PsiSiGen

"$(OutDir)\$(InputName).cnt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "hlp\$(InputName).cnt" $(OutDir)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hlp\RecFirst.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecLast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecNext.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecPrev.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmax.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\ScMenu.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmin.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=..\server\DvbSentinel\WinNT\Lib\Spromeps.lib
# End Source File
# End Target
# End Project
