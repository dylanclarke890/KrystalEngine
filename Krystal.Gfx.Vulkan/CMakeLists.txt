add_library(Krystal.Gfx.Vulkan)

target_link_libraries(Krystal.Gfx.Vulkan
	PUBLIC 
    Krystal.Core
	  Krystal.Gfx
    Vulkan::Headers
)

set_target_properties(Krystal.Gfx.Vulkan
 PROPERTIES
  CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Gfx.Vulkan
 PUBLIC
  cxx_std_23
)

target_compile_options(Krystal.Gfx.Vulkan
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

find_package(VulkanHeaders CONFIG REQUIRED)

target_sources(Krystal.Gfx.Vulkan
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Gfx.Vulkan/VulkanContext.hpp
      include/Krystal.Gfx.Vulkan/Hooks/vulkan_hooks.hpp
      include/Krystal.Gfx.Vulkan/Hooks/vulkan_win32_hooks.hpp

	PRIVATE 
    src/VulkanContext.cpp
    src/Hooks/vulkan_hooks.cpp
    src/Hooks/vulkan_win32_hooks.cpp
)
