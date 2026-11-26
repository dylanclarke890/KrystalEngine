#pragma once

#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Web.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Svg/Elements/SVGSvgElement.hpp"

namespace krys::web::svg::tests
{
  /// @brief A loaded page, so its animations start as they do for the host, whose `<svg id=s>` timeline is
  /// paused at 0 for tests to seek.
  struct AnimationFixture : web::tests::LoadFixture
  {
    explicit AnimationFixture(const utf8_string &markup) noexcept
    {
      LoadMarkup(u8"<!DOCTYPE html>" + markup, u8"https://example.com/");
      html::EventLoop::Current().RunUntilIdle();
      Svg().PauseAnimations();
      Svg().SetCurrentTime(0);
    }

    template <typename T>
    KRYS_NODISCARD T &ById(DOMStringView id) const noexcept
    {
      auto element = Document().GetElementById(id);
      krys_assert(element);
      return Downcast<T>(Downcast<SVGElement>(*element));
    }

    KRYS_NODISCARD SVGSvgElement &Svg() const noexcept
    {
      return ById<SVGSvgElement>(u8"s");
    }

    /// @brief Seeks the timeline to @p seconds.
    void Seek(float seconds) const noexcept
    {
      Svg().SetCurrentTime(seconds);
    }
  };
}
