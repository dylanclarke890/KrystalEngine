#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/Encodings/Encoding.hpp"
#include "Krystal.Text/Unicode.hpp"

namespace Krys
{
  class XUserDefinedEncoding : public Encoding
  {
  private:
    KRYS_NODISCARD static EncodingInfo GetEncodingInfo() noexcept
    {
      static EncodingInfo info {
        .Name = u8"x-user-defined",
        .Aliases = {u8"x-user-defined"},
        .IsSingleByte = IsSingleByteEncoding {true},
      };

      return info;
    }

  public:
    static constexpr uint32 XUserDefinedStart = 0xF780u;
    static constexpr uint32 XUserDefinedEnd = 0xF7FFu;

  public:
    XUserDefinedEncoding() noexcept : Encoding(GetEncodingInfo())
    {
    }

    virtual ~XUserDefinedEncoding() noexcept = default;

    void Encode(Span<const Rune> characters, List<byte> &out) const noexcept override
    {
      for (Rune ch : characters)
      {
        if (ASCII::IsASCII(ch))
        {
          out.push_back(static_cast<byte>(ch.Value));
        }
        else if (ch >= XUserDefinedStart && ch <= XUserDefinedEnd)
        {
          out.push_back(byte {ch.Value - XUserDefinedStart + ASCII::ExtendedASCIIStart});
        }
        else
        {
          out.push_back(ASCII::ReplacementCharacter.ToByte());
        }
      }
    }

    KRYS_NODISCARD size_t GetMaxByteCount(size_t charCount) const noexcept override
    {
      return charCount; // 1 byte per character in x-user-defined
    }

    void Decode(Span<const byte> bytes, List<Rune> &out) const noexcept override
    {
      for (byte b : bytes)
      {
        if (ASCII::IsASCII(b))
        {
          out.push_back(Rune(b));
        }
        else
        {
          uint8 ub = static_cast<uint8>(b);
          out.push_back(Rune(XUserDefinedStart + (ub - ASCII::ExtendedASCIIStart)));
        }
      }
    }

    KRYS_NODISCARD size_t GetMaxCharCount(size_t byteCount) const noexcept override
    {
      return byteCount; // 1 character per byte in x-user-defined
    }
  };
}