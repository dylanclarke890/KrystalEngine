#include "Krystal.Booey/DOM/Internals/RareData/ElementRareData.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo::dom
{
  ElementRareData::ElementRareData() noexcept = default;

  ElementRareData::~ElementRareData() noexcept
  {
    krys_debug_assert(!_beforePseudoElement);
    krys_debug_assert(!_afterPseudoElement);
  }

  void ElementRareData::SetComputedStyle(UniquePtr<RenderStyle> style) noexcept
  {
    _computedStyle = krys::move(style);
  }

  void ElementRareData::SetDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept
  {
    _displayContentsOrNoneStyle = krys::move(style);
  }

  void ElementRareData::SetBeforePseudoElement(RefPtr<PseudoElement> &&pseudoElement) noexcept
  {
    krys_debug_assert(!_beforePseudoElement || !pseudoElement);
    _beforePseudoElement = krys::move(pseudoElement);
  }

  void ElementRareData::SetAfterPseudoElement(RefPtr<PseudoElement> &&pseudoElement) noexcept
  {
    krys_debug_assert(!_afterPseudoElement || !pseudoElement);
    _afterPseudoElement = krys::move(pseudoElement);
  }
}
