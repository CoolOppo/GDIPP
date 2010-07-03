if defined gdipp_registry (
	mklink "%SystemRoot%\SysWOW64\EasyHook32.dll" "%~f1\EasyHook32.dll"
	mklink "%SystemRoot%\SysWOW64\gdipp_common_32.dll" "%~f1\gdipp_common_32.dll"
	mklink "%SystemRoot%\SysWOW64\gdipp_setting.xml" "%~f1\gdipp_setting.xml"
	mklink "%SystemRoot%\System32\EasyHook64.dll" "%~f1\EasyHook64.dll"
	mklink "%SystemRoot%\System32\gdipp_common_64.dll" "%~f1\gdipp_common_64.dll"
	mklink "%SystemRoot%\System32\gdipp_setting.xml" "%~f1\gdipp_setting.xml"

	reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "AppInit_DLLs" /t REG_SZ /d "%~f1\gdimm_64.dll" /f
	reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "LoadAppInit_DLLs" /t REG_DWORD /d 1 /f
	reg add "HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows NT\CurrentVersion\Windows" /v "AppInit_DLLs" /t REG_SZ /d "%~f1\gdimm_32.dll" /f
	reg add "HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows NT\CurrentVersion\Windows" /v "LoadAppInit_DLLs" /t REG_DWORD /d 1 /f

) else (
	set gdipp_registry=1
	call "%0" "%~dp0.."
	set gdipp_registry=
)
