#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"

namespace krys::boo::css
{
  PlatformColorResolutionDelegate::~PlatformColorResolutionDelegate() noexcept = default;

  boo::Color PlatformColorResolutionDelegate::CurrentColor() const noexcept
  {
    return {};
  }

  boo::Color PlatformColorResolutionDelegate::InternalDocumentTextColor() const noexcept
  {
    return {};
  }

  boo::Color PlatformColorResolutionDelegate::WebkitLink() const noexcept
  {
    return {};
  }

  boo::Color PlatformColorResolutionDelegate::WebkitLinkVisited() const noexcept
  {
    return {};
  }

  boo::Color PlatformColorResolutionDelegate::WebkitActiveLink() const noexcept
  {
    return {};
  }

  boo::Color PlatformColorResolutionDelegate::WebkitFocusRingColor() const noexcept
  {
    return {};
  }
}
