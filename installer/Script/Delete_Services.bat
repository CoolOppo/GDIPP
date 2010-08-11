sc stop gdipp_svc_32
sc delete gdipp_svc_32
sc stop gdipp_svc_64
sc delete gdipp_svc_64

ping -n 3 -w 1 localhost > NUL
