 ::
 :: BSD 3-Clause License                                                           
 ::                                                                                
 :: Copyright (c) 2018, pa.eeapai@gmail.com                                        
 :: All rights reserved.                                                           
 ::                                                                                
 :: Redistribution and use in source and binary forms, with or without             
 :: modification, are permitted provided that the following conditions are met:    
 ::                                                                                
 :: * Redistributions of source code must retain the above copyright notice, this  
 ::   list of conditions and the following disclaimer.                             
 ::                                                                                
 :: * Redistributions in binary form must reproduce the above copyright notice,    
 ::   this list of conditions and the following disclaimer in the documentation    
 ::   and/or other materials provided with the distribution.                       
 ::                                                                                
 :: * Neither the name of the copyright holder nor the names of its                
 ::   contributors may be used to endorse or promote products derived from         
 ::   this software without specific prior written permission.                     
 ::                                                                                
 :: THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"    
 :: AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE      
 :: IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 :: DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE   
 :: FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL     
 :: DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR     
 :: SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER     
 :: CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,  
 :: OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  
 :: OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.           
 ::
@echo off
cls
set ECLIPSE_BUILD=%~nx0

set ECLIPSE_LOCATION=%~1
set EXTERNAL_TOOLS=%~2
set EXTERNAL_LIBS=%~3

set THIS_DRIVE=%~d0\

  :: ##############################################
echo [%ECLIPSE_BUILD%] Eclipse build started

set JAVA_LOC=
call :getjavaloc java.exe JAVA_LOC
if [%JAVA_LOC%]==[""] (
  echo [%ECLIPSE_INIT%] Java not found. Aborting.
  exit /b -1
)

set BACKUP_ECLIPSE_LOCATION=%~d0\eclipse
if [%ECLIPSE_LOCATION%]==[] (
  set ECLIPSE_LOCATION=%BACKUP_ECLIPSE_LOCATION%
  echo [%ECLIPSE_BUILD%] Eclipse location not provided.
)
if exist %ECLIPSE_LOCATION% (
  echo [%ECLIPSE_BUILD%] Using %ECLIPSE_LOCATION%
) else (
  echo [%ECLIPSE_BUILD%] Eclipse not found. Aborting.
  exit /b -1
)
if [%PROTOMORPH_APPS%]==[] (
  echo [%ECLIPSE_BUILD%] FYI, PROTOMORPH_APPS environment variable is not set
)

if [%EXTERNAL_TOOLS%]==[] (
  set EXTERNAL_TOOLS=%PROTOMORPH_APPS%
  echo [%ECLIPSE_BUILD%] FYI, External tools location not provided
)

if [%EXTERNAL_TOOLS%]==[] (
  set EXTERNAL_TOOLS=%THIS_DRIVE%zdev\appz\tools
)
echo [%ECLIPSE_BUILD%] Using external tools from %EXTERNAL_TOOLS%

if not exist %EXTERNAL_TOOLS% (
  echo [%ECLIPSE_BUILD%] External tools not found. Aborting.
  exit /b -1
)

set DEFAULT_ARM_XGCC_PATH=%EXTERNAL_TOOLS%\tools\gcc-arm-none-eabi-7-2017-q4-major-win32\bin
call :normalizePath %DEFAULT_ARM_XGCC_PATH% DEFAULT_ARM_XGCC_PATH
set DEFAULT_MAKE_PATH=%EXTERNAL_TOOLS%\tools\msys\1.0\bin
call :normalizePath %DEFAULT_MAKE_PATH% DEFAULT_MAKE_PATH
 
if [%PROTOMORPH_EXTERNAL_LIBS%]==[] (
  echo [%ECLIPSE_BUILD%] FYI, PROTOMORPH_EXTERNAL_LIBS environment variable is not set
)
if [%EXTERNAL_LIBS%]==[] (
  set EXTERNAL_LIBS=%PROTOMORPH_EXTERNAL_LIBS%
  echo [%ECLIPSE_BUILD%] FYI, External libs location not provided
)
if [%EXTERNAL_LIBS%]==[] (
  set EXTERNAL_LIBS=%THIS_DRIVE%zdev/libz
)
echo [%ECLIPSE_BUILD%] Using external libs from %EXTERNAL_LIBS%

if not exist %EXTERNAL_LIBS% (
  echo [%ECLIPSE_BUILD%] External libs not found. Aborting.
  exit /b -1
)
set ECLIPSE_WS=%~dp0..\EclipseWS
call :normalizePath %ECLIPSE_WS% ECLIPSE_WS

