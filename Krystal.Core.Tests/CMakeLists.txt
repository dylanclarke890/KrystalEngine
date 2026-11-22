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

    <catch2/catch_test_macros.hpp>
    <catch2/matchers/catch_matchers_floating_point.hpp>
)

target_sources(Krystal.Core.Tests
	PRIVATE tests/Memory/CheckedPtr.cpp
	PRIVATE tests/Memory/DataRef.cpp
	PRIVATE tests/Memory/NeverDestroyed.cpp
	PRIVATE tests/Memory/Packed.cpp
	PRIVATE tests/Memory/RefCounted.cpp
	PRIVATE tests/Memory/RefPtr.cpp
	PRIVATE tests/Memory/ThreadSafeWeakPtr.cpp
	PRIVATE tests/Memory/WeakPtr.cpp

  # Hashing
	PRIVATE tests/Hashing/FastHash.cpp

  # Colour
	PRIVATE tests/Colour/ColourConversion.cpp
	PRIVATE tests/Colour/ColourInterpolation.cpp
	PRIVATE tests/Colour/ColourUtilities.cpp

  # Strings
	PRIVATE tests/Strings/ASCII.cpp
	PRIVATE tests/Strings/CodePointIterator.cpp
	PRIVATE tests/Strings/StringAtom.cpp
	PRIVATE tests/Strings/StringBuilder.cpp
	PRIVATE tests/Strings/StringToIntegerConversion.cpp
	PRIVATE tests/Strings/ParsingUtilities.cpp
	PRIVATE tests/Strings/TextStream.cpp
	PRIVATE tests/Strings/Unicode.cpp

  # Types
	PRIVATE tests/Containers/BloomFilter.cpp
	PRIVATE tests/Types/CompactVariant.cpp
	PRIVATE tests/Types/Decimal.cpp
	PRIVATE tests/Containers/FixedList.cpp
	PRIVATE tests/Containers/IntervalTree.cpp
	PRIVATE tests/Containers/ListHashSet.cpp
	PRIVATE tests/Types/SharedBuffer.cpp
	PRIVATE tests/Types/TypeList.cpp

  # Threading
	PRIVATE tests/Threading/CancellationToken.cpp
	PRIVATE tests/Threading/JobSequence.cpp
	PRIVATE tests/Threading/JobSystem.cpp
	PRIVATE tests/Threading/MainThread.cpp
	PRIVATE tests/Threading/ManualWorkQueue.cpp
	PRIVATE tests/Threading/TaskInbox.cpp
	PRIVATE tests/Threading/Thread.cpp

  # Time
	PRIVATE tests/Time/Timer.cpp

  # Commands
	PRIVATE tests/Commands/Command.cpp
	
  # Geometry
	PRIVATE tests/Geometry/AffineTransform.cpp
	PRIVATE tests/Geometry/GeometryUtilities.cpp
	PRIVATE tests/Geometry/Point.cpp
	PRIVATE tests/Geometry/Point3.cpp
	PRIVATE tests/Geometry/PointGraph.cpp
	PRIVATE tests/Geometry/Polygon.cpp
	PRIVATE tests/Geometry/Quad.cpp
	PRIVATE tests/Geometry/Rect.cpp
	PRIVATE tests/Geometry/Region.cpp
	PRIVATE tests/Geometry/Segment.cpp
	PRIVATE tests/Geometry/Size.cpp
	PRIVATE tests/Geometry/TransformationMatrix.cpp

  # IO
  PRIVATE tests/IO/Backends/NativeFileBackend.cpp
	PRIVATE tests/IO/Streams/NativeFileStream.cpp
	PRIVATE tests/IO/Streams/MemoryStream.cpp
	PRIVATE tests/IO/Path.cpp
	PRIVATE tests/IO/VirtualFileSystem.cpp

  # Maths
	PRIVATE tests/Maths/Clamp.cpp
	PRIVATE tests/Maths/Clipspace.cpp
	PRIVATE tests/Maths/Compare.cpp
	PRIVATE tests/Maths/Convert.cpp
	PRIVATE tests/Maths/Easing.cpp
	PRIVATE tests/Maths/Interpolate.cpp
	PRIVATE tests/Maths/Maths.cpp
	PRIVATE tests/Maths/Matrix.cpp
	PRIVATE tests/Maths/MortonCodes.cpp
	PRIVATE tests/Maths/Packing.cpp
	PRIVATE tests/Maths/Project.cpp
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
