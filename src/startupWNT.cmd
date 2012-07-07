@echo off
setlocal

set port=5005
if defined %1 set port=%1

if exist SHUTDOWN.TXT del SHUTDOWN.TXT
if exist ..\log\envy.log del ..\log\envy.log

:while1
..\src\envy2 %port% > ..\log\envy.log
if not exist SHUTDOWN.TXT goto :while1

del SHUTDOWN.TXT
endlocal
