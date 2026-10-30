#include "Krystal.Booey/Graphics/WOFFFileFormat.hpp"
#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <cstring>
#include <woff2/decode.h>
#include <zlib.h>

namespace krys::boo
{
  namespace
  {
    constexpr uint32 Woff2Signature = 0x77'4f'46'32; // "wOF2"
    constexpr uint32 WoffSignature = 0x77'4f'46'46;  // "wOFF"

    KRYS_NODISCARD bool ReadUInt32(const SharedBuffer &buffer, size_t &offset, uint32 &value) noexcept
    {
      krys_debug_assert(offset <= buffer.Size());
      if (buffer.Size() - offset < sizeof(value))
      {
        return false;
      }
      auto bytes = buffer.GetSpan().subspan(offset, sizeof(value));
      value =
        (uint32(bytes[0]) << 24) | (uint32(bytes[1]) << 16) | (uint32(bytes[2]) << 8) | uint32(bytes[3]);
      offset += sizeof(value);
      return true;
    }

    KRYS_NODISCARD bool ReadUInt16(const SharedBuffer &buffer, size_t &offset, uint16 &value) noexcept
    {
      krys_debug_assert(offset <= buffer.Size());
      if (buffer.Size() - offset < sizeof(value))
      {
        return false;
      }
      auto bytes = buffer.GetSpan().subspan(offset, sizeof(value));
      value = static_cast<uint16>((uint32(bytes[0]) << 8) | uint32(bytes[1]));
      offset += sizeof(value);
      return true;
    }

    void WriteUInt32(List<uint8> &vector, uint32 value) noexcept
    {
      vector.push_back(static_cast<uint8>(value >> 24));
      vector.push_back(static_cast<uint8>(value >> 16));
      vector.push_back(static_cast<uint8>(value >> 8));
      vector.push_back(static_cast<uint8>(value));
    }

    void WriteUInt16(List<uint8> &vector, uint16 value) noexcept
    {
      vector.push_back(static_cast<uint8>(value >> 8));
      vector.push_back(static_cast<uint8>(value));
    }

    void PutUInt32(uint8 *destination, uint32 value) noexcept
    {
      destination[0] = static_cast<uint8>(value >> 24);
      destination[1] = static_cast<uint8>(value >> 16);
      destination[2] = static_cast<uint8>(value >> 8);
      destination[3] = static_cast<uint8>(value);
    }

    KRYS_NODISCARD bool IsWOFF(const SharedBuffer &buffer) noexcept
    {
      size_t offset = 0;
      uint32 signature;
      if (!ReadUInt32(buffer, offset, signature))
      {
        return false;
      }
      return signature == WoffSignature || signature == Woff2Signature;
    }

    class WOFF2VectorOut final : public woff2::WOFF2Out
    {
    private:
      List<uint8> &_vector;

    public:
      explicit WOFF2VectorOut(List<uint8> &vector) noexcept : _vector(vector)
      {
      }

      bool Write(const void *data, size_t n) override
      {
        auto *bytes = static_cast<const uint8 *>(data);
        _vector.insert(_vector.end(), bytes, bytes + n);
        return true;
      }

      bool Write(const void *data, size_t offset, size_t n) override
      {
        if (offset + n > _vector.size())
        {
          _vector.resize(offset + n);
        }
        std::memcpy(_vector.data() + offset, data, n);
        return true;
      }

      size_t Size() override
      {
        return _vector.size();
      }
    };

