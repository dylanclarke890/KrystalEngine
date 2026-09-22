# NOTE: we only build what's needed for llvm's SmallVector type. We'll migrate away from this dependency in the future.
add_library(llvm STATIC)

set_target_properties(llvm
 PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(llvm
 PUBLIC
  cxx_std_23
)

target_compile_options(llvm
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_sources(llvm
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      ADT/ADL.h
      ADT/SmallVector.h
      Support/Compiler.h
      Support/MemAlloc.h
  PRIVATE
    ADT/SmallVector.cpp
    Support/MemAlloc.cpp
)