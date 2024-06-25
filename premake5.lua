-- premake5.lua
workspace "pbr"
    architecture "x64"
    configurations
    { 
        "Debug",
        "Release"
    }
    startproject "pbr"

    flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
VULKAN_SDK = os.getenv("VULKAN_SDK")

group "Dependencies"
	include "vendor/glfw"
	include "vendor/imgui"
group ""

project "pbr"
    kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("bin/" .. outputdir)
	objdir ("bin/" .. outputdir .. "/temp")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.c",
		"src/nvulkan/**.h",
		"src/nvulkan/**.c",
		"src/gui/**.h",
		"src/gui/**.cpp",
		"src/models/**.h",
		"src/models/**.c",
		"src/models/**.cpp",
		"src/math/**.h",
		"src/math/**.c",
		"src/hl/**.h",
		"src/hl/**.c",
		"src/base/**.h",
		"src/base/**.c",
	}

	filter { "system:windows" }
    files { "src/base/win/*.c" }

  includedirs
  {
    "vendor/glfw/include",
    "vendor/imgui",
    "%{VULKAN_SDK}/Include",
	"src",
	"vendor/sh_libs"
  }

  links {
		"GLFW",
		"ImGui"
  }
	
	defines {
		GLFW_INCLUDE_NONE
	}

	filter { "system:windows" }
		links { "vulkan-1.lib" }
		libdirs { "%{VULKAN_SDK}/Lib"}
		systemversion "latest"

    filter { "system:macosx" }
      defines { "GL_SILENCE_DEPRECATION" }
      linkoptions { "-framework OpenGL -framework Cocoa -framework IOKit" }
   
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		defines { "BUILD_DEBUG" }

	filter "configurations:Release"
		runtime "Release"
    optimize "on"
