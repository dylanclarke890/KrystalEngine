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
      include/Krystal.PAL/Accessibility.hpp
      include/Krystal.PAL/CursorShapes.hpp
      include/Krystal.PAL/Display.hpp
      include/Krystal.PAL/DragAndDrop.hpp
      include/Krystal.PAL/Events.hpp
      include/Krystal.PAL/IWindow.hpp
      include/Krystal.PAL/Platform.hpp
      include/Krystal.PAL/Input/Gamepad.hpp
      include/Krystal.PAL/Input/Input.hpp
      include/Krystal.PAL/Input/Keyboard.hpp
      include/Krystal.PAL/Input/KeyModifiers.hpp
      include/Krystal.PAL/Input/Keys.hpp
      include/Krystal.PAL/Input/Mouse.hpp
      include/Krystal.PAL/Input/MouseButtons.hpp
      include/Krystal.PAL/Input/Pointer.hpp
      include/Krystal.PAL/Input/TextInput.hpp
      include/Krystal.PAL/Input/TouchpadPan.hpp

	PRIVATE 
    src/Events.cpp
    src/Input/Input.cpp
    src/Input/Keyboard.cpp
    src/Input/Mouse.cpp
)

if(WIN32)
  target_sources(Krystal.PAL
    PRIVATE
      include/Krystal.PAL/Win32/DirectManipulationPan.hpp
      include/Krystal.PAL/Win32/DisplayColour.hpp
      include/Krystal.PAL/Win32/DropTarget.hpp
      include/Krystal.PAL/Win32/UIAutomation.hpp
      include/Krystal.PAL/Win32/Utils.hpp
      include/Krystal.PAL/Win32/Win32Window.hpp
      src/Win32/DirectManipulationPan.cpp
      src/Win32/DisplayColour.cpp
      src/Win32/DropTarget.cpp
      src/Win32/Platform.cpp
      src/Win32/UIAutomation.cpp
      src/Win32/Utils.cpp
      src/Win32/Win32Window.cpp
      src/Win32/XInputGamepadBackend.cpp
  )

  target_compile_definitions(Krystal.PAL 
    PUBLIC
      _UNICODE
      UNICODE
  )
endif()

