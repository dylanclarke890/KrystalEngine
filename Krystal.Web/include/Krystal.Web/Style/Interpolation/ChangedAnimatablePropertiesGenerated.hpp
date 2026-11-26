#pragma once

namespace krys::web
{
  struct CSSPropertiesBitSet;
}

namespace krys::web::style
{
  class ComputedStyle;

  /// @brief The changed animatable properties of the computed style's storage, generated from the properties'
  /// JSON.
  class ChangedAnimatablePropertiesGenerated final
  {
  public:
    static void
      ConservativelyCollectChangedAnimatableProperties(const ComputedStyle &a, const ComputedStyle &b,
                                                       CSSPropertiesBitSet &changingProperties) noexcept;
  };
}
