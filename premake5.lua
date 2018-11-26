workspace "RSE"
	configurations
	{ 
		"Debug", 
		"Release" 
	}

	platforms
	{ 
		"Win32", 
		"x64" 
	}
	
	filter "platforms:x86"
		system "Windows"
		architecture "x32"
	
	filter "platforms:x64"
		system "Windows"
		architecture "x64"

outputdir = "%{cfg.buildcfg}-%{cfg.platform}"

project "Engine"
	location "Engine"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/%{prj.name}/" .. outputdir)
	objdir ("bin/intermediate/%{prj.name}/" .. outputdir)

	files
	{
		"%{prj.name}/Engine/**.h",
		"%{prj.name}/Engine/**.hpp",
		"%{prj.name}/Engine/**.cpp"
	}

	includedirs
	{
		"Thirdparty/Glew/include",
		"Thirdparty/SDL2/include",
		"Thirdparty/SDL2_image/include",
		"Thirdparty/SDL2_mixer/include",
		"Thirdparty/SDL2_ttf/include",
		"Thirdparty/glm/include",
		"Thirdparty/imgui/include",
		"Thirdparty/Box2D/include",
		"Thirdparty/cereal",
		"Thirdparty/rapidxml",
		"Thirdparty/entt"
	}
	
	libdirs
	{
		"Thirdparty/Glew/lib/%{cfg.platform}",
		"Thirdparty/SDL2/lib/%{cfg.platform}",
		"Thirdparty/SDL2_image/lib/%{cfg.platform}",
		"Thirdparty/SDL2_mixer/lib/%{cfg.platform}",
		"Thirdparty/SDL2_ttf/lib/%{cfg.platform}",
		"Thirdparty/Box2D/lib/%{cfg.platform}"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

	filter "configurations:Debug"
		defines "_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"
	
	filter "platforms:x86"
		defines "WIN32"

project "Editor"
	location "Editor"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/%{prj.name}/" .. outputdir)
	objdir ("bin/intermediate/%{prj.name}/" .. outputdir)

	files
	{
		"%{prj.name}/Source/**.h",
		"%{prj.name}/Source/**.hpp",
		"%{prj.name}/Source/**.cpp"
	}

	includedirs
	{
		"Thirdparty/Glew/include",
		"Thirdparty/SDL2/include",
		"Thirdparty/SDL2_image/include",
		"Thirdparty/SDL2_mixer/include",
		"Thirdparty/SDL2_ttf/include",
		"Thirdparty/glm/include",
		"Thirdparty/imgui/include",
		"Thirdparty/Box2D/include",
		"Thirdparty/cereal",
		"Thirdparty/rapidxml",
		"Thirdparty/entt"
	}
	
	libdirs
	{
		"Thirdparty/Glew/lib/%{cfg.architecture}",
		"Thirdparty/SDL2/lib/%{cfg.architecture}",
		"Thirdparty/SDL2_image/lib/%{cfg.architecture}",
		"Thirdparty/SDL2_mixer/lib/%{cfg.architecture}",
		"Thirdparty/SDL2_ttf/lib/%{cfg.architecture}",
		"Thirdparty/Box2D/lib/%{cfg.architecture}",
		("bin/Engine/" .. outputdir)
	}

	links
	{
		"Engine"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

	postbuildcommands
	{
		("{COPY} Thirdparty/Glew/bin/%{cfg.platform}/*.dll bin/%{prj.name}/" .. outputdir),
		("{COPY} Thirdparty/SDL2/bin/%{cfg.platform}/*.dll bin/%{prj.name}/" .. outputdir),
		("{COPY} Thirdparty/SDL2_image/bin/%{cfg.platform}/*.dll bin/%{prj.name}/" .. outputdir),
		("{COPY} Thirdparty/SDL2_mixer/bin/%{cfg.platform}/*.dll bin/%{prj.name}/" .. outputdir),
		("{COPY} Thirdparty/SDL2_ttf/bin/%{cfg.platform}/*.dll bin/%{prj.name}/" .. outputdir)
	}

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"

	filter "platforms:x86"
		defines "WIN32"