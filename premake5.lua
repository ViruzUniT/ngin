workspace("Ngin")
architecture("x64")
configurations({ "DEBUG", "STAGING", "RELEASE" })
location("build")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include("src/ngin")
include("src/sandbox")
