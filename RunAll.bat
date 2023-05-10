start /MIN %~dp0\x64\Debug\JNetDemoMasterServer.exe
start /MIN %~dp0\x64\Debug\JNetDemoBalancedServer.exe 127.0.0.1 6000 "Balanced Server 1" 127.0.0.1 6051
start /MIN %~dp0\x64\Debug\JNetDemoBalancedServer.exe 127.0.0.1 6000 "Balanced Server 2" 127.0.0.1 6052
%~dp0\x64\Debug\JNetDemoClient.exe