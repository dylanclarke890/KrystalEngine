add_library(Krystal.PAL)

target_link_libraries(Krystal.PAL
	PUBLIC Krystal.Core
)

set_target_properties(Krystal.PAL
  PROPERTIES
  CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.PAL
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.PAL
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_sources(Krystal.PAL
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.PAL/Events.hpp
      include/Krystal.PAL/Input.hpp
      include/Krystal.PAL/IWindow.hpp
      include/Krystal.PAL/Keyboard.hpp
      include/Krystal.PAL/Keys.hpp
      include/Krystal.PAL/Mouse.hpp
      include/Krystal.PAL/MouseButtons.hpp
      include/Krystal.PAL/Platform.hpp

	PRIVATE 
    src/Events.cpp
    src/Input.cpp
    src/Keyboard.cpp
    src/Mouse.cpp
)

if(WIN32)
  target_sources(Krystal.PAL
    PRIVATE
      include/Krystal.PAL/Win32/Utils.hpp
      include/Krystal.PAL/Win32/Win32Window.hpp
      src/Win32/Platform.cpp
      src/Win32/Utils.cpp
      src/Win32/Win32Window.cpp
  )

  target_compile_definitions(Krystal.PAL 
    PUBLIC
      _UNICODE
      UNICODE
  )
endif()

