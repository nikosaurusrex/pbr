-- premake5.lua
workspace "raytracer"
    architecture "x64"
    configurations
    { 
        "Debug",
        "Release"
    }
    startproject "raytracer"

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

project "raytracer"
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
		"src/nvulkan/**.h",
		"src/nvulkan/**.c",
	}

  includedirs
  {
    "vendor/glfw/include",
    "vendor/imgui",
    "%{VULKAN_SDK}/Include",
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

	filter "configurations:Release"
		runtime "Release"
    optimize "on"
