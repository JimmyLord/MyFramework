-- This is not a complete premake5 lua script, it's meant to be included from another script that defines the workspace.
-- Like this, for example:
--     local rootDir = os.getcwd();
--     os.chdir( "../Engine/" )
--     include( "premake5inc.lua" )
--     os.chdir( rootDir )

project "MyFramework"
    configurations      { "Debug", "Release", "EditorDebug", "EditorRelease" }
    uuid                "016089D0-2136-4A3D-B08C-5031542BE1D7"
    kind                "StaticLib"
    language            "C++"
    targetdir           "$(SolutionDir)Output/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    objdir              "$(SolutionDir)Output/Intermediate/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    pchheader           "MyFrameworkPCH.h"
    pchsource           "MyFramework/SourceCommon/MyFrameworkPCH.cpp"

    includedirs {
        "MyFramework/SourceCommon",
		"Libraries/b2Settings",
		"Libraries/Box2D",
		"Libraries/OpenAL/include",
    }

    files {
        "MyFramework/SourceCommon/**.cpp",
        "MyFramework/SourceCommon/**.h",
        "Libraries/b2Settings/b2Settings.h",
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
        "Libraries/pthreads-w32/pthread.h",
        "Libraries/pthreads-w32/sched.h",
        "Libraries/pthreads-w32/semaphore.h",
        "README.md",
        "premake5inc.lua",
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

    filter "system:windows"
        platforms       { "x86", "x64" }
        defines         "MYFW_WINDOWS"
        systemversion   "latest"
        characterset    "MBCS"
        files {
            "MyFramework/SourceWindows/**.cpp",
            "MyFramework/SourceWindows/**.h",
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

    filter "configurations:Debug or EditorDebug"
        defines         "_DEBUG"
        symbols         "on"

    filter "configurations:Release or EditorRelease"
        defines         "NDEBUG"
        optimize        "Full"

    filter "configurations:EditorDebug or EditorRelease"
        defines         { "MYFW_EDITOR", "MYFW_USING_IMGUI" }

    filter "configurations:EditorDebug or EditorRelease"
        files {
            "MyFramework/SourceEditor/**.cpp",
            "MyFramework/SourceEditor/**.h",
        }
