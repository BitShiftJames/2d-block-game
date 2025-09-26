
setlocal

set SRC=C:/Users/byte/CodingStuffs/Projects/main_game/src/main.cpp
set OUT=C:/Users/byte/CodingStuffs/Projects/main_game/src/main.exe

set INCLUDE="C:/program files/mingw64/include/raylib/src/"
set RAYLIB="C:/Program Files/mingw64/include/raylib/src/libraylib.a"
set LIBS=-lgdi32 -lwinmm -lopengl32 -luser32

gcc %SRC% -I%INCLUDE% %RAYLIB% %LIBS% -o %OUT% 

endlocal
