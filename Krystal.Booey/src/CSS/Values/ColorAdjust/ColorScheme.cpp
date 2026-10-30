#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  void Serialiser<ColorScheme>::operator()(StringBuilder &builder, const SerialisationContext &context,
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
      builder.Append(u8' ');
      Serialise(builder, context, *value.Only);
    }
  }
}
