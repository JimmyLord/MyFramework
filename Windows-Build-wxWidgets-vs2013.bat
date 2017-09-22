pushd Libraries\wxWidgets\build\msw
call "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/vcvarsall.bat"
nmake -f makefile.vc BUILD=debug
nmake -f makefile.vc BUILD=release
cd ../../lib
rename vc_lib vc120_lib
popd
