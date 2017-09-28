@rem $Novell: bldw32.bat,v 1.5 2016/01/07 20:06:05 $
@echo off

rem   A batch file for building LDAP samples or test programs on Win32 platforms
rem   using the Microsoft Visual C++ compiler.

rem   This relies on the Microsoft script VCVARS32.BAT to set up the PATH and
rem   environment variables for command line tools.


rem   You may want to customize settings marked with "***".

if "%1"=="" goto Usage

rem *** Base path for NetIQ NDK.    Set this for your environment.
set _NDK_BASE=C:\openldapsdk

rem  *** If not already done, set up compiler command line tools.
rem  *** Edit this path for your system.
if "%MSVCDir%" == "" call "C:\PROGRA~1\MICROS~1\VC98\Bin\vcvars32.bat"

rem *** A debug version of your program will be built if the environment
rem     variable DEBUG is defined.   i.e. SET DEBUG=Y

rem   Set path for LDAP libraries.
set _LDAP_LIB_PATH=%_NDK_BASE%\lib

rem   Set which LDAP libraries to use.
rem   All LDAP programs require LDAPSDK.  LDAPSSL and LDAPX are optional.
rem   This sample uses all three.
set _LDAP_LIBS=%_LDAP_LIB_PATH%\LIBLDAP_R.LIB

rem *** LDAPSSL is needed only if your program uses SSL encryption (ldapssl_xxx calls).
set _LDAP_LIBS=%_LDAP_LIBS% %_LDAP_LIB_PATH%\LDAP_SSL.LIB

rem *** LDAPX is needed only if your program uses NetIQ LDAP extensions
set _LDAP_LIBS=%_LDAP_LIBS% %_LDAP_LIB_PATH%\NLDAPEXTD.LIB


rem  Compiler flags
rem   /c  - Compile to an object file
rem   /W3 - Display warning messages
rem   /MT - Create a multi-threaded executable using LIBCMT.LIB
rem   /nologo - Suppress sign-on banner
rem   /TP - Compile as .cpp file
rem   /DWIN32 - Define symbol WIN32
rem   /Od - (DEBUG) Suppress optimization
rem   /Z7 - (DEBUG) Produce C7.0-style debug information
set _COMP_FLAGS=/c /W3 /MT /nologo /TP /DWIN32 /D_WINDOWS /DLDAP_DEPRECATED
if NOT "%DEBUG%" == ""  set _COMP_FLAGS=%_COMP_FLAGS% /Z7 /Od
set _INC_PATHS=%_NDK_BASE%\inc


rem  Linker flags
rem   /nologo - Suppress sign-on banner
rem   /DEBUG - For building a debug version
rem   /DEBUGTYPE:BOTH  /PDB:NONE - Build debugging information into the EXE file
set _LINK_FLAGS=/nologo
if NOT "%DEBUG%" == ""  set _LINK_FLAGS=%_LINK_FLAGS% /DEBUG /DEBUGTYPE:BOTH /PDB:NONE
set _MSLIBS=LIBCMT.LIB KERNEL32.LIB


echo.
echo   Building LDAP program ...
echo COMPILING.........
cl %_COMP_FLAGS% -I%_INC_PATHS% %1.c
if ERRORLEVEL 1 goto done


echo LINKING.........
link  %_LINK_FLAGS% %1.obj %_LDAP_LIBS% %_MSLIBS%
if ERRORLEVEL 1 goto done

echo   Build complete.

:done
del %1.obj 2>nul
set _NDK_BASE=
set _INC_PATHS=
set _LDAP_LIB_PATH=
set _LDAP_LIBS=
set _COMP_FLAGS=
set _LINK_FLAGS=
set _MSLIBS=
exit /B %ERRORLEVEL%

:Usage
echo.
echo Usage:  bldw32 filename  (with no extension.  Assumes .c)
echo    Builds a program with NetIQ's LDAP Libraries for C for Win32.
echo.
echo    This file should be edited to set paths appropriate to your environment.
exit /B 1

