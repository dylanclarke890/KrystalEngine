#pragma once

#include "Krystal.Core/Text/Encodings/Base.hpp"

namespace krys::text
{
  class ICodec
  {
  public:
    constexpr virtual ~ICodec() noexcept = default;

    KRYS_NODISCARD constexpr virtual EncodingId Id() const noexcept = 0;

    KRYS_NODISCARD constexpr virtual utf8_stringview Name() const noexcept = 0;

    KRYS_NODISCARD constexpr virtual Span<const utf8_stringview> Aliases() const noexcept = 0;
  };

  template <Encoding TEncoding>
  class Codec final : public ICodec
  {
  public:
    constexpr Codec() noexcept = default;

    KRYS_NODISCARD constexpr EncodingId Id() const noexcept override
    {
      return TEncoding::Id;
    }

    KRYS_NODISCARD constexpr utf8_stringview Name() const noexcept override
    {
      return TEncoding::Name;
    }

    KRYS_NODISCARD constexpr Span<const utf8_stringview> Aliases() const noexcept override
    {
      return TEncoding::Aliases;
    }
  };
}