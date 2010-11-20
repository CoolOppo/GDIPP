sc stop gdipp_svc
sc delete gdipp_svc

ping -n 3 -w 1 localhost > NUL
