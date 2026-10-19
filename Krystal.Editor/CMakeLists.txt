add_executable(Krystal.Editor)

target_link_libraries(Krystal.Editor
	PUBLIC Krystal.Core
	PUBLIC Krystal.Gfx
	PUBLIC Krystal.PAL
	PUBLIC Krystal.UI
)

set_target_properties(Krystal.Editor
 PROPERTIES
  CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Editor
 PUBLIC
  cxx_std_23
)

target_compile_options(Krystal.Editor
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8 /bigobj>
)

# The editor draws its labels in Liberation Sans, from the repository.
target_compile_definitions(Krystal.Editor
  PRIVATE
    KRYS_EDITOR_FONTS_DIR="${CMAKE_SOURCE_DIR}/third-party/liberation-fonts"
)

target_sources(Krystal.Editor
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			include
		FILES
			include/Krystal.Editor/Application.hpp
			include/Krystal.Editor/BooeyScene.hpp
			include/Krystal.Editor/Editor.hpp
		
	PRIVATE
		src/Application.cpp
		src/Entry.cpp
)

