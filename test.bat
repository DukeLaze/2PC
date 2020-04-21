echo "Building required executables (only tested with MINGW-64 Posix, g++)"
cd src/testtc
g++ tc.cpp -o tc -lws2_32 -std=c++17
cd ..
cd ..
cd src/testclient
g++ client.cpp -o client -lws2_32 -std=c++17
cd ..
cd ..
cd src/testparticipant
g++ participant.cpp -o participant -lws2_32 -std=c++17
cd ..
cd ..

echo "Testing with 1 TC, 3 participants and 1 client making 3 requests. 2 requests shall succeed and 1 shall fail."
mkdir ".\src\testparticipant\1"
mkdir ".\src\testparticipant\2"
copy /B ".\src\testparticipant\participant.exe" ".\src\testparticipant\1\participant.exe" /B
copy /B ".\src\testparticipant\participant.exe" ".\src\testparticipant\2\participant.exe" /B

echo "Running TC"
start /D ".\src\testtc" "" ".\src\testtc\tc.exe"
timeout /t 1 /nobreak > NUL
start /D ".\src\testparticipant" "" "".\src\testparticipant\participant.exe"
timeout /t 1 /nobreak > NUL
start  /D ".\src\testparticipant\1" "" "".\src\testparticipant\1\participant.exe"
timeout /t 1 /nobreak > NUL
start  /D ".\src\testparticipant\2" "" "".\src\testparticipant\2\participant.exe"
timeout /t 1 /nobreak > NUL
start  /D ".\src\testclient\" "" ".\src\testclient\client.exe"

echo "Beklager at jeg ikke rydder opp etter de genererte filene, men gikk tom for tid :( |||RD||| |||DEL|||"