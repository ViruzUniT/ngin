project("Ngin")
kind("StaticLib")
language("C++")
cppdialect("C++23")
staticruntime("off")
pchheader("ngin/pch.h")
pchsource("src/pch.cpp")

targetdir("../../bin/" .. outputdir .. "/%{prj.name}")
objdir("../../bin-int/" .. outputdir .. "/%{prj.name}")

files({
	"**.hpp",
	"**.cpp",
	"**.h",
	"**.c",
	"shaders/**.hlsl",
})

includedirs({
	"./include",
	"./src",
	"../../vendor/imgui",
})

filter("toolset:gcc or toolset:clang")
warnings("Extra")
buildoptions({
	"-Wunused-variable",
	"-Wunused-parameter",
	"-Wunused-result",
	"-Wall",
	"-Wextra",
	"-Wunused-result",
})

filter("toolset:msc")
warnings("Extra")
buildoptions({
	"/w34100",
	"/w34101",
	"/w34834",
})

filter("system:windows")
links({
	"user32",
	"gdi32",
})
toolset("clang")

filter("configurations:DEBUG")
defines({ "NGIN_DEBUG", "NGIN_DEV_UI" })
runtime("Debug")
symbols("on")

filter("configurations:STAGING")
defines({ "NGIN_STAGING", "NGIN_DEV_UI" })
runtime("Release")
optimize("Speed")
symbols("on")

filter("configurations:RELEASE")
defines({ "NGIN_RELEASE" })
runtime("Release")
optimize("Speed")
symbols("off")
