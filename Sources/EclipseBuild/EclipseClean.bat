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
set ECLIPSE_CLEAN=%~nx0
set ECLIPSE_WS=%~dp0..\EclipseWS

echo [%ECLIPSE_CLEAN%] Initializing %ECLIPSE_WS%

echo [%ECLIPSE_CLEAN%] Removing any content in %ECLIPSE_WS%
call :emptyFolder %ECLIPSE_WS%

goto:EOF

:emptyFolder
  set FOLDER_TO_EMPTY=%~1
  IF EXIST %FOLDER_TO_EMPTY% (
    FOR /D %%p IN ("%FOLDER_TO_EMPTY%\*.*") DO (
      rmdir "%%p" /S /Q
    )
    del %FOLDER_TO_EMPTY%\* /F /Q
  )
  goto:EOF

