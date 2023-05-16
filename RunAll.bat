@echo off
echo Starting Master Server minamised listening on 127.0.0.1:6000
start /MIN %~dp0\x64\Debug\JNetDemoMasterServer.exe

echo Starting Balanced Server 1 minamised listening on 127.0.0.1:6050/1
start /MIN %~dp0\x64\Debug\JNetDemoBalancedServer.exe 127.0.0.1 6000 "Balanced Server 1" 127.0.0.1 6050 6051

echo Starting Game Session 1 minamised listening on 127.0.0.1:6052
start /MIN %~dp0\x64\Debug\JNetDemoGameSession.exe 127.0.0.1 6051 "Game Session 1" 127.0.0.1 6052

echo Starting Client
%~dp0\x64\Debug\JNetDemoClient.exe