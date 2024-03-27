workspace "XephNet"
	architecture "x64"
	configurations
	{
		"Debug",
		"Release"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "XephNet"
	location "%{prj.name}"
	kind "StaticLib"
	language "C++"
	targetname "%{prj.name}"
	targetdir ("bin/".. outputdir)
	objdir ("%{prj.name}/int/".. outputdir)
	cppdialect "C++17"
	staticruntime "Off"
	
	defines { "SFML_STATIC" }

	files
	{
		"%{prj.name}/**.h",
		"%{prj.name}/**.cpp"
	}
	
	includedirs
	{
		"%{prj.name}/include",
		"%{prj.name}/src"
	}
	
	filter "system:windows"
		systemversion "latest"
		defines { "WIN32" }
		
	filter "system:linux"
		systemversion "latest"
		defines { "LINUX" }

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"
		
	filter "configurations:Release"
		defines { "NDEBUG", "NCONSOLE" }
		optimize "On"
		
project "TestServer"
	location "%{prj.name}"
	kind "ConsoleApp"
	language "C++"
	targetname "%{prj.name}"
	targetdir ("bin/".. outputdir)
	objdir ("%{prj.name}/int/".. outputdir)
	cppdialect "C++17"
	staticruntime "Off"
	
	defines { "SFML_STATIC" }

	files
	{
		"%{prj.name}/**.h",
		"%{prj.name}/**.cpp"
	}
	
	includedirs
	{
		"XephNet/include"
	}
	
	links
	{
		"XephNet"
	}
	
	filter "system:windows"
		systemversion "latest"
		libdirs "XephNet/lib/windows"
		links
		{
		    "ws2_32",
		    "winmm"
		}
		
	filter "system:linux"
		systemversion "latest"
		libdirs "XephNet/lib/linux"
		
	filter "system:macos"
		systemversion "latest"
		libdirs "XephNet/lib/macos"

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"
		links
		{
			"sfml-system-s-d",
			"sfml-network-s-d",
			"sfml-window-s-d",
			"sfml-graphics-s-d",
		}
		
	filter "configurations:Release"
		defines { "NDEBUG", "NCONSOLE" }
		optimize "On"
		links
		{
			"sfml-system-s",
			"sfml-network-s",
			"sfml-window-s",
			"sfml-graphics-s"
		}
		
project "TestClient"
	location "%{prj.name}"
	kind "ConsoleApp"
	language "C++"
	targetname "%{prj.name}"
	targetdir ("bin/".. outputdir)
	objdir ("%{prj.name}/int/".. outputdir)
	cppdialect "C++17"
	staticruntime "Off"
	
	defines { "SFML_STATIC" }

	files
	{
		"%{prj.name}/**.h",
		"%{prj.name}/**.cpp"
	}
	
	includedirs
	{
		"XephNet/include"
	}
	
	links
	{
		"XephNet"
	}
	
	filter "system:windows"
		systemversion "latest"
		libdirs "XephNet/lib/windows"
		links
		{
		    "ws2_32",
		    "winmm"
		}
		
	filter "system:linux"
		systemversion "latest"
		libdirs "XephNet/lib/linux"
		
	filter "system:macos"
		systemversion "latest"
		libdirs "XephNet/lib/macos"

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"
		links
		{
			"sfml-system-s-d",
			"sfml-network-s-d",
			"sfml-window-s-d",
			"sfml-graphics-s-d",
		}
		
	filter "configurations:Release"
		defines { "NDEBUG", "NCONSOLE" }
		optimize "On"
		links
		{
			"sfml-system-s",
			"sfml-network-s",
			"sfml-window-s",
			"sfml-graphics-s"
		}