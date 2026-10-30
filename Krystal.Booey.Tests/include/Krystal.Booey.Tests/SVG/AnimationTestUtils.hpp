#pragma once

#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  /// @brief A loaded page, so its animations start as they do for the host, whose `<svg id=s>` timeline is
  /// paused at 0 for tests to seek.
  struct AnimationFixture : boo::tests::LoadFixture
  {
    explicit AnimationFixture(utf8_string &&markup) noexcept
    {
      LoadMarkup(u8"<!DOCTYPE html>" + markup, u8"https://example.com/");
      html::EventLoop::Current().RunUntilIdle();
      Svg().PauseAnimations();
      Svg().SetCurrentTime(0);
    }

    template <typename T>
    KRYS_NODISCARD T &ById(dom::DOMStringView id) const noexcept
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