    KRYS_NODISCARD bool ConvertWOFFToSfnt(const SharedBuffer &woff, List<uint8> &sfnt) noexcept
    {
      krys_debug_assert(sfnt.empty());

      size_t offset = 0;

      // Read the WOFF header.
      uint32 signature;
      if (!ReadUInt32(woff, offset, signature))
      {
        krys_debug_assert(false);
        return false;
      }

      if (signature == Woff2Signature)
      {
        auto woffData = woff.GetSpan();
        const size_t sfntSize = woff2::ComputeWOFF2FinalSize(woffData.data(), woffData.size());
        sfnt.reserve(sfntSize);
        WOFF2VectorOut out(sfnt);
        return woff2::ConvertWOFF2ToTTF(woffData.data(), woffData.size(), &out);
      }

      if (signature != WoffSignature)
      {
        krys_debug_assert(false);
        return false;
      }

      uint32 flavor;
      if (!ReadUInt32(woff, offset, flavor))
      {
        return false;
      }

      uint32 length;
      if (!ReadUInt32(woff, offset, length) || length != woff.Size())
      {
        return false;
      }

      uint16 numTables;
      if (!ReadUInt16(woff, offset, numTables))
      {
        return false;
      }

      if (!numTables || numTables > 0x0fff)
      {
        return false;
      }

      uint16 reserved;
      if (!ReadUInt16(woff, offset, reserved) || reserved)
      {
        return false;
      }

      uint32 totalSfntSize;
      if (!ReadUInt32(woff, offset, totalSfntSize))
      {
        return false;
      }

      if (woff.Size() - offset < sizeof(uint16) + sizeof(uint16) + 5 * sizeof(uint32))
      {
        return false;
      }

      offset += sizeof(uint16); // majorVersion
      offset += sizeof(uint16); // minorVersion
      offset += sizeof(uint32); // metaOffset
      offset += sizeof(uint32); // metaLength
      offset += sizeof(uint32); // metaOrigLength
      offset += sizeof(uint32); // privOffset
      offset += sizeof(uint32); // privLength

      // Check if the WOFF can supply as many tables as it claims it has.
      if (woff.Size() - offset < numTables * 5 * sizeof(uint32))
      {
        return false;
      }

      // Write the sfnt offset subtable.
      uint16 entrySelector = 0;
      uint16 searchRange = 1;
      while (searchRange < numTables >> 1)
      {
        entrySelector++;
        searchRange <<= 1;
      }
      searchRange <<= 4;
      uint16 rangeShift = static_cast<uint16>((numTables << 4) - searchRange);

      WriteUInt32(sfnt, flavor);
      WriteUInt16(sfnt, numTables);
      WriteUInt16(sfnt, searchRange);
      WriteUInt16(sfnt, entrySelector);
      WriteUInt16(sfnt, rangeShift);

      if (sfnt.size() > totalSfntSize)
      {
        return false;
      }

      if (totalSfntSize - sfnt.size() < numTables * 4 * sizeof(uint32))
      {
        return false;
      }

      size_t sfntTableDirectoryCursor = sfnt.size();
      sfnt.resize(sfnt.size() + numTables * 4 * sizeof(uint32));

      // Process tables.
      for (uint16 i = 0; i < numTables; ++i)
      {
        // Read a WOFF table directory entry.
        uint32 tableTag;
        if (!ReadUInt32(woff, offset, tableTag))
        {
          return false;
        }

        uint32 tableOffset;
        if (!ReadUInt32(woff, offset, tableOffset))
        {
          return false;
        }

        uint32 tableCompLength;
        if (!ReadUInt32(woff, offset, tableCompLength))
        {
          return false;
        }

        if (tableOffset > woff.Size() || tableCompLength > woff.Size() - tableOffset)
        {
          return false;
        }

        uint32 tableOrigLength;
        if (!ReadUInt32(woff, offset, tableOrigLength) || tableCompLength > tableOrigLength)
        {
          return false;
        }

        if (tableOrigLength > totalSfntSize || sfnt.size() > totalSfntSize - tableOrigLength)
        {
          return false;
        }

        uint32 tableOrigChecksum;
        if (!ReadUInt32(woff, offset, tableOrigChecksum))
        {
          return false;
        }

        // Write an sfnt table directory entry.
        uint8 *sfntTableDirectory = sfnt.data() + sfntTableDirectoryCursor;
        PutUInt32(sfntTableDirectory, tableTag);
        PutUInt32(sfntTableDirectory + 4, tableOrigChecksum);
        PutUInt32(sfntTableDirectory + 8, static_cast<uint32>(sfnt.size()));
        PutUInt32(sfntTableDirectory + 12, tableOrigLength);
        sfntTableDirectoryCursor += 4 * sizeof(uint32);

        auto table = woff.GetSpan().subspan(tableOffset, tableCompLength);
        if (tableCompLength == tableOrigLength)
        {
          // The table is not compressed.
          sfnt.insert(sfnt.end(), table.begin(), table.end());
        }
        else
        {
          uLongf destLen = tableOrigLength;
          size_t start = sfnt.size();
          sfnt.resize(sfnt.size() + tableOrigLength);
          if (uncompress(reinterpret_cast<Bytef *>(sfnt.data() + start), &destLen,
                         reinterpret_cast<const Bytef *>(table.data()), tableCompLength)
              != Z_OK)
          {
            return false;
          }
          if (destLen != tableOrigLength)
          {
            return false;
          }
        }

        // Pad to a multiple of 4 bytes.
        while (sfnt.size() % 4)
        {
          sfnt.push_back(0);
        }
      }

      return sfnt.size() == totalSfntSize;
    }
  }

  bool ConvertWOFFToSfntIfNecessary(RefPtr<SharedBuffer> &buffer) noexcept
  {
    if (!buffer || !IsWOFF(*buffer))
    {
      return false;
    }

    List<uint8> convertedFont;
    if (ConvertWOFFToSfnt(*buffer, convertedFont))
    {
      buffer = SharedBuffer::Create(krys::move(convertedFont));
    }
    else
    {
      buffer = nullptr;
    }
    return true;
  }
}
