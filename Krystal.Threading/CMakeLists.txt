add_library(Krystal.Threading STATIC)

target_link_libraries(Krystal.Threading
  PUBLIC
    Krystal.Core
)

set_target_properties(Krystal.Threading
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Threading
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Threading
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_sources(Krystal.Threading
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Threading/CancellationToken.hpp
      include/Krystal.Threading/JobSystem.hpp
      include/Krystal.Threading/MainThread.hpp
      include/Krystal.Threading/TaskTarget.hpp
      include/Krystal.Threading/Thread.hpp

  PRIVATE
    src/JobSystem.cpp
    src/MainThread.cpp
    src/TaskTarget.cpp
    src/Thread.cpp
)
