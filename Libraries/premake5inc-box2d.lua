-- This is not a complete premake5 lua script, it's meant to be included from another script that defines the workspace.
-- Like this, for example:
--     local rootDir = os.getcwd();
--     os.chdir( "../Engine/" )
--     include( "premake5inc.lua" )
--     os.chdir( rootDir )

if PremakeConfig_BuildFolder == nil then
    PremakeConfig_BuildFolder = "$SolutionDir/"
end

project "Box2D"
    location            ( PremakeConfig_BuildFolder .. "/Box2D" )
    configurations      { "Debug", "Release" }
    configmap           {
                            ["EditorDebug"] = "Debug",
                            ["EditorRelease"] = "Release"
                        }
    uuid                "98400D17-43A5-1A40-95BE-C53AC78E7694"
    kind                "StaticLib"
    language            "C++"
    targetdir           "%{wks.location}/Output/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"
    objdir              "%{wks.location}/Output/Intermediate/%{cfg.platform}-%{prj.name}-%{cfg.buildcfg}"

    includedirs {
        "b2Settings",
        "Box2D/include",
        "Box2D/src",
    }

    files {
        "Box2D/src/**",
    }

    filter "action:vs*"
        platforms       { "x86", "x64" }
        systemversion   "latest"
        characterset    "MBCS"

    filter "configurations:Debug"
        defines         "_DEBUG"
        symbols         "on"

    filter "configurations:Release"
        defines         "NDEBUG"
        optimize        "Full"

-- String pooling is causing build errors in VS2019, don't feel like looking into it, so disabling.
--    filter { "system:windows", "configurations:Release" }
--        buildoptions    { "\\GF" } -- /GF -> Enable String Pooling 
