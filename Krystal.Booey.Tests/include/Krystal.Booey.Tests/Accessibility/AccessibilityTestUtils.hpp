#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/PageInFrameFixture.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Booey/Accessibility/AXLogger.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"

namespace krys::boo::ax::tests
{
  /// @brief A laid-out page in a frame with accessibility enabled (for the fixture's lifetime).
  struct AccessibilityFixture : boo::tests::PageInFrameFixture
  {
    explicit AccessibilityFixture(utf8_string &&markup, IntSize viewSize = {400, 300}) noexcept
        : PageInFrameFixture(krys::move(markup), viewSize)
    {
      AXObjectCache::EnableAccessibility();
      (void)Document->UpdateLayout();
    }

    ~AccessibilityFixture() noexcept
    {
      AXObjectCache::DisableAccessibility();
    }

    KRYS_NODISCARD AXObjectCache &Cache() noexcept
    {
      auto *cache = Document->AxObjectCache();
      REQUIRE(cache);
      return *cache;
    }

    /// @brief The object for the element with @p id, created if needed; fails the test if there's none.
    KRYS_NODISCARD AccessibilityObject &Object(dom::DOMStringView id) noexcept
    {
      auto *object = Cache().GetOrCreate(ById(id));
      REQUIRE(object);
      return *object;
    }

    /// @brief The tree from the root, as `AXLogger` dumps it.
    KRYS_NODISCARD utf8_string Tree(EnumSet<AXStreamOption> options = {AXStreamOption::Role,
                                                                       AXStreamOption::Name}) noexcept
    {
      return AXLogger::TreeAsText(Cache(), options);
    }
  };
}
