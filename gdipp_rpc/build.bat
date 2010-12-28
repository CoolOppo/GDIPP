@echo off
set vc_vars="%VS90COMNTOOLS%..\..\VC\vcvarsall.bat"

call %vc_vars% x86
echo.
mkdir Win32
midl /nologo /win32 /target NT51 /W3 /acf gdipp_rpc.acf /h ..\gdipp_rpc.h /out Win32 gdipp_rpc.idl
echo.

call %vc_vars% x64
echo.
mkdir x64
midl /nologo /x64 /target NT51 /W3 /acf gdipp_rpc.acf /h ..\gdipp_rpc.h /out x64 gdipp_rpc.idl
echo.

pause
