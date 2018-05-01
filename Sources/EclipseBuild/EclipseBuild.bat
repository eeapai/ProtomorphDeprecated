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

echo [%ECLIPSE_BUILD%] Eclipse build started

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
  echo [%ECLIPSE_BUILD%] External tools location not provided
)

if exist %EXTERNAL_TOOLS% (
  echo [%ECLIPSE_BUILD%] Using external tools from %EXTERNAL_TOOLS%
) else (
  echo [%ECLIPSE_BUILD%] External tools not found. Aborting.
  exit /b -1
)

set DEFAULT_ARM_XGCC=%EXTERNAL_TOOLS%\tools\gcc-arm-none-eabi-7-2017-q4-major-win32\bin
call :normalizePath %DEFAULT_ARM_XGCC% DEFAULT_ARM_XGCC
set DEFAULT_MAKE_PATH=%EXTERNAL_TOOLS%\tools\msys\1.0\bin
call :normalizePath %DEFAULT_MAKE_PATH% DEFAULT_MAKE_PATH
 
if [%PROTOMORPH_EXTERNAL_LIBS%]==[] (
  echo [%ECLIPSE_BUILD%] FYI, PROTOMORPH_EXTERNAL_LIBS environment variable is not set
)
if [%EXTERNAL_LIBS%]==[] (
  set EXTERNAL_LIBS=%PROTOMORPH_EXTERNAL_LIBS%
  echo [%ECLIPSE_BUILD%] External libs location not provided
)
if exist %EXTERNAL_LIBS% (
  echo [%ECLIPSE_BUILD%] Using external libs from %EXTERNAL_LIBS%
) else (
  echo [%ECLIPSE_BUILD%] External libs not found. Aborting.
  exit /b -1
)
set ECLIPSE_WS=%~dp0..\EclipseWS
call :normalizePath %ECLIPSE_WS% ECLIPSE_WS

echo [%ECLIPSE_BUILD%] Eclipse workspace used for build: %ECLIPSE_WS%

echo [%ECLIPSE_BUILD%] Importing projects
  :: Add eclipse projects below. Project can be imported multiple times ATM.
  :: If this changes the list of imported projects can be checked as list of sub folders in 
  :: .metadata\.plugins\org.eclipse.core.resources\.projects
  :: Name of the project should be probably added to parameters of :importProject call
  :: so it can be checked.
call :importProject %~dp0..\Libs\HAL\IO32STM32F103USB\Eclipse

echo [%ECLIPSE_BUILD%] Specifying variables for linked resources to %ECLIPSE_WS% (hack)
set RES_FILE=%ECLIPSE_WS%\.metadata\.plugins\org.eclipse.core.runtime\.settings\org.eclipse.core.resources.prefs
echo [%ECLIPSE_BUILD%] %RES_FILE%
echo eclipse.preferences.version=1 > %RES_FILE%
  :: echo each linked resorce location variable to RES_FILE
call :addResourceLocationVar %RES_FILE% IO32STM32F103USB_CUBEF1_LOC %EXTERNAL_LIBS%\ST\STM32Cube_FW_F1_V1.6.0
echo version=1 >> %RES_FILE%

 :: For build variables
 :: \EclipseWS\.metadata\.plugins\org.eclipse.core.runtime\.settings\org.eclipse.cdt.core.prefs

echo [%ECLIPSE_BUILD%] Building projects
 :: Add -build <PROJECT>/<CONFIGURATION> line for each project
 :: Add -E <VAR NAME>=<VALUE> for all new variables needed by projects
setlocal
set PATH=%PATH%;%DEFAULT_ARM_XGCC%

%ECLIPSE_LOCATION%\eclipsec.exe ^
-nosplash ^
-application org.eclipse.cdt.managedbuilder.core.headlessbuild ^
-data %ECLIPSE_WS% ^
-E DEFAULT_MAKE_PATH=%DEFAULT_MAKE_PATH% ^
-E DEFAULT_ARM_XGCC=%DEFAULT_ARM_XGCC% ^
-E IO32STM32F103USB_XGCC_PATH=${DEFAULT_MAKE_PATH};${DEFAULT_ARM_XGCC} ^
-E IO32STM32F103USB_CUBEF1_SDK=%EXTERNAL_LIBS%/ST/STM32Cube_FW_F1_V1.6.0 ^
-build IO32STM32F103USB/DebugBluePill ^
-printErrorMarkers ^
--launcher.ini %ECLIPSE_LOCATION%eclipse.ini ^
--launcher.suppressErrors

endlocal

GOTO:eof

:importProject
  set _PROJECT_TO_IMPORT=%~f1
  echo [%ECLIPSE_BUILD%] Importing %_PROJECT_TO_IMPORT%
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
  echo pathvariable.%_VAR_NAME%=%_LOCATION_FINAL%>> %_RES_FILE%
  goto:EOF

:normalizePath
  set %2=%~f1
  goto:EOF

:printUsage
  echo EclipseBuild.bat <ECLIPSE PATH> <TOOLS PATH> <LIBS PATH>
  echo ECLIPSE PATH - Path to folder with Eclipse executables
  echo   TOOLS PATH - Path to folder with GCC compiler(s) and make  
  echo    LIBS PATH - Path to folder with external libraries
  echo Example:
  echo EclipseBuild.bat E:\Eclipse X:\Programs\CompileTools D:\ExternalLibs\ProtomorphLibs
  goto:EOF