add_library(Krystal.Core STATIC)

target_link_libraries(Krystal.Core 
  PUBLIC
    llvm

  PRIVATE
    stb
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
  "include/Krystal.Core/Types/IntrusivePtr.hpp"
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
      include/Krystal.Core/CanMakeCheckedPtr.hpp
      include/Krystal.Core/CanMakeWeakPtr.hpp
      include/Krystal.Core/Concepts.hpp
      include/Krystal.Core/Debug.hpp
      include/Krystal.Core/Endian.hpp
      include/Krystal.Core/Enum.hpp
      include/Krystal.Core/Hash.hpp
      include/Krystal.Core/Macros.hpp
      include/Krystal.Core/MagicEnum.hpp
      include/Krystal.Core/Numeric.hpp
      include/Krystal.Core/RefCounted.hpp
      include/Krystal.Core/TypeCast.hpp
      include/Krystal.Core/TypeTraits.hpp
      include/Krystal.Core/Visitor.hpp

      # Commands
      include/Krystal.Core/Commands/CommandArena.hpp
      include/Krystal.Core/Commands/CommandList.hpp
      include/Krystal.Core/Commands/CommandListReader.hpp
      include/Krystal.Core/Commands/CommandType.hpp

      # Detection
      include/Krystal.Core/Detection/AddressSpaceBitSize.hpp
      include/Krystal.Core/Detection/Compiler.hpp
      include/Krystal.Core/Detection/CPU.hpp
      include/Krystal.Core/Detection/NumberOfCores.hpp
      include/Krystal.Core/Detection/OS.hpp

      # Events
      include/Krystal.Core/Events/Event.hpp
      include/Krystal.Core/Events/EventManager.hpp

      # IO
      include/Krystal.Core/IO/Backends/NativeFileBackend.hpp
      include/Krystal.Core/IO/Streams/MemoryStream.hpp
      include/Krystal.Core/IO/Streams/NativeFileStream.hpp
      include/Krystal.Core/IO/Streams/Stream.hpp
      include/Krystal.Core/IO/Streams/StreamUtils.hpp
      include/Krystal.Core/IO/Common.hpp
      include/Krystal.Core/IO/IFileBackend.hpp
      include/Krystal.Core/IO/Image.hpp
      include/Krystal.Core/IO/ImageLoader.hpp
      include/Krystal.Core/IO/Path.hpp
      include/Krystal.Core/IO/VirtualFileSystem.hpp
      
      # Log
      include/Krystal.Core/Log/ILogger.hpp

      # Maths
      include/Krystal.Core/Maths/Base.hpp
      include/Krystal.Core/Maths/Clamp.hpp
      include/Krystal.Core/Maths/Clipspace.hpp
      include/Krystal.Core/Maths/Compare.hpp
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
      include/Krystal.Core/Maths/Swizzle.hpp
      include/Krystal.Core/Maths/Traits.hpp
      include/Krystal.Core/Maths/Transform.hpp
      include/Krystal.Core/Maths/Vector.hpp

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

      # Text
      include/Krystal.Core/Text/Encodings/All.hpp
      include/Krystal.Core/Text/Encodings/ASCII.hpp
      include/Krystal.Core/Text/Encodings/Atari.hpp
      include/Krystal.Core/Text/Encodings/Base.hpp
      include/Krystal.Core/Text/Encodings/Big5-HKSCS.hpp
      include/Krystal.Core/Text/Encodings/Decode.hpp
      include/Krystal.Core/Text/Encodings/Encode.hpp
      include/Krystal.Core/Text/Encodings/EncodingId.hpp
      include/Krystal.Core/Text/Encodings/EUC-KR-UHC.hpp
      include/Krystal.Core/Text/Encodings/GB18030.hpp
      include/Krystal.Core/Text/Encodings/IBM.hpp
      include/Krystal.Core/Text/Encodings/ISO-8859.hpp
      include/Krystal.Core/Text/Encodings/Kamenicky.hpp
      include/Krystal.Core/Text/Encodings/KOI8.hpp
      include/Krystal.Core/Text/Encodings/KZ-1048.hpp
      include/Krystal.Core/Text/Encodings/Macintosh.hpp
      include/Krystal.Core/Text/Encodings/PETSCII.hpp
      include/Krystal.Core/Text/Encodings/ShiftJIS.hpp
      include/Krystal.Core/Text/Encodings/Tatar.hpp
      include/Krystal.Core/Text/Encodings/UTF.hpp
      include/Krystal.Core/Text/Encodings/Windows.hpp
      include/Krystal.Core/Text/ASCII.hpp
      include/Krystal.Core/Text/EncodingRegistry.hpp
      include/Krystal.Core/Text/ICodec.hpp
      include/Krystal.Core/Text/Unicode.hpp

      # Time
      include/Krystal.Core/Time/Duration.hpp
      include/Krystal.Core/Time/MonotonicTime.hpp

      # Types
      include/Krystal.Core/Types/Any.hpp
      include/Krystal.Core/Types/Array.hpp
      include/Krystal.Core/Types/BoolOr.hpp
      include/Krystal.Core/Types/CheckedPtr.hpp
      include/Krystal.Core/Types/CompactPtrTuple.hpp
      include/Krystal.Core/Types/CompactRefPtrTuple.hpp
      include/Krystal.Core/Types/CompactUniquePtrTuple.hpp
      include/Krystal.Core/Types/Expected.hpp
      include/Krystal.Core/Types/Func.hpp
      include/Krystal.Core/Types/HashMap.hpp
      include/Krystal.Core/Types/HashSet.hpp
      include/Krystal.Core/Types/InlineWeakPtr.hpp
      include/Krystal.Core/Types/IntrusivePtr.hpp
      include/Krystal.Core/Types/List.hpp
      include/Krystal.Core/Types/Markable.hpp
      include/Krystal.Core/Types/Maybe.hpp
      include/Krystal.Core/Types/NTTPMaybe.hpp
      include/Krystal.Core/Types/NTTPString.hpp
      include/Krystal.Core/Types/Packed.hpp
      include/Krystal.Core/Types/Pair.hpp
      include/Krystal.Core/Types/Queue.hpp
      include/Krystal.Core/Types/RefPtr.hpp
      include/Krystal.Core/Types/SmallList.hpp
      include/Krystal.Core/Types/Span.hpp
      include/Krystal.Core/Types/Stack.hpp
      include/Krystal.Core/Types/String.hpp
      include/Krystal.Core/Types/StringAtom.hpp
      include/Krystal.Core/Types/StronglyTypedValue.hpp
      include/Krystal.Core/Types/TaggedPtr.hpp
      include/Krystal.Core/Types/ThreadSafeWeakPtr.hpp
      include/Krystal.Core/Types/UniqueArray.hpp
      include/Krystal.Core/Types/UniquePtr.hpp
      include/Krystal.Core/Types/Variant.hpp
      include/Krystal.Core/Types/WeakPtr.hpp
      include/Krystal.Core/Types/WeakPtrFactory.hpp
      include/Krystal.Core/Types/WeakPtrImpl.hpp

      # Utils
      include/Krystal.Core/Utils/AlignedStorage.hpp
      include/Krystal.Core/Utils/AllowCompactPointers.hpp
      include/Krystal.Core/Utils/Apply.hpp
      include/Krystal.Core/Utils/ByteUtils.hpp
      include/Krystal.Core/Utils/ForbidHeapAllocation.hpp
      include/Krystal.Core/Utils/GetPtrHelper.hpp
      include/Krystal.Core/Utils/Handle.hpp
      include/Krystal.Core/Utils/IndexedRange.hpp
      include/Krystal.Core/Utils/Move.hpp
      include/Krystal.Core/Utils/NativeHandle.hpp
      include/Krystal.Core/Utils/NeverDestroyed.hpp
      include/Krystal.Core/Utils/ScopedProfiler.hpp
      include/Krystal.Core/Utils/SetForScope.hpp
      include/Krystal.Core/Utils/StringId.hpp
      include/Krystal.Core/Utils/StrongOrder.hpp
      include/Krystal.Core/Utils/Tags.hpp
      include/Krystal.Core/Utils/UnalignedAccess.hpp
      include/Krystal.Core/Utils/ZippedRange.hpp

  PRIVATE
    # Detection
    src/Detection/NumberOfCores.cpp
    
    # Events
    src/Events/EventManager.cpp
    
    # IO
    src/IO/Backends/NativeFileBackend.cpp
    src/IO/Streams/NativeFileStream.cpp
    src/IO/ImageLoader.cpp
    src/IO/Path.cpp
    src/IO/VirtualFileSystem.cpp
    
    # Log
    src/Log/SpdlogLogger.cpp
    
    # Utils
    src/Utils/ScopedProfiler.cpp
)
