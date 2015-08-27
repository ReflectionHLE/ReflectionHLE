@echo off
if "%1"=="" goto help
del %1.SYM
del %1.DSK
del OBJ\%1\*.*
md OBJ
md OBJ\%1
bc %1.PRJ
goto end
:help
echo Missed some project name (without file extension)
:end
