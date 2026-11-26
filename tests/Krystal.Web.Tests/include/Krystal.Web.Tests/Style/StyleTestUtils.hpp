#pragma once

#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Dom/DOMTestUtils.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "Krystal.Web/Css/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Html/Elements/HTMLStyleElement.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"

namespace krys::web::style::tests
{
  inline const web::Color Blue {web::Color::blue};

  /// @brief @p string as an atom.
  KRYS_NODISCARD inline CSSOMStringAtom Atom(utf8_stringview string) noexcept
  {
    return CSSOMStringAtom(string);
  }

  /// @brief A style sheet parsed from @p text in standards mode.
  KRYS_NODISCARD inline Ref<css::StyleSheetContents> ParseSheet(utf8_stringview text)
  {
    auto sheet = css::StyleSheetContents::Create(css::ParserContext(css::ParserMode::HTMLStandard));
    sheet->ParseString(CSSOMString(text));
    return sheet;
  }

  /// @brief A document parsed from @p markup, with its style sheets collected.
  KRYS_NODISCARD inline Ref<HTMLDocument> ParseWithStyleSheets(utf8_string markup) noexcept
  {
    auto document = web::tests::ParseDocument(krys::move(markup));
    document->StyleScope().FlushPendingUpdate();
    return document;
  }

  /// @brief The style @p element was rendered with, which it must have.
  KRYS_NODISCARD inline const RenderStyle &Rendered(const Element &element)
  {
    auto *style = element.GetRenderStyle();
    REQUIRE(style);
    return *style;
  }

  /// @brief Whether @p element was rendered blue.
  KRYS_NODISCARD inline bool IsBlue(const Element &element)
  {
    return Rendered(element).Color() == Blue;
  }

  /// @brief A new style element of @p document with @p text as its sheet.
  KRYS_NODISCARD inline Ref<HTMLStyleElement> CreateStyle(Document &document, DOMString text)
  {
    auto style = CreateRef<HTMLStyleElement>(document);
    REQUIRE_FALSE(style->TextContent(krys::move(text)).HasException());
    return style;
  }
}
