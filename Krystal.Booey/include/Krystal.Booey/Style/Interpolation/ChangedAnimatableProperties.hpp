#pragma once

namespace krys::boo
{
  class RenderStyle;
  struct CSSPropertiesBitSet;
}

namespace krys::boo::style
{
  /// @brief Adds the properties whose values may differ between @p a and @p b to @p changingProperties.
  /// @note Conservatively answers what CSS properties we should visit for CSS transitions. We do not need to
  /// precisely check equivalence before saying "this property needs to be visited".
  void ConservativelyCollectChangedAnimatableProperties(const RenderStyle &a, const RenderStyle &b,
                                                        CSSPropertiesBitSet &changingProperties) noexcept;
}
