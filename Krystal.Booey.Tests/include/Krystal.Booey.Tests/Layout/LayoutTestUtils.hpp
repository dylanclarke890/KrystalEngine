#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
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

  /// @brief A document whose body has no margin and uses 10px Ahem, so every character is 10px wide and a
  /// line is 10px tall, with @p style appended to its style sheet and @p body as its body's content.
  KRYS_NODISCARD inline utf8_string AhemDocument(utf8_stringview style, utf8_stringview body)
  {
    RegisterAhem();
    auto html = utf8_string {u8"<!DOCTYPE html><style>body { margin: 0; font: 10px Ahem }"};
    html += style;
    html += u8"</style><body>";
    html += body;
    html += u8"</body>";
    return html;
  }

  /// @brief The render tree dump of the document in @p fixture, with element ids and classes.
  KRYS_NODISCARD inline utf8_string DumpWithIds(style::tests::ResolverFixture &fixture) noexcept
  {
    return ExternalRepresentation(fixture.Frame.get(), {RenderAsTextFlag::ShowIDAndClass});
  }

  /// @brief Releases the line layout a block container got from layout, which tears down its layout boxes.
  inline void DetachLineLayout(RenderBlock &renderer) noexcept
  {
    if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(renderer); blockFlow && blockFlow->InlineLayout())
    {
      blockFlow->InvalidateLineLayout(RenderBlockFlow::InvalidationReason::InternalMove);
    }
  }

  /// @brief Releases the line layouts of every block container of the document laid out in @p fixture.
  inline void DetachLineLayouts(style::tests::ResolverFixture &fixture) noexcept
  {
    for (auto &blockFlow : DescendantsOfType<RenderBlockFlow>(*fixture.Document->GetRenderView()))
    {
      DetachLineLayout(blockFlow);
    }
  }

  /// @brief Builds the layout tree of @p renderer, after releasing the one its line layout built.
  KRYS_NODISCARD inline ElementBox &BuildBoxTree(RenderBlock &renderer) noexcept
  {
    DetachLineLayout(renderer);
    return *layout_integration::BoxTreeUpdater(renderer).Build();
  }

  /// @brief Builds the layout tree of the block flow with id @p id and returns its root box.
  KRYS_NODISCARD inline const ElementBox &RootBoxOf(style::tests::ResolverFixture &fixture,
                                                    dom::DOMStringView id) noexcept
  {
    auto *renderer = DynamicDowncast<RenderBlockFlow>(fixture.ById(id).Renderer());
    REQUIRE(renderer);
    return BuildBoxTree(*renderer);
  }
}
