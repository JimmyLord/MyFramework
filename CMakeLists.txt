cmake_minimum_required( VERSION 3.7 )

# Project
project( MyFramework_Project VERSION 0.1 LANGUAGES CXX )

# Framework source files.
file( GLOB_RECURSE sourcefiles
    "MyFramework/SourceCommon/*.cpp"
    "MyFramework/SourceLinux/*.cpp"
    "Libraries/Box2D/Box2D/Box2D/*.cpp"
    "Libraries/cJSON/*.cpp"
    "Libraries/LodePNG/*.cpp"
)
list( REMOVE_ITEM sourcefiles "${CMAKE_SOURCE_DIR}/MyFramework/SourceCommon/GraphicsWrappers/DXWrapper.cpp" )
list( REMOVE_ITEM sourcefiles "${CMAKE_SOURCE_DIR}/MyFramework/SourceLinux/Screenshot.cpp" )

# Extra files specific to WxWidgets builds.
if( CMAKE_BUILD_TYPE MATCHES wxDebug OR CMAKE_BUILD_TYPE MATCHES wxRelease )
    file( GLOB_RECURSE widgetfiles "MyFramework/SourceWidgets/*.cpp" )
    set( sourcefiles ${sourcefiles} ${widgetfiles} )
endif()

# Library
add_library( MyFramework ${sourcefiles} )

#set_property( GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time" )

# Framework include directories.
target_include_directories( MyFramework PUBLIC
    "MyFramework/SourceCommon"
    "MyFramework/SourceLinux"
    "MyFramework/SourceWidgets"
    "Libraries/Box2D/Box2D"
    "Libraries/cJSON"
    "Libraries/LodePNG"
    "Libraries/wxWidgets/include"
    "Libraries/wxWidgets/gtk-build-debug/lib/wx/include/gtk3-unicode-static-3.1"
)

# Base preprocessor defines.
target_compile_definitions( MyFramework PRIVATE MYFW_LINUX=1 )

if( CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES wxDebug )
    target_compile_definitions( MyFramework PRIVATE _DEBUG=1 )
endif()

# WxWidgets specific settings.
if( CMAKE_BUILD_TYPE MATCHES wxDebug OR CMAKE_BUILD_TYPE MATCHES wxRelease )

    # Preprocessor defines.
    target_compile_definitions( MyFramework PRIVATE MYFW_USING_WX=1 _FILE_OFFSET_BITS=64 __WXGTK__ )

    # Set base CMake build type to debug or release. not sure if works.
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
