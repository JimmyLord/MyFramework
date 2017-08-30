
if [[ ! $1 =~ ^(Debug|Release|wxDebug|wxRelease)$ ]]; then
    echo "Specify a build configuration: Debug, Release, wxDebug, wxRelease"
    exit 2
fi

BuildConfiguration="$1"

if [[ $BuildConfiguration == wxDebug ]]; then
    if [ ! -d "Libraries/wxWidgets/gtk-build-debug" ]; then
        pushd Libraries/wxWidgets > /dev/null
            echo "$(tput setaf 5)==> Building wxWidgets Debug... this could take a while$(tput sgr0)"
            mkdir gtk-build-debug
            cd gtk-build-debug
            ../configure --enable-debug --with-opengl --disable-shared --prefix=$(pwd) -with-gtk=3
            make
        popd > /dev/null
    fi
fi

if [[ $BuildConfiguration == wxRelease ]]; then
    if [ ! -d "Libraries/wxWidgets/gtk-build-release" ]; then
        pushd Libraries/wxWidgets > /dev/null
            echo "$(tput setaf 5)==> Building wxWidgets Release... this could take a while$(tput sgr0)"
            mkdir gtk-build-release
            cd gtk-build-release
            ../configure --with-opengl --disable-shared --prefix=$(pwd) -with-gtk=3
            make
        popd > /dev/null
    fi
fi

echo "$(tput setaf 2)==> Building MyFramework$(tput sgr0)"

if [ ! -d "build" ]; then
    mkdir build
fi

if [ ! -d build/$BuildConfiguration ]; then
    mkdir build/$BuildConfiguration
    pushd build/$BuildConfiguration > /dev/null
        cmake -DCMAKE_BUILD_TYPE=$BuildConfiguration ../..
    popd > /dev/null
fi

pushd build/$BuildConfiguration > /dev/null
    make
popd > /dev/null
