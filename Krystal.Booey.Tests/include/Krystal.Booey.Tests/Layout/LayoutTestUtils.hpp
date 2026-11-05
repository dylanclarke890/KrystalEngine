#pragma once

#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"

namespace krys::boo
{
  class RenderBlock;
  class ElementBox;
}

namespace krys::boo::layout::tests
{
  /// @brief Registers the Ahem test font, whose glyphs are 1em squares (0.8em above the baseline, 0.2em
  /// below).
  void RegisterAhem();

  /// @brief A document whose body has no margin and uses 10px Ahem, so every character is 10px wide and a
  /// line is 10px tall, with @p style appended to its style sheet and @p body as its body's content.
  KRYS_NODISCARD utf8_string AhemDocument(utf8_stringview style, utf8_stringview body);

  /// @brief The render tree dump of the document in @p fixture, with element ids and classes.
  KRYS_NODISCARD utf8_string DumpWithIds(style::tests::ResolverFixture &fixture) noexcept;

  /// @brief Releases the line layout a block container got from layout, which tears down its layout boxes.
  void DetachLineLayout(RenderBlock &renderer) noexcept;

  /// @brief Releases the line layouts of every block container of the document laid out in @p fixture.
  void DetachLineLayouts(style::tests::ResolverFixture &fixture) noexcept;

  /// @brief Builds the layout tree of @p renderer, after releasing the one its line layout built.
  KRYS_NODISCARD ElementBox &BuildBoxTree(RenderBlock &renderer) noexcept;

  /// @brief Builds the layout tree of the block flow with id @p id and returns its root box.
  KRYS_NODISCARD const ElementBox &RootBoxOf(style::tests::ResolverFixture &fixture,
                                             dom::DOMStringView id) noexcept;
}
