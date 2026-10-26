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

filter("system:windows")
local shaderProfiles = {
	vs = "vs_6_0",
	vertex = "vs_6_0",
	ps = "ps_6_0",
	pixel = "ps_6_0",
	gs = "gs_6_0",
	geometry = "gs_6_0",
	hs = "hs_6_0",
	hull = "hs_6_0",
	ds = "ds_6_0",
	domain = "ds_6_0",
	cs = "cs_6_0",
	compute = "cs_6_0",
	rootSignature = "rootsig_1_1",
}

local shaderCommands = { "{MKDIR} %{cfg.targetdir}" }
local dxc = path.getabsolute("../../.tools/dxc/1.9.2609/bin/x64/dxc.exe")
for _, shader in ipairs(os.matchfiles("../ngin/shaders/*.hlsl")) do
	local filename = path.getbasename(shader)
	local stage = path.getbasename(shader):match("%.([^.]+)$") or filename
	local profile = filename:lower() == "rootsignature" and "rootsig_1_1" or shaderProfiles[stage:lower()]
	if profile then
		local output = path.join("%{cfg.targetdir}", filename .. ".dxil")
		local entryPoint = profile == "rootsig_1_1" and "-E ROOTSIG " or "-E main "
		table.insert(
			shaderCommands,
			string.format('"%s" -T %s %s-Fo "%s" "../src/ngin/shaders/%s.hlsl"', dxc, profile, entryPoint, output, filename)
		)
	end
end

prebuildcommands(shaderCommands)

links({
	"user32",
	"gdi32",
	"d3d12",
	"dxgi",
	"dxguid",
	"d3dcompiler",
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
