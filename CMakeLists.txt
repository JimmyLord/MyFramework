cmake_minimum_required( VERSION 3.4.1 )

# Project
#project( MyFramework_Project VERSION 0.1 LANGUAGES C CXX )

# Framework source files.
file( GLOB_RECURSE sourceFiles
    "MyFramework/SourceCommon/*.cpp"
    "Libraries/Box2D/Box2D/*.cpp"
    "Libraries/cJSON/*.cpp"
    "Libraries/LodePNG/*.cpp"
    "Libraries/OpenSimplexInC/open-simplex-noise.c"
)
list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyFramework/SourceCommon/GraphicsWrappers/DXWrapper.cpp" )

# Extra files specific to Android builds.
if( NOT ${ANDROID_ABI} STREQUAL "" )
    list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyFramework/SourceCommon/Sound/SoundPlayerOpenAL.cpp" )
    file( GLOB_RECURSE platformFiles "MyFramework/SourceAndroid/*.cpp" )
    list( REMOVE_ITEM platformFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyFramework/SourceAndroid/SoundPlayer.cpp" )
    set( sourceFiles ${sourceFiles} ${platformFiles} )
    file( GLOB_RECURSE platformFiles "MyFramework/SourceAndroid/*.c" )
    set( sourceFiles ${sourceFiles} ${platformFiles} )
endif()

# Extra files specific to Linux builds.
if( CMAKE_BUILD_TYPE MATCHES Linux )
    file( GLOB_RECURSE platformFiles "MyFramework/SourceLinux/*.cpp" )
    set( sourceFiles ${sourceFiles} ${platformFiles} )
    list( REMOVE_ITEM sourceFiles "${CMAKE_CURRENT_SOURCE_DIR}/MyFramework/SourceLinux/Screenshot.cpp" )
endif()

# Extra files specific to WxWidgets builds.
if( CMAKE_BUILD_TYPE MATCHES wxDebug OR CMAKE_BUILD_TYPE MATCHES wxRelease )
    file( GLOB_RECURSE widgetfiles "MyFramework/SourceWidgets/*.cpp" )
    set( sourceFiles ${sourceFiles} ${widgetfiles} )
endif()

# Library
add_library( MyFramework STATIC ${sourceFiles} )

#set_property( GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time" )

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

#############
# Preprocessor defines.
#############

# Android defines.
if( NOT ${ANDROID_ABI} STREQUAL "" )
    target_compile_definitions( MyFramework PRIVATE MYFW_ANDROID=1 )
    #target_link_libraries( MyFramework android log GLESv2 OpenSLES EGL ) #c m gcc )
endif()

# Linux defines.
if( CMAKE_BUILD_TYPE MATCHES Linux )
    target_compile_definitions( MyFramework PRIVATE MYFW_LINUX=1 )
endif()

if( CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES wxDebug )
    target_compile_definitions( MyFramework PRIVATE _DEBUG=1 )
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

# cotire (Compile time reducer)
include( Libraries/Cotire/CMake/cotire.cmake )
set_target_properties( MyFramework PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "MyFramework/SourceCommon/CommonHeader.h" )
cotire( MyFramework )
