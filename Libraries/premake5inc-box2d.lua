-- This is not a complete premake5 lua script, it's meant to be included from another script that defines the workspace.
-- Like this, for example:
--     local rootDir = os.getcwd();
--     os.chdir( "../Engine/" )
--     include( "premake5inc.lua" )
--     os.chdir( rootDir )

project "Box2D"
    configurations      { "Debug", "Release" }
    configmap           {
                            ["EditorDebug"] = "Debug",
                            ["EditorRelease"] = "Release"
                        }
    uuid                "98400D17-43A5-1A40-95BE-C53AC78E7694"
    kind                "StaticLib"
    language            "C++"
    targetdir           "$(SolutionDir)Output/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    objdir              "$(SolutionDir)Output/Intermediate/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"

    includedirs {
        "$(SolutionDir)../Framework/Libraries/b2Settings",
        "$(SolutionDir)../Framework/Libraries/Box2D",
    }

    files {
        "Box2D/Box2D/**",
    }

    filter "system:windows"
        platforms       { "x86", "x64" }
        systemversion   "latest"
        characterset    "MBCS"

    filter "configurations:Debug"
        defines         "_DEBUG"
        symbols         "on"

    filter "configurations:Release"
        defines         "NDEBUG"
        optimize        "Full"

    filter { "system:windows", "configurations:Release" }
        buildoptions    { "\\GF" } -- /GF -> Enable String Pooling 
