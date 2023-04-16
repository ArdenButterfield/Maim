# Microsoft Developer Studio Project File - Name="bladeenc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=bladeenc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bladeenc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bladeenc.mak" CFG="bladeenc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bladeenc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "bladeenc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bladeenc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gr /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "bladeenc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "bladeenc - Win32 Release"
# Name "bladeenc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bladeenc\arglink.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\bladesys.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\codec.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\common.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\encode.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\formatbitstream2.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\l3bitstream.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\l3psy.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\loop.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\main.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\mdct.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\reservoir.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\samplein.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\strupr.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\subs.c
# End Source File
# Begin Source File

SOURCE=.\bladeenc\tables.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bladeenc\arglink.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\bladesys.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\codec.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\common.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\encoder.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\formatbitstream2.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\huffman.h
# End Source File
# Begin Source File

SOURCE=".\bladeenc\l3bitstream-pvt.h"
# End Source File
# Begin Source File

SOURCE=.\bladeenc\l3bitstream.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\l3psy.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\l3side.h
# End Source File
# Begin Source File

SOURCE=".\bladeenc\loop-pvt.h"
# End Source File
# Begin Source File

SOURCE=.\bladeenc\loop.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\mdct.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\reservoir.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\samplein.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\system.h
# End Source File
# Begin Source File

SOURCE=.\bladeenc\tables.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
