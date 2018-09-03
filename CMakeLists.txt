cmake_minimum_required( VERSION 3.4.1 )

###############################################################################
# Source code file list.
###############################################################################

# Framework source files.
file( GLOB_RECURSE sourceFiles
    "MyFramework/SourceCommon/*.cpp"
    "Libraries/Box2D/Box2D/*.cpp"
    "Libraries/cJSON/*.cpp"
    "Libraries/LodePNG/*.cpp"
    "Libraries/OpenSimplexInC/open-simplex-noise.c"
)
list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyFramework/SourceCommon/GraphicsWrappers/DXWrapper.cpp" )

# Android specific source files.
if( CMAKE_SYSTEM_NAME MATCHES Android )
    file( GLOB_RECURSE platformFiles "MyFramework/SourceAndroid/*.cpp" )
    set( sourceFiles ${sourceFiles} ${platformFiles} )
    file( GLOB_RECURSE platformFiles "MyFramework/SourceAndroid/*.c" )
    set( sourceFiles ${sourceFiles} ${platformFiles} )
    list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyFramework/SourceCommon/Sound/SoundPlayerOpenAL.cpp" )
    list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyFramework/SourceAndroid/SoundPlayer.cpp" )
endif()

# Linux specific source files.
if( CMAKE_SYSTEM_NAME MATCHES Linux )
    file( GLOB_RECURSE platformFiles "MyFramework/SourceLinux/*.cpp" )
    set( sourceFiles ${sourceFiles} ${platformFiles} )
    list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyFramework/SourceLinux/Screenshot.cpp" )
endif()

# Windows/VisualStudio specific source files.
if( CMAKE_SYSTEM_NAME MATCHES Windows )
    file( GLOB_RECURSE platformFiles "MyFramework/SourceWindows/*.cpp" )
    set( sourceFiles ${sourceFiles} ${platformFiles} )
    list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyFramework/SourceCommon/Sound/SoundPlayerOpenAL.cpp" )
    list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyFramework/SourceWindows/SoundPlayerSDL.cpp" )
endif()

# WxWidgets specific source files.
if( CMAKE_BUILD_TYPE MATCHES wxDebug OR CMAKE_BUILD_TYPE MATCHES wxRelease )
    file( GLOB_RECURSE widgetfiles "MyFramework/SourceWidgets/*.cpp" )
    set( sourceFiles ${sourceFiles} ${widgetfiles} )
endif()

###############################################################################
# Library setup.
###############################################################################

# Library
add_library( MyFramework STATIC ${sourceFiles} )

# Framework include directories.
target_include_directories( MyFramework PUBLIC
    "MyFramework/SourceCommon"
    "MyFramework/SourceLinux"
    "MyFramework/SourceWidgets"
    "Libraries/b2Settings"
    "Libraries/Box2D"
    "Libraries/cJSON"
    "Libraries/LodePNG"
    "Libraries/OpenSimplexInC"
    "Libraries/wxWidgets/include"
    "Libraries/wxWidgets/gtk-build-debug/lib/wx/include/gtk3-unicode-static-3.1"
)

###############################################################################
# Platform specific settings.
###############################################################################

# Android defines.
if( CMAKE_SYSTEM_NAME MATCHES Android )
    target_compile_definitions( MyFramework PRIVATE MYFW_ANDROID=1 )
    #target_link_libraries( MyFramework android log GLESv2 OpenSLES EGL ) #c m gcc )
endif()

# Linux defines.
if( CMAKE_SYSTEM_NAME MATCHES Linux )
    target_compile_definitions( MyFramework PRIVATE MYFW_LINUX=1 )
endif()

# Windows/VisualStudio defines.
if( CMAKE_SYSTEM_NAME MATCHES Windows )
    target_compile_definitions( MyFramework PRIVATE MYFW_WINDOWS=1 )
	add_definitions( -DUNICODE -D_UNICODE )
endif()

# WxWidgets specific settings.
if( CMAKE_BUILD_TYPE MATCHES wxDebug OR CMAKE_BUILD_TYPE MATCHES wxRelease )
    # Preprocessor defines.
    target_compile_definitions( MyFramework PRIVATE MYFW_USING_WX=1 _FILE_OFFSET_BITS=64 __WXGTK__ )

    # Set base CMake build type to debug or release.
    if( CMAKE_BUILD_TYPE MATCHES wxDebug )
        set( CMAKE_BUILD_TYPE Debug )
    elseif( CMAKE_BUILD_TYPE MATCHES wxRelease )
        set( CMAKE_BUILD_TYPE Release )
    endif()
endif()

# Set debug preprocessor define.
if( CMAKE_BUILD_TYPE MATCHES Debug )
    target_compile_definitions( MyFramework PRIVATE _DEBUG=1 )
endif()

# cotire (Compile time reducer)
include( Libraries/Cotire/CMake/cotire.cmake )
set_target_properties( MyFramework PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "MyFramework/SourceCommon/CommonHeader.h" )
cotire( MyFramework )
