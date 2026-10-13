#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"

namespace krys::boo::css
{
  void Serialiser<ColorScheme>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                           const ColorScheme &value) noexcept
  {
    if (value.IsNormal())
    {
      Serialise(builder, context, keywords::Normal {});
      return;
    }

    Serialise(builder, context, value.Schemes);
    if (value.Only)
    {
      builder.push_back(u8' ');
      Serialise(builder, context, *value.Only);
    }
  }
}
