#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleBase.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+ConstructionInlines.hpp"

namespace krys::boo
{
  inline RenderStyleBase::RenderStyleBase(RenderStyleBase &&) noexcept = default;
  inline RenderStyleBase &RenderStyleBase::operator=(RenderStyleBase &&) noexcept = default;

  inline RenderStyleBase::RenderStyleBase(CreateDefaultStyleTag) noexcept
      : _computedStyle(style::ComputedStyle::CreateDefaultStyle)
  {
  }

  inline RenderStyleBase::RenderStyleBase(const RenderStyleBase &other, CloneTag) noexcept
      : _computedStyle(other._computedStyle, style::ComputedStyle::Clone)
  {
  }

  inline RenderStyleBase::RenderStyleBase(RenderStyleBase &a, RenderStyleBase &&b) noexcept
      : _computedStyle(a._computedStyle, krys::move(b._computedStyle))
  {
  }
}
