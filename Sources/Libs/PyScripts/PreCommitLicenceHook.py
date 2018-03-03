####################################################################################
## BSD 3-Clause License                                                           ##
##                                                                                ##
## Copyright (c) 2018, pa.eeapai@gmail.com                                        ##
## All rights reserved.                                                           ##
##                                                                                ##
## Redistribution and use in source and binary forms, with or without             ##
## modification, are permitted provided that the following conditions are met:    ##
##                                                                                ##
## * Redistributions of source code must retain the above copyright notice, this  ##
##   list of conditions and the following disclaimer.                             ##
##                                                                                ##
## * Redistributions in binary form must reproduce the above copyright notice,    ##
##   this list of conditions and the following disclaimer in the documentation    ##
##   and/or other materials provided with the distribution.                       ##
##                                                                                ##
## * Neither the name of the copyright holder nor the names of its                ##
##   contributors may be used to endorse or promote products derived from         ##
##   this software without specific prior written permission.                     ##
##                                                                                ##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"    ##
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE      ##
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE ##
## DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE   ##
## FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL     ##
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR     ##
## SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER     ##
## CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,  ##
## OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  ##
## OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.           ##
####################################################################################
import sys
import os
import shutil

"""
Every source file in this repository starts with licence.
Licence are first lines in the file starting with comment characters
until the line that doesn't start with comment character.
"""

licence = """BSD 3-Clause License

Copyright (c) 2018, pa.eeapai@gmail.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

licenceLines = licence.splitlines()
maxLicenceLineLength = 0
for line in licenceLines:
  if len(line) > maxLicenceLineLength:
    maxLicenceLineLength = len(line)

class Lang:
  def __init__(self, lang, exts, commentStart, commentFill, commentEnd):
    self.lang = lang
    self.exts = exts
    self
    self.commentStart = commentStart
    self.commentFill = commentFill
    self.commentEnd = commentEnd

languages = [Lang("Python"    , ["py"]            , "##", "#", "##"),
             Lang("C/C++"     , ["c", "cpp", "h"] , "//", "/", "//"),
             Lang("C#"        , ["cs"]            , "//", "/", "//"),
             Lang("GCC Linker", ["ind"]           , "/*", "*", "*/"),
             Lang("ASM"       , ["s"]             , "##", "#", "##"),
             Lang("BATCH"     , ["bat"]           , " ::", "", "")
             ]

def addLicence(data, commentStart, commentFill, commentEnd):
  licenceFill = commentStart + commentFill + commentFill * maxLicenceLineLength + commentFill + commentEnd + "\n" 
  licenceInComments = licenceFill
  for licenceLine in licenceLines:
    licenceLine = licenceLine.rstrip()
    licenceLine = commentStart + " " + licenceLine + " " * (maxLicenceLineLength - len(licenceLine)) + " " + commentEnd + "\n"
    licenceInComments = licenceInComments + licenceLine
  return licenceInComments + licenceFill + data

def removeLicence(data, commentStart):
  lines = data.splitlines()
  firstCodeLine = 0
  for line in lines:
    if line.startswith(commentStart):
      firstCodeLine = firstCodeLine + 1
      continue
    break
  if not lines[firstCodeLine].rstrip():
    firstCodeLine = firstCodeLine + 1

  lines = lines[firstCodeLine:]
  return '\n'.join(lines)

def manageLicence(filePath, commentStart, commentFill, commentEnd):
  backupPath = filePath + ".backup"
  shutil.copyfile(filePath, backupPath) 
  data = ""
  with open(filePath, 'r') as original:
    data = original.read()
    data = removeLicence(data, commentStart)
    data = addLicence(data, commentStart, commentFill, commentEnd)
  with open(filePath, 'w') as modified:
    modified.write(data)
  os.remove(backupPath)

def licenceToFile(filePath):
  for lang in languages:
    for ext in lang.exts:
      if filePath.endswith("." + ext):
        manageLicence(filePath, lang.commentStart, lang.commentFill, lang.commentEnd)

def licenceToFilesFromFile(filepath):
  with open(filepath) as f:
    for line in f:
      line = line.rstrip()
      licenceToFile(line)

def licenceToFilesFromFolder(path):
  for root, dirs, files in os.walk(path):
    for file in files:
      filePath = os.path.join(root, file)
      if not -1 == filePath.find("\\."):
        continue     
      licenceToFile(filePath)

def main():
  path = sys.argv[1]
  if os.path.isfile(path):
    licenceToFilesFromFile(path)
  else:
    licenceToFilesFromFolder(path)


if __name__ == "__main__":
  main()
  os.system("pause")