@REM Chempound Client CLI Start-up File
@REM Copyright (c) Sam Adams 2011
@REM Inspired by Apache Maven Project

@REM Required ENV vars:
@REM JAVA_HOME - location of a JDK home dir

@echo off
@setLocal

set ERROR_CODE=0

@REM ==== CHECK JAVA_HOME ====
if not "%JAVA_HOME%" == "" goto OkJHome
echo.
echo ERROR: JAVA_HOME not found in your environment.
echo Please set the JAVA_HOME variable in your environment to match the
echo location of your Java installation
echo.
goto error

:OkJHome
@REM ==== CHECK java.exe ====
if exist "%JAVA_HOME%\bin\java.exe" goto OkJavaExe
echo.
echo ERROR: JAVA_HOME is set to an invalid directory.
echo JAVA_HOME = "%JAVA_HOME%"
echo Please set the JAVA_HOME variable in your environment to match the
echo location of your Java installation
echo.
goto error

:OkJavaExe
if not "%CPCLIENT_HOME%" == "" goto StripCpClientHome
@REM ==== Find Chempound Client installation ====
set "CPCLIENT_HOME=%~dp0.."
goto CheckCpClientHome

:StripCpClientHome
@REM ==== Strip trailing slash ====
if not "_%CPCLIENT_HOME:~-1%"=="_\" goto CheckCpClientHome
set "CPCLIENT_HOME=%CPCLIENT_HOME:~0,-1%"
goto StripLCpClientHome


:CheckCpClientHome
@REM ==== CHECK CPCLIENT_HOME ====
if exist "%CPCLIENT_HOME%\bin\qc.bat" goto CheckCpClientOpts
echo.
echo ERROR: CPCLIENT_HOME is set to an invalid directory.
echo CPCLIENT_HOME = "%CPCLIENT_HOME%"
echo Please set the CPCLIENT_HOME variable in your environment to match the
echo location of the Chempound Client installation
echo.
goto error


:CheckCpClientOpts
if not "%CPCLIENT_OPTS%" == "" goto init
set "CPCLIENT_OPTS=-Xmx128m"

:init
"%JAVA_HOME%/bin/java" %CPCLIENT_OPTS% -classpath "%CPCLIENT_HOME%\lib\*" -Dcpclient.home="%CPCLIENT_HOME%" -Dlog4j.configuration=qc-log4j.properties uk.ac.cam.ch.wwmm.quixote.client.QuixoteClientCLI %*

if ERRORLEVEL 1 goto error
goto exit


:error
SET ERROR_CODE=1
goto exit

:exit
@endlocal
cmd /C exit /B %ERROR_CODE%
