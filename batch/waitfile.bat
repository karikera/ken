
:waitloop
IF EXIST "%1" GOTO waitloopend
timeout /t 1
goto waitloop
:waitloopend
