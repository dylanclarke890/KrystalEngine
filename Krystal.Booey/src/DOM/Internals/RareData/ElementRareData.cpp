#include "Krystal.Booey/DOM/Internals/RareData/ElementRareData.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo::dom
{
  ElementRareData::ElementRareData() noexcept = default;

  ElementRareData::~ElementRareData() noexcept = default;

  void ElementRareData::SetComputedStyle(UniquePtr<RenderStyle> style) noexcept
  {
    _computedStyle = krys::move(style);
  }

  void ElementRareData::SetDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept
  {
    _displayContentsOrNoneStyle = krys::move(style);
  }
}
