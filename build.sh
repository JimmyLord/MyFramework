
if [[ $1 == wxDebug ]]; then
    if [ ! -d "Libraries/wxWidgets/gtk-build-debug" ]; then
        pushd Libraries/wxWidgets
        echo "Building wxWidgets Debug... this could take a while."
        mkdir gtk-build-debug
        cd gtk-build-debug
        ../configure --enable-debug --with-opengl --disable-shared --prefix=$(pwd) -with-gtk=3
        make
        popd
    fi
fi

if [[ $1 == wxRelease ]]; then
    if [ ! -d "Libraries/wxWidgets/gtk-build-release" ]; then
        pushd Libraries/wxWidgets
        echo "Building wxWidgets Release... this could take a while."
        mkdir gtk-build-release
        cd gtk-build-release
        ../configure --with-opengl --disable-shared --prefix=$(pwd) -with-gtk=3
        make
        popd
    fi
fi

echo "Building MyFramework."

if [ ! -d "build" ]; then
    mkdir build
    cd build
    cmake ..
else
    cd build
fi
make