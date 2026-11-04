#include "Krystal.Booey/Style/Interpolation/ChangedAnimatableProperties.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleProperties+GettersInlines.hpp"
#include "Krystal.Booey/Style/Interpolation/ChangedAnimatablePropertiesGenerated.hpp"

namespace krys::boo::style
{
  void ConservativelyCollectChangedAnimatableProperties(const RenderStyle &a, const RenderStyle &b,
                                                        CSSPropertiesBitSet &changingProperties) noexcept
  {
    // Check property values on RenderStyle for changes.
    ChangedAnimatablePropertiesGenerated::ConservativelyCollectChangedAnimatableProperties(
      a.ComputedStyle(), b.ComputedStyle(), changingProperties);

    // Also, check some non-property and/or derived values on RenderStyle for changes.

    // `writingMode` changes conversion of logical -> physical properties, thus, we need to add all physical
    // properties.
    if (a.GetWritingMode() != b.GetWritingMode())
    {
      changingProperties.Merge(css::Property::PhysicalProperties());
      if (a.GetWritingMode().IsVerticalTypographic() != b.GetWritingMode().IsVerticalTypographic())
      {
        changingProperties.Set(css::PropertyId::TextEmphasisStyle);
      }
    }
  }
}
