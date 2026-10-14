#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <family-name> = <string> | <custom-ident>+
  // NOTE: Per spec, "If a sequence of identifiers is given as a <family-name>, the computed value is the name
  // converted to a string by joining all the identifiers in the sequence by single spaces."
  // https://drafts.csswg.org/css-fonts-4/#family-name-syntax
  struct FontFamilyName
  {
    css::CSSOMStringAtom Value;

    KRYS_NODISCARD bool operator==(const FontFamilyName &) const noexcept = default;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueCreation<FontFamilyName>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const FontFamilyName &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<FontFamilyName>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const FontFamilyName &) noexcept;
  };
}
