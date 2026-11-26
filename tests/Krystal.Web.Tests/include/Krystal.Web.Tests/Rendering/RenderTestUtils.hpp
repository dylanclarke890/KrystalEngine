#pragma once

#include "Krystal.Web.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Web/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Web/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"

namespace krys::web::tests
{
  /// @brief The renderer of the element of @p fixture's document with the id @p id.
  KRYS_NODISCARD inline RenderElement &RendererOf(style::tests::ResolverFixture &fixture,
                                                  DOMStringView id) noexcept
  {
    return *fixture.ById(id).Renderer();
  }

  /// @brief The renderer of the element of @p fixture's document with the id @p id, after laying the document
  /// out.
  KRYS_NODISCARD inline RenderElement &LaidOutRendererOf(style::tests::ResolverFixture &fixture,
                                                         DOMStringView id) noexcept
  {
    (void)fixture.Document->UpdateLayout();
    return RendererOf(fixture, id);
  }

  /// @brief The box of the element of @p fixture's document with the id @p id.
  KRYS_NODISCARD inline RenderBox &BoxOf(style::tests::ResolverFixture &fixture, DOMStringView id) noexcept
  {
    return Downcast<RenderBox>(RendererOf(fixture, id));
  }

  /// @brief The box of the element of @p fixture's document with the id @p id, after laying the document out.
  KRYS_NODISCARD inline RenderBox &LaidOutBoxOf(style::tests::ResolverFixture &fixture,
                                                DOMStringView id) noexcept
  {
    (void)fixture.Document->UpdateLayout();
    return BoxOf(fixture, id);
  }

  /// @brief The frame rect of the box of the element of @p fixture's document with the id @p id, after laying
  /// the document out.
  KRYS_NODISCARD inline LayoutRect FrameRectOf(style::tests::ResolverFixture &fixture,
                                               DOMStringView id) noexcept
  {
    return LaidOutBoxOf(fixture, id).FrameRect();
  }

  /// @brief The scrollable area of @p element's box, which must have one.
  KRYS_NODISCARD inline ScrollableArea &ScrollableAreaOf(Element &element) noexcept
  {
    auto *area = Downcast<RenderBox>(*element.Renderer()).Layer()->GetScrollableArea();
    REQUIRE(area);
    return *area;
  }
}
