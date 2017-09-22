pushd Libraries\wxWidgets\build\msw
call "C:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/vcvarsall.bat" x64
nmake /f makefile.vc BUILD=debug TARGET_CPU=AMD64
nmake /f makefile.vc BUILD=release TARGET_CPU=AMD64
cd ../../lib
rename vc_x64_lib vc100_x64_lib
popd