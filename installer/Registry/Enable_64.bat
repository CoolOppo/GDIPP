if defined gdipp_registry (
  setx Path "%Path%;%~f1" /m
) else (
  reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "AppInit_DLLs" /t REG_SZ /d "gdimm_64.dll" /f
  reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v "LoadAppInit_DLLs" /t REG_DWORD /d 1 /f
  reg add "HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows NT\CurrentVersion\Windows" /v "AppInit_DLLs" /t REG_SZ /d "gdimm_32.dll" /f
  reg add "HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows NT\CurrentVersion\Windows" /v "LoadAppInit_DLLs" /t REG_DWORD /d 1 /f

  set gdipp_registry=1
  call "%0" "%~dp0.."
  set gdipp_registry=
)