========================================================
Description
========================================================
Redirect all traffic found on port 80 to SOCKS 5 port 5556


========================================================
Requirements
========================================================
Visual studio redistribuitables :
	https://aka.ms/vs/16/release/vc_redist.x64.exe
a working SOCKS 5 proxy
	c:\Windows\System32\OpenSSH\ssh.exe -i ec2keypai2.pem ec2-user@3.22.221.46 -D 5556


========================================================
Instalation
========================================================
No special instalation is required


========================================================
Usage
========================================================
Executable is not configurable
- You need to run the executable as administrator or else the driver can not load
- make sure the SOCKS 5 proxy is running on port 5556
- make any traffic on port 80. Example : http://3.22.221.46/t2.html
- check the logs of the traffic, it should say that the request came from the SSH proxy and not from where the request was made