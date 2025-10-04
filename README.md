## Building

if for some reason you want to build this project.

git clone --recurse-submodules https://github.com/JamesWoolbright/2d-block-game

cd 2d-block-game

mkdir build && cd build

cmake ..

cmake --build . --config Release[^1]

[^1]: I have no idea if the below actually works because I use msbuild. Yes filthy windows user, I know.
