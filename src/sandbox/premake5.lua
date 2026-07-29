project("Sandbox")
kind("ConsoleApp")
language("C++")
cppdialect("C++23")
staticruntime("off")

targetdir("../../bin/" .. outputdir .. "/%{prj.name}")
objdir("../../bin-int/" .. outputdir .. "/%{prj.name}")

files({
	"**.hpp",
	"**.cpp",
	"**.h",
	"**.c",
})

includedirs({
	"../ngin/include",
	".",
})

links({
	"Ngin",
})

shell("cmd")
postbuildcommands({
	"{COPY} ../src/ngin/shaders/*.hlsl %{cfg.targetdir}",
})

filter("system:windows")
links({
	"user32",
	"gdi32",
	"d3d12",
})
toolset("clang")

filter("system:windows")
systemversion("latest")

filter("configurations:DEBUG")
defines({ "NGIN_DEBUG", "NGIN_DEV_UI" })
runtime("Debug")
symbols("on")
filter("system:windows")
links({
	"dxgi",
	"dxguid",
})

filter("configurations:STAGING")
defines({ "NGIN_STAGING", "NGIN_DEV_UI" })
runtime("Release")
optimize("Speed")
symbols("on")
links({
	"dxgi",
	"dxguid",
})

filter("configurations:RELEASE")
defines({ "NGIN_RELEASE" })
runtime("Release")
optimize("Speed")
symbols("off")
