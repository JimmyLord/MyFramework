cmake_minimum_required( VERSION 3.7 )

#set( CMAKE_BUILD_TYPE Debug )

project( MyFramework_Project VERSION 0.1 LANGUAGES CXX )

file( GLOB_RECURSE sourcefiles
    "MyFramework/SourceCommon/*.cpp"
    "MyFramework/SourceLinux/*.cpp"
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
    "Libraries/Box2D/Box2D"
    "Libraries/cJSON"
    "Libraries/LodePNG"
)

target_compile_definitions( MyFramework PRIVATE MYFW_LINUX=1 )
