# Microsoft Developer Studio Project File - Name="usrlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=usrlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "usrlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "usrlib.mak" CFG="usrlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "usrlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "usrlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/dev/dvb/usrlib", OAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "usrlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Obj\Release"
# PROP Intermediate_Dir "Obj\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /Gi /GX /O2 /Ob2 /I "h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX"usrlib.hpp" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\UsrlibR.lib"

!ELSEIF  "$(CFG)" == "usrlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinNt\Obj\Debug"
# PROP Intermediate_Dir "WinNt\Obj\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MDd /W2 /Gi /GX /Zi /Od /I "h" /I "e" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /YX"tools2.hpp" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\UsrlibD.lib"

!ENDIF 

# Begin Target

# Name "usrlib - Win32 Release"
# Name "usrlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\CRC32.CPP
# End Source File
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\CTRLTOOL.CPP
# End Source File
# Begin Source File

SOURCE=.\src\DeclStringTables.cpp
# End Source File
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\Dlg_tool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ENV.CPP
# End Source File
# Begin Source File

SOURCE=.\src\ENV1.CPP
# End Source File
# Begin Source File

SOURCE=.\src\Env_base.cpp
# End Source File
# Begin Source File

SOURCE=.\src\except.cpp
# End Source File
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\FILELST.CPP
# End Source File
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\FILES.CPP
# End Source File
# Begin Source File

SOURCE=.\src\GlbLogin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\loadRes.cpp
# End Source File
# Begin Source File

SOURCE=.\src\LoginClass.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MessageQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\MSGLOG.CPP
# End Source File
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\OKDIALOG.CPP
# End Source File
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\PROTECT.CPP
# End Source File
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\STRINGS.CPP
# End Source File
# Begin Source File

SOURCE=.\src\StringTable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\transport.cpp
# End Source File
# Begin Source File

SOURCE=.\src\transportClass.cpp
# End Source File
# Begin Source File

SOURCE=..\..\DVB\usrlib\src\UTILS.CPP
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\h\APP_RC.HPP
# End Source File
# Begin Source File

SOURCE=.\h\CtrlTool.hpp
# End Source File
# Begin Source File

SOURCE=.\h\Dlg_Tool.hpp
# End Source File
# Begin Source File

SOURCE=.\h\ENV.HPP
# End Source File
# Begin Source File

SOURCE=.\h\Env_base.hpp
# End Source File
# Begin Source File

SOURCE=.\h\envX.hpp
# End Source File
# Begin Source File

SOURCE=.\h\except.hpp
# End Source File
# Begin Source File

SOURCE=.\h\GENERAL.H
# End Source File
# Begin Source File

SOURCE=.\h\HEAP.H
# End Source File
# Begin Source File

SOURCE=.\h\LOADRES.HPP
# End Source File
# Begin Source File

SOURCE=.\h\MessageQueue.hpp
# End Source File
# Begin Source File

SOURCE=.\h\MSGLOG.HPP
# End Source File
# Begin Source File

SOURCE=.\h\ProtectedFile.hpp
# End Source File
# Begin Source File

SOURCE=.\h\SCREEN.H
# End Source File
# Begin Source File

SOURCE=.\h\SCRTOOLS.HPP
# End Source File
# Begin Source File

SOURCE=.\h\ScrTrace.h
# End Source File
# Begin Source File

SOURCE=.\h\SID.HPP
# End Source File
# Begin Source File

SOURCE=.\h\SLIST.H
# End Source File
# Begin Source File

SOURCE=.\h\STDERROR.H
# End Source File
# Begin Source File

SOURCE=.\h\TOOLS2.HPP
# End Source File
# Begin Source File

SOURCE=.\h\transport.hpp
# End Source File
# Begin Source File

SOURCE=.\h\transportClass.hpp
# End Source File
# Begin Source File

SOURCE=.\h\transportPrivate.hpp
# End Source File
# End Group
# Begin Group "e"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\e\ERRORS.E
# End Source File
# Begin Source File

SOURCE=.\e\SCR.E
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\Usrlib.lst
# End Source File
# End Target
# End Project
