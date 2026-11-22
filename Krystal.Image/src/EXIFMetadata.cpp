#include "Krystal.Image/EXIFMetadata.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include <algorithm>

// EXIF stores its metadata as a TIFF block: a header, then image file directories (IFDs) of tagged entries.
// See CIPA DC-008 (https://www.cipa.jp/std/documents/e/DC-008-2012_E.pdf).

namespace krys::img
{
  namespace
  {
    enum class EntryType : uint16
    {
      Short = 3,
      Long = 4,
      Rational = 5,
    };

    enum class Tag : uint16
    {
      Orientation = 0x0112,
      XResolution = 0x011A,
      YResolution = 0x011B,
      ResolutionUnit = 0x0128,
      EXIFDirectory = 0x8769,
      PixelXDimension = 0xA002,
      PixelYDimension = 0xA003,
    };

    constexpr uint32 ResolutionUnitInches = 2;
    // The resolution, in dots per inch, of an image meant to be shown at its own pixel size.
    constexpr float NaturalResolution = 72;

    // A directory entry: its tag and type, how many values it has, and where its 4-byte value field is. A
    // value that fits is stored in the field itself; a larger one is stored at the offset the field holds.
    struct DirectoryEntry
    {
      uint16 TagValue;
      uint16 Type;
      uint32 Count;
      size_t ValueField;
    };

    class TIFFBlock
    {
    private:
      Span<const uint8> _bytes;
      bool _bigEndian;

    public:
      TIFFBlock(Span<const uint8> bytes, bool bigEndian) noexcept : _bytes(bytes), _bigEndian(bigEndian)
      {
      }

      // The unsigned integer of @p size bytes at @p offset, if it's within the block.
      KRYS_NODISCARD Maybe<uint32> Read(size_t offset, size_t size) const noexcept
      {
        if (offset >= _bytes.size() || _bytes.size() - offset < size)
        {
          return null;
        }
        uint32 value = 0;
        for (size_t index = 0; index < size; ++index)
        {
          auto byte = _bytes[_bigEndian ? offset + index : offset + size - 1 - index];
          value = (value << 8) | byte;
        }
        return value;
      }

      KRYS_NODISCARD List<DirectoryEntry> Directory(size_t offset) const noexcept
      {
        List<DirectoryEntry> entries;
        auto count = Read(offset, 2);
        if (!count)
        {
          return entries;
        }
        constexpr size_t entrySize = 12;
        for (size_t index = 0; index < *count; ++index)
        {
          auto entry = offset + 2 + index * entrySize;
          auto tag = Read(entry, 2);
          auto type = Read(entry + 2, 2);
          auto valueCount = Read(entry + 4, 4);
          if (!tag || !type || !valueCount)
          {
            break;
          }
          entries.push_back({static_cast<uint16>(*tag), static_cast<uint16>(*type), *valueCount, entry + 8});
        }
        return entries;
      }

      // The entry's value, when it's a single short or long.
      KRYS_NODISCARD Maybe<uint32> Integer(const DirectoryEntry &entry) const noexcept
      {
        if (entry.Count != 1)
        {
          return null;
        }
        switch (static_cast<EntryType>(entry.Type))
        {
          case EntryType::Short: return Read(entry.ValueField, 2);
          case EntryType::Long:  return Read(entry.ValueField, 4);
          default:               return null;
        }
      }

      // The entry's value, when it's a single rational with a nonzero denominator.
      KRYS_NODISCARD Maybe<float> Rational(const DirectoryEntry &entry) const noexcept
      {
        if (entry.Count != 1 || static_cast<EntryType>(entry.Type) != EntryType::Rational)
        {
          return null;
        }
        auto offset = Read(entry.ValueField, 4);
        auto numerator = offset ? Read(*offset, 4) : null;
        auto denominator = offset ? Read(size_t {*offset} + 4, 4) : null;
        if (!numerator || !denominator || !*denominator)
        {
          return null;
        }
        return static_cast<float>(*numerator) / static_cast<float>(*denominator);
      }
    };

    KRYS_NODISCARD const DirectoryEntry *Find(const List<DirectoryEntry> &directory, Tag tag) noexcept
    {
      auto entry = std::ranges::find(directory, static_cast<uint16>(tag), &DirectoryEntry::TagValue);
      return entry == directory.end() ? nullptr : &*entry;
    }
  }

  EXIFMetadata EXIFMetadata::Read(Span<const uint8> tiff, const SizeI &physicalSize) noexcept
  {
    EXIFMetadata metadata;

    // The header: the byte order (`II` little endian, `MM` big endian), the number 42, then the offset of
    // the first directory.
    if (tiff.size() < 8 || tiff[0] != tiff[1] || (tiff[0] != 'I' && tiff[0] != 'M'))
    {
      return metadata;
    }
    TIFFBlock block {tiff, tiff[0] == 'M'};
    auto firstDirectoryOffset = block.Read(4, 4);
    if (block.Read(2, 2) != 42u || !firstDirectoryOffset)
    {
      return metadata;
    }
    auto firstDirectory = block.Directory(*firstDirectoryOffset);

    auto integerOf = [&](const List<DirectoryEntry> &directory, Tag tag) -> Maybe<uint32>
    {
      auto *entry = Find(directory, tag);
      return entry ? block.Integer(*entry) : null;
    };
    auto rationalOf = [&](const List<DirectoryEntry> &directory, Tag tag) -> Maybe<float>
    {
      auto *entry = Find(directory, tag);
      return entry ? block.Rational(*entry) : null;
    };

    if (auto orientation = integerOf(firstDirectory, Tag::Orientation))
    {
      metadata.Orientation = ImageOrientation::FromEXIFValue(static_cast<int>(*orientation));
    }

    // The preferred density-corrected dimensions are the pixel dimensions in the EXIF directory, when the
    // resolution (in inches) scales the image's physical size to exactly them. See
    // https://html.spec.whatwg.org/#preparing-an-image-for-presentation.
    auto exifDirectoryOffset = integerOf(firstDirectory, Tag::EXIFDirectory);
    if (!exifDirectoryOffset || *exifDirectoryOffset == *firstDirectoryOffset
        || integerOf(firstDirectory, Tag::ResolutionUnit) != ResolutionUnitInches)
    {
      return metadata;
    }
    auto resolutionX = rationalOf(firstDirectory, Tag::XResolution);
    auto resolutionY = rationalOf(firstDirectory, Tag::YResolution);
    auto exifDirectory = block.Directory(*exifDirectoryOffset);
    auto width = integerOf(exifDirectory, Tag::PixelXDimension);
    auto height = integerOf(exifDirectory, Tag::PixelYDimension);
    if (!resolutionX || !resolutionY || *resolutionX <= 0 || *resolutionY <= 0 || !width || !height || !*width
        || !*height)
    {
      return metadata;
    }

    auto scalesTo = [](int32 physical, float resolution, uint32 preferred)
    {
      return static_cast<float>(physical) * NaturalResolution / resolution == static_cast<float>(preferred);
    };
    if (scalesTo(physicalSize.Width(), *resolutionX, *width)
        && scalesTo(physicalSize.Height(), *resolutionY, *height))
    {
      metadata.DensityCorrectedSize = SizeI {static_cast<int32>(*width), static_cast<int32>(*height)};
    }
    return metadata;
  }
}
