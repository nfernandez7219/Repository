# Microsoft Developer Studio Project File - Name="dvblib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=dvblib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dvblib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dvblib.mak" CFG="dvblib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dvblib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "dvblib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/dev/dvb/dvblib", IAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dvblib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "winnt\obj\Release"
# PROP Intermediate_Dir "winnt\obj\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /Gi /GX /O2 /Ob2 /I "h" /I "..\usrlib\h" /I "..\dvbStandard\h" /I "..\drivers\cardNT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D _WIN32_WINNT=0x0400 /YX"mux.hpp" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\dvbR.lib"

!ELSEIF  "$(CFG)" == "dvblib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinNT\Obj\Debug"
# PROP Intermediate_Dir "winnt\obj\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MDd /W2 /Gi /GX /Zi /Od /Ob1 /Gy /I "h" /I "..\usrlib\h" /D "WIN32" /D "_MBCS" /D "_DEBUG" /D "_WINDOWS" /D _WIN32_WINNT=0x0400 /D "_AFXDLL" /YX"mux.hpp" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "WinNT\RC" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin\dvbD.lib"

!ENDIF 

# Begin Target

# Name "dvblib - Win32 Release"
# Name "dvblib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "Setup"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\BaseCfg.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\BaseRegistry.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\ClientCfg.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\ServCfg.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\SRC\Asserts.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\BaseIo.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\ComIO.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CommandReceiver.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\DvbError.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\DvbUser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fec.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\FileIo.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\INBOX.CPP
# End Source File
# Begin Source File

SOURCE=.\SRC\internet.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\Logger.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\MfxGlobals.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\mux.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\MuxPacket.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\Schedulers.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\SerialNum.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\Services.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\splash.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Group "Setup Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\H\BaseCfg.hpp
# End Source File
# Begin Source File

SOURCE=.\h\BaseRegistry.hpp
# End Source File
# Begin Source File

SOURCE=.\H\ClientCfg.hpp
# End Source File
# Begin Source File

SOURCE=.\H\ServCfg.hpp
# End Source File
# Begin Source File

SOURCE=.\H\Setup.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\H\ComIO.hpp
# End Source File
# Begin Source File

SOURCE=.\H\DrvInterface.hpp
# End Source File
# Begin Source File

SOURCE=.\H\DvbError.hpp
# End Source File
# Begin Source File

SOURCE=.\H\DvbUser.h
# End Source File
# Begin Source File

SOURCE=.\H\DvbUser.hpp
# End Source File
# Begin Source File

SOURCE=.\H\FileIo.hpp
# End Source File
# Begin Source File

SOURCE=.\H\INBOX.HPP
# End Source File
# Begin Source File

SOURCE=.\H\Internet.hpp
# End Source File
# Begin Source File

SOURCE=.\H\Logger.hpp
# End Source File
# Begin Source File

SOURCE=.\H\MfxGlobals.hpp
# End Source File
# Begin Source File

SOURCE=.\H\Mux.hpp
# End Source File
# Begin Source File

SOURCE=.\H\MuxPacket.hpp
# End Source File
# Begin Source File

SOURCE=.\h\serialNum.hpp
# End Source File
# Begin Source File

SOURCE=.\H\Service.hpp
# End Source File
# Begin Source File

SOURCE=.\H\splash.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\SRC\dvblib.lst
# End Source File
# End Target
# End Project
