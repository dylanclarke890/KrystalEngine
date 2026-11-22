add_library(Krystal.Image STATIC)

target_link_libraries(Krystal.Image
  PUBLIC
    Krystal.Core
)

set_target_properties(Krystal.Image
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Image
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Image
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

# Third-party - PNG, JPEG and WebP, decoded and encoded (GIF, BMP and ICO are decoded in-tree)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(WebP CONFIG REQUIRED)
target_link_libraries(Krystal.Image PRIVATE PNG::PNG JPEG::JPEG WebP::webp WebP::webpdemux)

# Third-party - colour management (ICC profiles in images)
find_package(lcms2 CONFIG REQUIRED)
target_link_libraries(Krystal.Image PRIVATE lcms2::lcms2)

target_sources(Krystal.Image
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Image/Decoders/BMPImageDecoder.hpp
      include/Krystal.Image/Decoders/BMPImageReader.hpp
      include/Krystal.Image/Decoders/GIFImageDecoder.hpp
      include/Krystal.Image/Decoders/ICOImageDecoder.hpp
      include/Krystal.Image/Decoders/JPEGImageDecoder.hpp
      include/Krystal.Image/Decoders/PNGImageDecoder.hpp
      include/Krystal.Image/Decoders/WEBPImageDecoder.hpp
      include/Krystal.Image/EXIFMetadata.hpp
      include/Krystal.Image/ICCProfileTransform.hpp
      include/Krystal.Image/ImageBackingStore.hpp
      include/Krystal.Image/ImageDecoder.hpp
      include/Krystal.Image/ImageDecoderFrame.hpp
      include/Krystal.Image/ImageEncoder.hpp
      include/Krystal.Image/ImageOrientation.hpp
      include/Krystal.Image/ImageTypes.hpp
      include/Krystal.Image/PixelData.hpp

  PRIVATE
    src/Decoders/BMPImageDecoder.cpp
    src/Decoders/BMPImageReader.cpp
    src/Decoders/GIFImageDecoder.cpp
    src/Decoders/ICOImageDecoder.cpp
    src/Decoders/JPEGImageDecoder.cpp
    src/Decoders/PNGImageDecoder.cpp
    src/Decoders/WEBPImageDecoder.cpp
    src/EXIFMetadata.cpp
    src/ICCProfileTransform.cpp
    src/ImageDecoder.cpp
    src/ImageDecoderFrame.cpp
    src/ImageEncoder.cpp
)
