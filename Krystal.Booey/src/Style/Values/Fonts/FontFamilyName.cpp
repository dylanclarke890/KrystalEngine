#include "Krystal.Booey/Style/Values/Fonts/FontFamilyName.hpp"

#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Ref<css::Value> CSSValueCreation<FontFamilyName>::operator()(css::ValuePool &pool, const RenderStyle &,
                                                               const FontFamilyName &value) noexcept
  {
    return pool.CreateFontFamilyValue(value.Value);
  }

  // MARK: - Serialization

  void Serialiser<FontFamilyName>::operator()(css::CSSOMString &builder, const css::SerialisationContext &,
                                              const RenderStyle &, const FontFamilyName &value) noexcept
  {
    builder.append(css::SerialiseFontFamily(css::CSSOMString {value.Value.View()}));
  }
}
