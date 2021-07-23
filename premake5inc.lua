-- This is not a complete premake5 lua script, it's meant to be included from another script that defines the workspace.
-- Like this, for example:
--     local rootDir = os.getcwd()
--     os.chdir( "../Engine/" )
--     include( "premake5inc.lua" )
--     os.chdir( rootDir )

-- To exclude Box2D, set 'PremakeConfig_UseBox2D' to false and add 'defines "MYFW_USE_BOX2D=0"' to your project.
if PremakeConfig_UseBox2D == nil then
    PremakeConfig_UseBox2D = true
end

if PremakeConfig_UseMemoryTracker == nil then
    PremakeConfig_UseMemoryTracker = true
end

project "MyFramework"
if PremakeConfig_BuildFolder ~= nil then
    location            ( PremakeConfig_BuildFolder .. "/Framework" )
end
    configurations      { "Debug", "Release", "EditorDebug", "EditorRelease" }
    uuid                "016089D0-2136-4A3D-B08C-5031542BE1D7"
    kind                "StaticLib"
    language            "C++"
    targetdir           "%{wks.location}/Output/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    objdir              "%{wks.location}/Output/Intermediate/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    pchheader           "MyFrameworkPCH.h"
    pchsource           "MyFramework/SourceCommon/MyFrameworkPCH.cpp"

    includedirs {
        "MyFramework/SourceCommon",
        "Libraries/b2Settings",
        "Libraries/Box2D/include",
        "Libraries/Box2D/src",
        "Libraries/OpenAL/include",
    }

    files {
        "MyFramework/SourceCommon/**.cpp",
        "MyFramework/SourceCommon/**.h",
        "Libraries/b2Settings/box2d/b2Settings.h",
        "Libraries/cJSON/cJSON.c",
        "Libraries/cJSON/cJSON.h",
        "Libraries/expr_eval/expr_eval.h",
        "Libraries/LodePNG/lodepng.cpp",
        "Libraries/LodePNG/lodepng.h",
        "Libraries/mtrand/mtrand.cpp",
        "Libraries/mtrand/mtrand.h",
        "Libraries/mtrand/mtreadme.txt",
        "Libraries/OpenSimplexInC/open-simplex-noise.c",
        "Libraries/OpenSimplexInC/open-simplex-noise.h",
        "README.md",
        "premake5inc.lua",
        "Libraries/premake5inc-box2d.lua",
    }

    vpaths {
        -- Place these files in the root of the project.
        [""] = {
            "README.md",
            "premake5inc.lua",
        },
        -- Place the SourceCommon, SourceEditor and SourceWindows folders in the root of the project.
        ["*"] = {
            "MyFramework",
        },
        -- Place the Libraries folder in the root of the project.
        ["Libraries*"] = {
            "Libraries",
        },
    }

    filter { "files:Libraries/**" }
        flags           "NoPCH"

    filter "action:vs*"
        platforms       { "x86", "x64" }
        defines         "MYFW_WINDOWS"
        systemversion   "latest"
        characterset    "MBCS"
        files {
            "Libraries/pthreads-w32/include/pthread.h",
            "Libraries/pthreads-w32/include/sched.h",
            "Libraries/pthreads-w32/include/semaphore.h",
            "MyFramework/SourceWindows/**.cpp",
            "MyFramework/SourceWindows/**.h",
        }

    filter "action:android-studio"
        defines         "MYFW_ANDROID"
        files {
            "MyFramework/SourceAndroid/**.cpp",
            "MyFramework/SourceAndroid/**.c",
            "MyFramework/SourceAndroid/**.h",
        }

    filter { "files:MyFramework/SourceCommon/DataTypes/ColorStructs.cpp"
            .. " or MyFramework/SourceCommon/DataTypes/Vector.cpp"
            .. " or MyFramework/SourceCommon/Helpers/TypeInfo.cpp"
            .. " or MyFramework/SourceCommon/Networking/GameService_MyServer.cpp"
            .. " or MyFramework/SourceCommon/Networking/GameService_ScoreLoop.cpp"
            .. " or MyFramework/SourceCommon/Renderers/Old/DXWrapper.*"
            .. " or MyFramework/SourceCommon/Sound/SoundPlayerOpenAL.*"
            .. " or MyFramework/SourceCommon/Sprites/SpriteSheet_XYZVertexColor.cpp"
            .. " or MyFramework/SourceWindows/SoundPlayerSDL.*"
           }
        flags           "ExcludeFromBuild"

    filter "files:Libraries/LodePNG/**"
        disablewarnings { "4551", "4334", "4267" }

    filter "configurations:Debug or EditorDebug"
        defines         "_DEBUG"
        symbols         "on"

    filter "configurations:Release or EditorRelease"
        defines         "NDEBUG"
        optimize        "Full"

    filter "configurations:EditorDebug or EditorRelease"
        defines         { "MYFW_EDITOR", "MYFW_USING_IMGUI" }

if MyFrameworkPremakeConfig_ForceIncludeEditorFiles == true then
    filter {}
else
    filter "configurations:EditorDebug or EditorRelease"
end
        files {
            "MyFramework/SourceEditor/**.cpp",
            "MyFramework/SourceEditor/**.h",
        }

if PremakeConfig_UseBox2D == true then
    filter {}
        defines         "MYFW_USE_BOX2D"
else
    filter {}
        defines         "MYFW_USE_BOX2D=0"
    filter { "files:MyFramework/SourceCommon/Physics/Box2D**" }
        flags           "ExcludeFromBuild"
end

if PremakeConfig_UseMemoryTracker == true then
    filter "configurations:Debug or EditorDebug or EditorRelease"
        defines         "MYFW_USE_MEMORY_TRACKER"
end
