add_executable(Krystal.Core.Tests)

target_link_libraries(Krystal.Core.Tests
  PRIVATE
	 Krystal.Core
   Catch2::Catch2WithMain
)

set_target_properties(Krystal.Core.Tests
 PROPERTIES
  CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Core.Tests
 PUBLIC
  cxx_std_23
)

target_compile_options(Krystal.Core.Tests
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_compile_definitions(Krystal.Core.Tests PRIVATE
    KRYS_CORE_TESTS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

catch_discover_tests(Krystal.Core.Tests)

target_precompile_headers(Krystal.Core.Tests
  PRIVATE
    <algorithm>
    <chrono>
    <concepts>
    <format>
    <functional>
    <memory>
    <optional>
    <span>
    <ranges>
    <string>
    <string_view>
    <tuple>
    <type_traits>
    <variant>
    <vector>

    catch2/catch_test_macros.hpp
    catch2/matchers/catch_matchers_floating_point.hpp
)

target_sources(Krystal.Core.Tests
	PRIVATE tests/RefCounted.cpp

  # Commands
	PRIVATE tests/Commands/Command.cpp
	
  # IO
  PRIVATE tests/IO/Backends/NativeFileBackend.cpp
	PRIVATE tests/IO/Streams/NativeFileStream.cpp
	PRIVATE tests/IO/Streams/MemoryStream.cpp
	PRIVATE tests/IO/ImageLoader.cpp
	PRIVATE tests/IO/Path.cpp
	PRIVATE tests/IO/VirtualFileSystem.cpp

  # Maths
	PRIVATE tests/Maths/Clamp.cpp
	PRIVATE tests/Maths/Clipspace.cpp
	PRIVATE tests/Maths/Compare.cpp
	PRIVATE tests/Maths/Convert.cpp
	PRIVATE tests/Maths/Interpolate.cpp
	PRIVATE tests/Maths/Maths.cpp
	PRIVATE tests/Maths/Matrix.cpp
	PRIVATE tests/Maths/MortonCodes.cpp
	PRIVATE tests/Maths/Packing.cpp
	PRIVATE tests/Maths/Quaternion.cpp
	PRIVATE tests/Maths/Round.cpp
	PRIVATE tests/Maths/Swizzle.cpp
	PRIVATE tests/Maths/Traits.cpp
	PRIVATE tests/Maths/Transform.cpp
	PRIVATE tests/Maths/Vector.cpp

  # Serialisation
	PRIVATE tests/Serialisation/Types/Array.cpp
	PRIVATE tests/Serialisation/Types/HashMap.cpp
	PRIVATE tests/Serialisation/Types/List.cpp
	PRIVATE tests/Serialisation/Types/Pair.cpp
	PRIVATE tests/Serialisation/Types/Quaternion.cpp
	PRIVATE tests/Serialisation/Types/Vector.cpp

	PRIVATE tests/Serialisation/Access.cpp
	PRIVATE tests/Serialisation/Builtins.cpp
	PRIVATE tests/Serialisation/Versioning.cpp
)