echo [%ECLIPSE_BUILD%] Eclipse workspace used for build: %ECLIPSE_WS%

  :: ##############################################
echo [%ECLIPSE_BUILD%] Importing projects
  :: Add eclipse projects below. Project can be imported multiple times ATM.
  :: If this changes the list of imported projects can be checked as list of sub folders in 
  :: .metadata\.plugins\org.eclipse.core.resources\.projects
  :: Name of the project should be probably added to parameters of :importProject call
  :: so it can be checked.
call :importProject %~dp0..\Libs\HAL\IO32STM32F103USB\Eclipse

  :: ##############################################
echo [%ECLIPSE_BUILD%] Specifying variables for linked resources to %ECLIPSE_WS% (hack)
  :: Add linked paths for linked resources (files - usually in virtual folders)
set RES_FILE=%ECLIPSE_WS%\.metadata\.plugins\org.eclipse.core.runtime\.settings\org.eclipse.core.resources.prefs
  :: File is recreated on each run (need to be careful when changing Eclipse version) 
echo [%ECLIPSE_BUILD%] %RES_FILE%
echo eclipse.preferences.version=1> %RES_FILE%
  :: echo each linked resorce location variable to RES_FILE
call :addResourceLocationVar %RES_FILE% IO32STM32F103USB_CUBEF1_LOC %EXTERNAL_LIBS%/ST/STM32Cube_FW_F1_V1.6.0
echo version=1>> %RES_FILE%

  :: ##############################################
  :: \EclipseWS\.metadata\.plugins\org.eclipse.core.runtime\.settings\org.eclipse.cdt.core.prefs
echo [%ECLIPSE_BUILD%] Adding buid variables to %ECLIPSE_WS% (hack)
set BUILD_VARS_FILE=%ECLIPSE_WS%\.metadata\.plugins\org.eclipse.core.runtime\.settings\org.eclipse.cdt.core.prefs
  :: File is recreated on each run (need to be careful when changing Eclipse version and adding new vars) 
echo [%ECLIPSE_BUILD%] %BUILD_VARS_FILE%

echo build.all.configs.enabled=false> %BUILD_VARS_FILE%
echo eclipse.preferences.version=1 >> %BUILD_VARS_FILE%

echo macros/workspace=^<?xml version\="1.0" encoding\="UTF-8" standalone\="no"?^>>tempVars.xml
echo ^<macros^>>>tempVars.xml

if not [%DEFAULT_ARM_XGCC_PATH%]==[] (
call :addBuildVar VALUE_PATH_DIR DEFAULT_ARM_XGCC_PATH %DEFAULT_ARM_XGCC_PATH%
)
if not [%DEFAULT_MAKE_PATH%]==[] (
call :addBuildVar VALUE_PATH_DIR DEFAULT_MAKE_PATH %DEFAULT_MAKE_PATH%
)

call :addBuildVar VALUE_PATH_DIR IO32STM32F103USB_CUBEF1_SDK_PATH %EXTERNAL_LIBS%/ST/STM32Cube_FW_F1_V1.6.0
call :addBuildVar VALUE_TEXT IO32STM32F103USB_XGCC_PATH "${DEFAULT_MAKE_PATH};${DEFAULT_ARM_XGCC_PATH}"

echo ^</macros^>>>tempVars.xml
call :appendBuildVarsToConf

if [%DEFAULT_MAKE_PATH%]==[] (
echo [%ECLIPSE_BUILD%] Add build variable DEFAULT_MAKE_PATH to point to make
)

if [%DEFAULT_ARM_XGCC_PATH%]==[] (
echo [%ECLIPSE_BUILD%] Add build variable DEFAULT_ARM_XGCC_PATH to point to GCC ARM compiler
)

  :: ##############################################
echo [%ECLIPSE_BUILD%] Building projects
 :: Add -build <PROJECT>/<CONFIGURATION> line for each project
 :: Add -E <VAR NAME>=<VALUE> for all new variables needed by projects [obsolete]

  :: Modify PATH temorarly to workaround indexer problem 
