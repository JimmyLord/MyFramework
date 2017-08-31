
# Parse command line arguments.
CleanBuild=dontclean
BuildConfiguration=none

for var in "$@"
do
    if [[ $var =~ ^(Debug|Release|wxDebug|wxRelease)$ ]]; then
        BuildConfiguration=$var;
    fi
    if [[ $var = clean ]]; then
        CleanBuild=clean;
    fi
done

# Exit if a valid build config wasn't specified.
if [[ ! $BuildConfiguration =~ ^(Debug|Release|wxDebug|wxRelease)$ ]]; then
    echo "Specify a build configuration: Debug, Release, wxDebug, wxRelease"
    echo "add 'clean' to clean that configuration"
    exit 2
fi

# Clean and exit.
if [[ $CleanBuild == clean ]]; then
    echo "$(tput setaf 5)==> Cleaning MyFramework$(tput sgr0)"
    rm -r build/$BuildConfiguration
    exit 1
fi

# Build wxWidgets. Clean must be done manually.
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

# Build MyFramework.
let NumJobs=$(nproc)*2
echo "$(tput setaf 2)==> Building MyFramework (make -j$NumJobs)$(tput sgr0)"

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
    make -j$NumJobs
popd > /dev/null
