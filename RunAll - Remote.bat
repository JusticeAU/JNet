@echo off
echo Starting Master Server minamised listening on 127.0.0.1:6000
start /MIN %~dp0\x64\Debug\JNetDemoMasterServer.exe

echo Starting Balanced Server 1 minamised listening on 127.0.0.1:6050-1
start /MIN %~dp0\x64\Debug\JNetDemoBalancedServer.exe 127.0.0.1 6001 "Australia 1" home.justinetchell.com 6050 6051 AU

echo Starting Game Session 1 minamised listening on 127.0.0.1:6052
start /MIN %~dp0\x64\Debug\JNetDemoGameSession.exe 127.0.0.1 6051 "AU Game Session 1" home.justinetchell.com 6052
echo Starting Game Session 1 minamised listening on 127.0.0.1:6053
start /MIN %~dp0\x64\Debug\JNetDemoGameSession.exe 127.0.0.1 6051 "AU Game Session 2" home.justinetchell.com 6053
echo Starting Game Session 1 minamised listening on 127.0.0.1:6054
start /MIN %~dp0\x64\Debug\JNetDemoGameSession.exe 127.0.0.1 6051 "AU Game Session 3" home.justinetchell.com 6054



echo Starting Balanced Server 2 minamised listening on 127.0.0.1:6060-1
start /MIN %~dp0\x64\Debug\JNetDemoBalancedServer.exe 127.0.0.1 6001 "United States 1" home.justinetchell.com 6060 6061 US DEFAULT

echo Starting Game Session 1 minamised listening on 127.0.0.1:6062
start /MIN %~dp0\x64\Debug\JNetDemoGameSession.exe 127.0.0.1 6061 "US Game Session 1" home.justinetchell.com 6062
echo Starting Game Session 1 minamised listening on 127.0.0.1:6063
start /MIN %~dp0\x64\Debug\JNetDemoGameSession.exe 127.0.0.1 6061 "US Game Session 2" home.justinetchell.com 6063
echo Starting Game Session 1 minamised listening on 127.0.0.1:6064
start /MIN %~dp0\x64\Debug\JNetDemoGameSession.exe 127.0.0.1 6061 "US Game Session 3" home.justinetchell.com 6064

echo Starting Client
%~dp0\x64\Debug\JNetDemoClient.exe