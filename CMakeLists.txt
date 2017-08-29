cmake_minimum_required( VERSION 3.7 )

project( MyFramework_Project VERSION 0.1 LANGUAGES CXX )

file( GLOB_RECURSE sourcefiles
    "MyFramework/SourceCommon/*.cpp"
    "MyFramework/SourceLinux/*.cpp"
    "MyFramework/SourceWidgets/*.cpp"
    "Libraries/Box2D/Box2D/Box2D/*.cpp"
    "Libraries/cJSON/*.cpp"
    "Libraries/LodePNG/*.cpp"
)
list( REMOVE_ITEM sourcefiles "${CMAKE_SOURCE_DIR}/MyFramework/SourceCommon/GraphicsWrappers/DXWrapper.cpp" )
list( REMOVE_ITEM sourcefiles "${CMAKE_SOURCE_DIR}/MyFramework/SourceLinux/Screenshot.cpp" )

add_library( MyFramework ${sourcefiles} )

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

target_compile_definitions( MyFramework PRIVATE MYFW_LINUX=1 )

if( CMAKE_BUILD_TYPE MATCHES Debug )
    target_compile_definitions( MyFramework PRIVATE _DEBUG=1 )
endif()

if( CMAKE_BUILD_TYPE MATCHES wxDebug )
    target_compile_definitions( MyFramework PRIVATE MYFW_USING_WX=1 _FILE_OFFSET_BITS=64 __WXGTK__ )
elseif( CMAKE_BUILD_TYPE MATCHES wxRelease )
    target_compile_definitions( MyFramework PRIVATE MYFW_USING_WX=1 _FILE_OFFSET_BITS=64 __WXGTK__ )
endif()

if( CMAKE_BUILD_TYPE MATCHES wxDebug )
    set( CMAKE_BUILD_TYPE Debug )
elseif( CMAKE_BUILD_TYPE MATCHES wxRelease )
    set( CMAKE_BUILD_TYPE Release )
endif()
