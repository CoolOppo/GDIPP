if defined gdipp_manual (
	sc create gdipp_svc binpath= "%~f1\gdipp_svc_64.exe" DisplayName= "gdipp Service"
	
	ping -n 3 -w 1 localhost > NUL
) else (
	set gdipp_manual=1
	call %0 "%~dp0.."
	set gdipp_manual=
)
