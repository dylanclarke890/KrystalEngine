#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontDatabase.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include <filesystem>

namespace krys::boo::layout::tests
{
  void RegisterAhem()
  {
    (void)FontDatabase::Singleton().AddFontFile(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts"
                                                / "Ahem.ttf");
  }

  utf8_string AhemDocument(utf8_stringview style, utf8_stringview body)
  {
    RegisterAhem();
    auto html = utf8_string {u8"<!DOCTYPE html><style>body { margin: 0; font: 10px Ahem }"};
    html += style;
    html += u8"</style><body>";
    html += body;
    html += u8"</body>";
    return html;
  }

  utf8_string DumpWithIds(style::tests::ResolverFixture &fixture) noexcept
  {
    return ExternalRepresentation(fixture.Frame.get(), {RenderAsTextFlag::ShowIDAndClass});
  }

  void DetachLineLayout(RenderBlock &renderer) noexcept
  {
    if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(renderer); blockFlow && blockFlow->InlineLayout())
    {
      blockFlow->InvalidateLineLayout(RenderBlockFlow::InvalidationReason::InternalMove);
    }
  }

  void DetachLineLayouts(style::tests::ResolverFixture &fixture) noexcept
  {
    for (auto &blockFlow : DescendantsOfType<RenderBlockFlow>(*fixture.Document->GetRenderView()))
    {
      DetachLineLayout(blockFlow);
    }
  }

  ElementBox &BuildBoxTree(RenderBlock &renderer) noexcept
  {
    DetachLineLayout(renderer);
    return *layout_integration::BoxTreeUpdater(renderer).Build();
  }

  const ElementBox &RootBoxOf(style::tests::ResolverFixture &fixture, dom::DOMStringView id) noexcept
  {
    auto *renderer = DynamicDowncast<RenderBlockFlow>(fixture.ById(id).Renderer());
    REQUIRE(renderer);
    return BuildBoxTree(*renderer);
  }
}
