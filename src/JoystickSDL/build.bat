
rem SET CMAKEEXE=%~dp0"../Tools/CMake/bin/cmake.exe"
SET CMAKEEXE=cmake.exe
SET VCPKG_DIR=D:/github/vcpkg

mkdir build64 & pushd build64
%CMAKEEXE% -G "Visual Studio 17 2022" .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_DIR%/scripts/buildsystems/vcpkg.cmake -DCMAKE_INSTALL_PREFIX=../../../
popd

%CMAKEEXE% --build build64 --config Release --target INSTALL