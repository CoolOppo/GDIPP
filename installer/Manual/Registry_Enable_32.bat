if defined gdipp_manual (
	mklink "%SystemRoot%\SysWOW64\EasyHook32.dll" "%~f1\EasyHook32.dll"
	mklink "%SystemRoot%\SysWOW64\gdipp_common_32.dll" "%~f1\gdipp_common_32.dll"
	mklink "%SystemRoot%\SysWOW64\gdipp_setting.xml" "%~f1\gdipp_setting.xml"

	reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "AppInit_DLLs" /t REG_SZ /d "%~f1\gdimm_32.dll" /f
	reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "LoadAppInit_DLLs" /t REG_DWORD /d 1 /f
	
	ping -n 3 -w 1 localhost > NUL
) else (
	set gdipp_manual=1
	call "%0" "%~dp0.."
	set gdipp_manual=
)
