project("Ngin")
kind("StaticLib")
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
	"./include",
	"./src",
	"../../vendor/imgui",
})

filter("system:windows")
links({
	"user32",
	"gdi32",
})

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
