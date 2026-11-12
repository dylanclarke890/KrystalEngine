/*
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 *
 * Portions are Copyright (C) 2001-6 mozilla.org
 *
 * Other contributors:
 *   Stuart Parmenter <stuart@mozilla.com>
 *
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 */

#include "Krystal.Booey/Platform/ImageDecoders/EXIFMetadata.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr uint32 ShortType = 3;
    constexpr uint32 LongType = 4;
    constexpr uint32 RationalType = 5;

    constexpr uint32 OrientationTag = 0x112;
    constexpr uint32 XResolutionTag = 0x11A;
    constexpr uint32 YResolutionTag = 0x11B;
    constexpr uint32 ResolutionUnitTag = 0x128;
    constexpr uint32 ExifIFDPointerTag = 0x8769;
    constexpr uint32 PixelXDimensionTag = 0xA002;
    constexpr uint32 PixelYDimensionTag = 0xA003;

    constexpr uint32 InchesResolutionUnit = 2;
    // EXIF's resolution is in points per inch, so 72 is the resolution of an image shown at its own size.
    constexpr float DefaultResolution = 72;

    /// @brief Reads a TIFF block's integers in its byte order, within its bounds.
    class TIFFReader
    {
    private:
      Span<const uint8> _data;
      bool _isBigEndian;

    public:
      TIFFReader(Span<const uint8> data, bool isBigEndian) noexcept : _data(data), _isBigEndian(isBigEndian)
      {
      }

      KRYS_NODISCARD Maybe<uint32> Uint16(size_t offset) const noexcept
      {
        if (offset > _data.size() || _data.size() - offset < 2)
        {
          return null;
        }
        auto *data = _data.data() + offset;
        return _isBigEndian ? (uint32(data[0]) << 8) | data[1] : (uint32(data[1]) << 8) | data[0];
      }

      KRYS_NODISCARD Maybe<uint32> Uint32(size_t offset) const noexcept
      {
        auto first = Uint16(offset);
        auto second = Uint16(offset + 2);
        if (!first || !second)
        {
          return null;
        }
        return _isBigEndian ? (*first << 16) | *second : (*second << 16) | *first;
      }

      /// @brief The single integer (a short or a long) of the entry at @p entry.
      KRYS_NODISCARD Maybe<uint32> Integer(size_t entry) const noexcept
      {
        auto type = Uint16(entry + 2);
        if (Uint32(entry + 4) != 1u || !type)
        {
          return null;
        }
        if (*type == ShortType)
        {
          return Uint16(entry + 8);
        }
        if (*type == LongType)
        {
          return Uint32(entry + 8);
        }
        return null;
      }

      /// @brief The single rational of the entry at @p entry, stored at the offset it holds.
      KRYS_NODISCARD Maybe<float> Rational(size_t entry) const noexcept
      {
        if (Uint16(entry + 2) != RationalType || Uint32(entry + 4) != 1u)
        {
          return null;
        }
        auto offset = Uint32(entry + 8);
        if (!offset)
        {
          return null;
        }
        auto numerator = Uint32(*offset);
        auto denominator = Uint32(size_t(*offset) + 4);
        if (!numerator || !denominator || !*denominator)
        {
          return null;
        }
        return static_cast<float>(*numerator) / static_cast<float>(*denominator);
      }

      /// @brief Calls @p function with each tag of the image file directory at @p offset, and the offset of
      /// its entry.
      template <typename TFunction>
      void ForEachEntry(size_t offset, TFunction &&function) const noexcept
      {
        // An image file directory is a count of entries, then that many 12-byte entries: the tag, the type,
        // the count, and the value or the offset to it.
        auto count = Uint16(offset);
        if (!count)
        {
          return;
        }
        constexpr size_t EntrySize = 12;
        for (size_t entry = offset + 2, end = entry + *count * EntrySize; entry < end; entry += EntrySize)
        {
          auto tag = Uint16(entry);
          if (!tag)
          {
            return;
          }
          function(*tag, entry);
        }
      }
    };
  }

  EXIFMetadata EXIFMetadata::Read(Span<const uint8> tiff, const IntSize &physicalSize) noexcept
  {
    // A TIFF block starts with its byte order (`II`, little endian, or `MM`, big endian), 42, and the offset
    // of its first image file directory. See https://www.cipa.jp/std/documents/e/DC-008-2012_E.pdf.
    EXIFMetadata metadata;
    if (tiff.size() < 8 || !((tiff[0] == 'I' && tiff[1] == 'I') || (tiff[0] == 'M' && tiff[1] == 'M')))
    {
      return metadata;
    }
    TIFFReader reader(tiff, tiff[0] == 'M');
    auto firstDirectory = reader.Uint32(4);
    if (reader.Uint16(2) != 42u || !firstDirectory)
    {
      return metadata;
    }

    Maybe<float> resolutionX;
    Maybe<float> resolutionY;
    Maybe<uint32> resolutionUnit;
    Maybe<uint32> exifDirectory;
    reader.ForEachEntry(*firstDirectory,
                        [&](uint32 tag, size_t entry)
                        {
                          switch (tag)
                          {
                            case OrientationTag:
                              if (auto value = reader.Integer(entry))
                              {
                                metadata.Orientation =
                                  ImageOrientation::FromEXIFValue(static_cast<int>(*value));
                              }
                              break;
                            case XResolutionTag:    resolutionX = reader.Rational(entry); break;
                            case YResolutionTag:    resolutionY = reader.Rational(entry); break;
                            case ResolutionUnitTag: resolutionUnit = reader.Integer(entry); break;
                            case ExifIFDPointerTag: exifDirectory = reader.Integer(entry); break;
                            default:                break;
                          }
                        });

    Maybe<uint32> dimensionX;
    Maybe<uint32> dimensionY;
    if (exifDirectory && *exifDirectory != *firstDirectory)
    {
      reader.ForEachEntry(*exifDirectory,
                          [&](uint32 tag, size_t entry)
                          {
                            if (tag == PixelXDimensionTag)
                            {
                              dimensionX = reader.Integer(entry);
                            }
                            else if (tag == PixelYDimensionTag)
                            {
                              dimensionY = reader.Integer(entry);
                            }
                          });
    }

    // https://html.spec.whatwg.org/#preparing-an-image-for-presentation: the pixel dimensions are the
    // preferred density-corrected dimensions when the resolution, in inches, scales the image's size to them.
    if (!dimensionX || !dimensionY || !*dimensionX || !*dimensionY || !resolutionX || !resolutionY
        || *resolutionX <= 0 || *resolutionY <= 0 || resolutionUnit != InchesResolutionUnit)
    {
      return metadata;
    }
    if (static_cast<float>(physicalSize.Width()) * DefaultResolution / *resolutionX
          == static_cast<float>(*dimensionX)
        && static_cast<float>(physicalSize.Height()) * DefaultResolution / *resolutionY
             == static_cast<float>(*dimensionY))
    {
      metadata.DensityCorrectedSize =
        IntSize(static_cast<int32>(*dimensionX), static_cast<int32>(*dimensionY));
    }
    return metadata;
  }
}
