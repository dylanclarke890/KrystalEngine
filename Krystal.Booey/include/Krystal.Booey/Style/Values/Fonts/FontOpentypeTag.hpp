#pragma once

#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <opentype-tag> = <string>
  // https://drafts.csswg.org/css-fonts-4/#typedef-opentype-tag
  struct FontOpentypeTag
  {
    boo::FontTag Value;

    KRYS_NODISCARD constexpr bool operator==(const FontOpentypeTag &) const noexcept = default;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<FontOpentypeTag>
  {
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &, const RenderStyle &,
                    const FontOpentypeTag &tag) noexcept
    {
      builder.push_back(u8'"');
      for (auto c : tag.Value)
      {
        builder.push_back(static_cast<char8>(c));
      }
      builder.push_back(u8'"');
    }
  };
}
