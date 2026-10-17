#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include <filesystem>

namespace krys::boo::layout::tests
{
  /// @brief Registers the Ahem test font, whose glyphs are 1em squares (0.8em above the baseline, 0.2em
  /// below).
  inline void RegisterAhem()
  {
    (void)FontDatabase::Singleton().AddFontFile(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts"
                                                / "Ahem.ttf");
  }

  /// @brief Builds the layout tree of the block flow with id @p id and returns its root box.
  KRYS_NODISCARD inline const ElementBox &RootBoxOf(style::tests::ResolverFixture &fixture,
                                                    dom::DOMStringView id) noexcept
  {
    auto *renderer = DynamicDowncast<RenderBlockFlow>(fixture.ById(id).Renderer());
    REQUIRE(renderer);
    return *layout_integration::BoxTreeUpdater(*renderer).Build();
  }
}