setlocal
set PATH=%PATH%;%DEFAULT_ARM_XGCC_PATH%
 ::
 :: call :back2slash %DEFAULT_MAKE_PATH% DEFAULT_MAKE_PATH
 :: call :back2slash %DEFAULT_ARM_XGCC_PATH% DEFAULT_ARM_XGCC_PATH
 :: call :back2slash %EXTERNAL_LIBS% EXTERNAL_LIBS
 ::
 :: -E DEFAULT_MAKE_PATH=%DEFAULT_MAKE_PATH% ^
 :: -E DEFAULT_ARM_XGCC_PATH=%DEFAULT_ARM_XGCC_PATH% ^
 :: -E IO32STM32F103USB_XGCC_PATH=${DEFAULT_MAKE_PATH};${DEFAULT_ARM_XGCC_PATH} ^
 :: -E IO32STM32F103USB_CUBEF1_SDK_PATH=%EXTERNAL_LIBS%/ST/STM32Cube_FW_F1_V1.6.0 ^


%ECLIPSE_LOCATION%\eclipsec.exe ^
-nosplash ^
-application org.eclipse.cdt.managedbuilder.core.headlessbuild ^
-data %ECLIPSE_WS% ^
-build IO32STM32F103USB/DebugBluePill ^
-printErrorMarkers ^
--launcher.ini %ECLIPSE_LOCATION%eclipse.ini ^
--launcher.suppressErrors

endlocal

GOTO:eof

  :: ##############################################
:importProject
  set _PROJECT_TO_IMPORT=%~f1
  echo [%ECLIPSE_BUILD%]   %_PROJECT_TO_IMPORT%
  %ECLIPSE_LOCATION%\eclipsec.exe ^
  -nosplash ^
  -application org.eclipse.cdt.managedbuilder.core.headlessbuild ^
  -data %ECLIPSE_WS% ^
  -import %_PROJECT_TO_IMPORT% ^
  -printErrorMarkers ^
  --launcher.ini %ECLIPSE_LOCATION%eclipse.ini ^
  --launcher.suppressErrors
  )
  goto:EOF

:addResourceLocationVar
  set _RES_FILE=%~1
  set _VAR_NAME=%~2
  set _LOCATION=%~f3
  set _LOCATION_SLASHES=%_LOCATION:\=/%
  set _LOCATION_FINAL=%_LOCATION_SLASHES:~0,1%\:%_LOCATION_SLASHES:~2%
  echo [%ECLIPSE_BUILD%]   %_VAR_NAME%: %_LOCATION%
  echo pathvariable.%_VAR_NAME%=%_LOCATION_FINAL%>> %_RES_FILE%
  goto:EOF

:addBuildVar
set _TYPE=%1
set _NAME=%2
set _VALUE=%3
if [%_TYPE%]==[VALUE_PATH_DIR] (
  setlocal EnableDelayedExpansion
  call :back2slash !_VALUE! _VALUE
  echo [%ECLIPSE_BUILD%]   %_NAME%: !_VALUE!
  call :escapeDriveSemicolon !_VALUE! _VALUE
  echo ^<stringMacro name\="%_NAME%" type\="%_TYPE%" value\="!_VALUE!"/^>>> tempVars.xml
  endlocal
) else (
  :: Always with double qoutes
echo [%ECLIPSE_BUILD%]   %_NAME%: %_VALUE:~1,-1%
echo ^<stringMacro name\="%_NAME%" type\="%_TYPE%" value\="%_VALUE:~1,-1%"/^>>> tempVars.xml
)
GOTO:eof

:appendBuildVarsToConf
for /f "tokens=*" %%a in ('find /n /v "" ^< "tempVars.xml"') do ( 
    set line=%%a
    SetLocal EnableDelayedExpansion
    set line=!line:*]=!\n\r
    set /p =!line!<nul
    EndLocal
)>>%BUILD_VARS_FILE%
GOTO:eof

:normalizePath
  set %2=%~f1
  goto:EOF

:back2slash
  set _WITH_BACKSLASHES=%1
  set _WITH_SLASHES=%_WITH_BACKSLASHES:\=/%
  set %2=%_WITH_SLASHES%  
  goto:EOF

:escapeDriveSemicolon
  set _UNESCAPED=%1
  set %2=%_UNESCAPED:~0,1%\:%_UNESCAPED:~2%
  goto:EOF

:getjavaloc
  set %2="%~$PATH:1"
  goto:EOF

:printUsage
  echo EclipseBuild.bat <ECLIPSE PATH> <TOOLS PATH> <LIBS PATH>
  echo ECLIPSE PATH - Path to folder with Eclipse executables
  echo   TOOLS PATH - Path to folder with GCC compiler(s) and make  
  echo    LIBS PATH - Path to folder with external libraries
  echo Example:
  echo EclipseBuild.bat E:\Eclipse X:\Programs\CompileTools D:\ExternalLibs\ProtomorphLibs
  goto:EOF