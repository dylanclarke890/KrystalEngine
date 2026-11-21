add_library(Krystal.Core STATIC)

target_link_libraries(Krystal.Core 
  PUBLIC
    llvm

  PRIVATE
    RapidJSON rapidjson
    spdlog::spdlog
    # we don't link RapidXML as a library since it's header-only
)

set_target_properties(Krystal.Core
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
	  CXX_EXTENSIONS OFF
	  COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Core
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Core
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

# Third-party - RapidJSON
find_package(RapidJSON CONFIG REQUIRED)

# Third-party - spdlog
find_package(spdlog CONFIG REQUIRED)

# Third-party - RapidXML
find_path(RAPIDXML_INCLUDE_DIRS "rapidxml/rapidxml.hpp")
target_include_directories(Krystal.Core PRIVATE ${RAPIDXML_INCLUDE_DIRS})

# Third-party - rapidhash (header-only; public, since the hashers inline it)
find_path(RAPIDHASH_INCLUDE_DIRS "rapidhash.h" REQUIRED)
target_include_directories(Krystal.Core PUBLIC ${RAPIDHASH_INCLUDE_DIRS})

target_precompile_headers(Krystal.Core PRIVATE
  <chrono>
  <concepts>
  <format>
  <functional>
  <memory>
  <optional>
  <span>
  <string>
  <string_view>
  <tuple>
  <type_traits>
  <utility>
  <vector>
  "include/Krystal.Core/Base.hpp"
  "include/Krystal.Core/Memory/IntrusivePtr.hpp"
  "include/Krystal.Core/Maths/Matrix.hpp"
  "include/Krystal.Core/Maths/Quaternion.hpp"
  "include/Krystal.Core/Maths/Vector.hpp"
)

target_sources(Krystal.Core
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      # Top-level headers
      include/Krystal.Core/Assert.hpp
      include/Krystal.Core/Atomics.hpp
      include/Krystal.Core/Attributes.hpp
      include/Krystal.Core/Base.hpp
      include/Krystal.Core/Concepts.hpp
      include/Krystal.Core/Debug.hpp
      include/Krystal.Core/Endian.hpp
      include/Krystal.Core/Enum.hpp
      include/Krystal.Core/Hashing/FastHash.hpp
      include/Krystal.Core/Hashing/HashAlgorithm.hpp
      include/Krystal.Core/Hashing/StableHash.hpp
      include/Krystal.Core/Macros.hpp
      include/Krystal.Core/Move.hpp
      include/Krystal.Core/Numeric.hpp
      include/Krystal.Core/TriState.hpp
      include/Krystal.Core/TypeCast.hpp
      include/Krystal.Core/TypeTraits.hpp
      include/Krystal.Core/Visitor.hpp

      # Colour
      include/Krystal.Core/Colour/AlphaPremultiplication.hpp
      include/Krystal.Core/Colour/BlendMode.hpp
      include/Krystal.Core/Colour/ColourComponents.hpp
      include/Krystal.Core/Colour/ColourConversion.hpp
      include/Krystal.Core/Colour/ColourInterpolation.hpp
      include/Krystal.Core/Colour/ColourInterpolationMethod.hpp
      include/Krystal.Core/Colour/ColourLuminance.hpp
      include/Krystal.Core/Colour/ColourMatrix.hpp
      include/Krystal.Core/Colour/ColourModels.hpp
      include/Krystal.Core/Colour/ColourNormalization.hpp
      include/Krystal.Core/Colour/ColourSpace.hpp
      include/Krystal.Core/Colour/ColourTextStream.hpp
      include/Krystal.Core/Colour/ColourTransferFunctions.hpp
      include/Krystal.Core/Colour/ColourTypes.hpp
      include/Krystal.Core/Colour/ColourUtilities.hpp
      include/Krystal.Core/Colour/GradientSpreadMethod.hpp

      # Commands
      include/Krystal.Core/Commands/CommandArena.hpp
      include/Krystal.Core/Commands/CommandList.hpp
      include/Krystal.Core/Commands/CommandListReader.hpp
      include/Krystal.Core/Commands/CommandType.hpp

      # Detection
      include/Krystal.Core/Detection/AddressSpaceBitSize.hpp
      include/Krystal.Core/Detection/Compiler.hpp
      include/Krystal.Core/Detection/CPU.hpp
      include/Krystal.Core/Detection/OS.hpp

      # Events
      include/Krystal.Core/Events/Event.hpp
      include/Krystal.Core/Events/EventManager.hpp

      # Geometry
      include/Krystal.Core/Geometry/AffineTransform.hpp
      include/Krystal.Core/Geometry/GeometryForward.hpp
      include/Krystal.Core/Geometry/GeometryUtilities.hpp
      include/Krystal.Core/Geometry/Point.hpp
      include/Krystal.Core/Geometry/Point3.hpp
      include/Krystal.Core/Geometry/PointGraph.hpp
      include/Krystal.Core/Geometry/Polygon.hpp
      include/Krystal.Core/Geometry/Quad.hpp
      include/Krystal.Core/Geometry/Rect.hpp
      include/Krystal.Core/Geometry/RectHash.hpp
      include/Krystal.Core/Geometry/Region.hpp
      include/Krystal.Core/Geometry/Segment.hpp
      include/Krystal.Core/Geometry/Size.hpp
      include/Krystal.Core/Geometry/TransformationMatrix.hpp

      # IO
      include/Krystal.Core/IO/Backends/NativeFileBackend.hpp
      include/Krystal.Core/IO/Streams/MemoryStream.hpp
      include/Krystal.Core/IO/Streams/NativeFileStream.hpp
      include/Krystal.Core/IO/Streams/Stream.hpp
      include/Krystal.Core/IO/Streams/StreamUtils.hpp
      include/Krystal.Core/IO/Common.hpp
      include/Krystal.Core/IO/IFileBackend.hpp
      include/Krystal.Core/IO/Path.hpp
      include/Krystal.Core/IO/VirtualFileSystem.hpp
      
      # Log
      include/Krystal.Core/Log/ILogger.hpp

      # Maths
      include/Krystal.Core/Maths/Extras.hpp
      include/Krystal.Core/Maths/Clamp.hpp
      include/Krystal.Core/Maths/Clipspace.hpp
      include/Krystal.Core/Maths/Compare.hpp
      include/Krystal.Core/Maths/CompositeOperation.hpp
      include/Krystal.Core/Maths/Conventions.hpp
      include/Krystal.Core/Maths/Convert.hpp
      include/Krystal.Core/Maths/Easing.hpp
      include/Krystal.Core/Maths/EulerAngles.hpp
      include/Krystal.Core/Maths/Format.hpp
      include/Krystal.Core/Maths/Gauss.hpp
      include/Krystal.Core/Maths/Hash.hpp
      include/Krystal.Core/Maths/Interpolate.hpp
      include/Krystal.Core/Maths/Maths.hpp
      include/Krystal.Core/Maths/Matrix.hpp
      include/Krystal.Core/Maths/MortonCodes.hpp
      include/Krystal.Core/Maths/MovingAverage.hpp
      include/Krystal.Core/Maths/Packing.hpp
      include/Krystal.Core/Maths/Project.hpp
      include/Krystal.Core/Maths/Quaternion.hpp
      include/Krystal.Core/Maths/Random.hpp
      include/Krystal.Core/Maths/Round.hpp
      include/Krystal.Core/Maths/SpringSolver.hpp
      include/Krystal.Core/Maths/Swizzle.hpp
      include/Krystal.Core/Maths/Traits.hpp
      include/Krystal.Core/Maths/Transform.hpp
      include/Krystal.Core/Maths/UnitBezier.hpp
      include/Krystal.Core/Maths/Vector.hpp

      # Memory
      include/Krystal.Core/Memory/AlignedStorage.hpp
      include/Krystal.Core/Memory/AllowCompactPointers.hpp
      include/Krystal.Core/Memory/CanMakeCheckedPtr.hpp
      include/Krystal.Core/Memory/CanMakeWeakPtr.hpp
      include/Krystal.Core/Memory/CheckedPtr.hpp
      include/Krystal.Core/Memory/CompactPtrTuple.hpp
      include/Krystal.Core/Memory/CompactRefPtrTuple.hpp
      include/Krystal.Core/Memory/CompactUniquePtrTuple.hpp
      include/Krystal.Core/Memory/DataRef.hpp
      include/Krystal.Core/Memory/ForbidHeapAllocation.hpp
      include/Krystal.Core/Memory/InlineWeakPtr.hpp
      include/Krystal.Core/Memory/IntrusivePtr.hpp
      include/Krystal.Core/Memory/NeverDestroyed.hpp
      include/Krystal.Core/Memory/Packed.hpp
      include/Krystal.Core/Memory/PointerComparison.hpp
      include/Krystal.Core/Memory/RefCounted.hpp
      include/Krystal.Core/Memory/RefPtr.hpp
      include/Krystal.Core/Memory/TaggedPtr.hpp
      include/Krystal.Core/Memory/ThreadSafeWeakPtr.hpp
      include/Krystal.Core/Memory/UnalignedAccess.hpp
      include/Krystal.Core/Memory/UniqueArray.hpp
      include/Krystal.Core/Memory/UniquePtr.hpp
      include/Krystal.Core/Memory/WeakPtr.hpp
      include/Krystal.Core/Memory/WeakPtrFactory.hpp
      include/Krystal.Core/Memory/WeakPtrImpl.hpp

      # Serialisation
      include/Krystal.Core/Serialisation/Access.hpp
      include/Krystal.Core/Serialisation/Builtins.hpp
      include/Krystal.Core/Serialisation/Concepts.hpp
      include/Krystal.Core/Serialisation/Archives/Adapters/RapidJsonStreamAdapters.hpp
      include/Krystal.Core/Serialisation/Archives/BaseArchive.hpp
      include/Krystal.Core/Serialisation/Archives/BinaryArchive.hpp
      include/Krystal.Core/Serialisation/Archives/JsonArchive.hpp
      include/Krystal.Core/Serialisation/Archives/XmlArchive.hpp
      include/Krystal.Core/Serialisation/Types/Generic/KeyValuePairContainer.hpp
      include/Krystal.Core/Serialisation/Types/Array.hpp
      include/Krystal.Core/Serialisation/Types/HashMap.hpp
      include/Krystal.Core/Serialisation/Types/List.hpp
      include/Krystal.Core/Serialisation/Types/Pair.hpp
      include/Krystal.Core/Serialisation/Types/Quaternion.hpp
      include/Krystal.Core/Serialisation/Types/Vector.hpp

      # Strings
      include/Krystal.Core/Strings/ASCII.hpp
      include/Krystal.Core/Strings/CodePointIterator.hpp
      include/Krystal.Core/Strings/HexNumber.hpp
      include/Krystal.Core/Strings/ParsingUtilities.hpp
      include/Krystal.Core/Strings/String.hpp
      include/Krystal.Core/Strings/StringAtom.hpp
      include/Krystal.Core/Strings/StringBuilder.hpp
      include/Krystal.Core/Strings/StringToIntegerConversion.hpp
      include/Krystal.Core/Strings/TextStream.hpp
      include/Krystal.Core/Strings/Unicode.hpp

      # Threading
      include/Krystal.Core/Threading/CancellationToken.hpp
      include/Krystal.Core/Threading/JobSequence.hpp
      include/Krystal.Core/Threading/JobSystem.hpp
      include/Krystal.Core/Threading/MainThread.hpp
      include/Krystal.Core/Threading/ManualWorkQueue.hpp
      include/Krystal.Core/Threading/TaskInbox.hpp
      include/Krystal.Core/Threading/Thread.hpp
      include/Krystal.Core/Threading/WorkQueue.hpp

      # Time
      include/Krystal.Core/Time/Duration.hpp
      include/Krystal.Core/Time/MonotonicTime.hpp
      include/Krystal.Core/Time/ThreadTimers.hpp
      include/Krystal.Core/Time/Timer.hpp

      # Types
      include/Krystal.Core/Types/Any.hpp
      include/Krystal.Core/Types/Array.hpp
      include/Krystal.Core/Types/BoolOr.hpp
      include/Krystal.Core/Types/BloomFilter.hpp
      include/Krystal.Core/Types/CompactVariant.hpp
      include/Krystal.Core/Types/Decimal.hpp
      include/Krystal.Core/Types/Expected.hpp
      include/Krystal.Core/Containers/FixedList.hpp
      include/Krystal.Core/Types/Func.hpp
      include/Krystal.Core/Types/HashMap.hpp
      include/Krystal.Core/Types/HashSet.hpp
      include/Krystal.Core/Types/List.hpp
      include/Krystal.Core/Types/ListHashSet.hpp
      include/Krystal.Core/Types/RedBlackTree.hpp
      include/Krystal.Core/Types/IntervalTree.hpp
      include/Krystal.Core/Types/IterationStatus.hpp
      include/Krystal.Core/Types/Markable.hpp
      include/Krystal.Core/Types/Maybe.hpp
      include/Krystal.Core/Types/NTTPMaybe.hpp
      include/Krystal.Core/Types/Null.hpp
      include/Krystal.Core/Types/NTTPString.hpp
      include/Krystal.Core/Types/Pair.hpp
      include/Krystal.Core/Types/Queue.hpp
      include/Krystal.Core/Containers/RefCountedFixedList.hpp
      include/Krystal.Core/Types/SharedBuffer.hpp
      include/Krystal.Core/Types/SmallList.hpp
      include/Krystal.Core/Types/Span.hpp
      include/Krystal.Core/Types/Stack.hpp
      include/Krystal.Core/Types/TinyLRUCache.hpp
      include/Krystal.Core/Types/Variant.hpp

      # Utils
      include/Krystal.Core/Utils/Apply.hpp
      include/Krystal.Core/Utils/ByteUtils.hpp
      include/Krystal.Core/Utils/IsEOF.hpp
      include/Krystal.Core/Utils/NestingLevelIncrementer.hpp
      include/Krystal.Core/Utils/NativeHandle.hpp
      include/Krystal.Core/Utils/ScopeExit.hpp
      include/Krystal.Core/Utils/ScopedProfiler.hpp
      include/Krystal.Core/Utils/SetForScope.hpp
      include/Krystal.Core/Utils/SourceLocation.hpp
      include/Krystal.Core/Utils/StringId.hpp
      include/Krystal.Core/Utils/StrongOrder.hpp
      include/Krystal.Core/Utils/Tags.hpp

  PRIVATE
    # Colour
    src/Colour/ColourConversion.cpp
    src/Colour/ColourInterpolation.cpp
    src/Colour/ColourTextStream.cpp

    # Detection
    
    # Events
    src/Events/EventManager.cpp
    
    # Geometry
    src/Geometry/AffineTransform.cpp
    src/Geometry/GeometryUtilities.cpp
    src/Geometry/Point.cpp
    src/Geometry/Point3.cpp
    src/Geometry/PointGraph.cpp
    src/Geometry/Polygon.cpp
    src/Geometry/Quad.cpp
    src/Geometry/Rect.cpp
    src/Geometry/Region.cpp
    src/Geometry/Segment.cpp
    src/Geometry/Size.cpp
    src/Geometry/TransformationMatrix.cpp
    
    # IO
    src/IO/Backends/NativeFileBackend.cpp
    src/IO/Streams/NativeFileStream.cpp
    src/IO/Path.cpp
    src/IO/VirtualFileSystem.cpp
    
    # Log
    src/Log/SpdlogLogger.cpp
    
    # Maths
    src/Maths/Easing.cpp
    
    # Threading
    src/Threading/JobSequence.cpp
    src/Threading/JobSystem.cpp
    src/Threading/MainThread.cpp
    src/Threading/ManualWorkQueue.cpp
    src/Threading/TaskInbox.cpp
    src/Threading/Thread.cpp

    # Time
    src/Time/ThreadTimers.cpp
    src/Time/Timer.cpp

    # Types
    src/Types/Decimal.cpp
    src/Types/SharedBuffer.cpp

    # Utils
    src/Utils/ScopedProfiler.cpp

    # Strings
    src/Strings/StringBuilder.cpp
    src/Strings/TextStream.cpp
)
