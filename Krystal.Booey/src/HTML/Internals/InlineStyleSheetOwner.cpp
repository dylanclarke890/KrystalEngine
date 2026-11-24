#include "Krystal.Booey/HTML/Internals/InlineStyleSheetOwner.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLStyleElement.hpp"
#include "Krystal.Booey/Style/Rules/StyleSheetContentsCache.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include <utility>

namespace krys::boo::html
{
  namespace
  {
    /// @brief The context a style element's sheet is parsed with: the document's, with its base URL.
    KRYS_NODISCARD css::ParserContext InlineSheetParserContext(const Element &element) noexcept
    {
      auto &document = element.NodeDocument();
      return css::ParserContext(document, document.BaseUrl(), u8"UTF-8");
    }

    /// @brief Gives @p sheet the media and title of its style element, @p element.
    void SetUpInlineSheet(CSSStyleSheet &sheet, const Element &element,
                          css::mq::MediaQueryList mediaQueries) noexcept
    {
      sheet.SetMediaQueries(krys::move(mediaQueries));
      // Only a sheet in a document tree takes the element's title.
      if (!dom::TreeQueries::IsInShadowTree(element))
      {
        sheet.SetTitle(element.GetAttribute(u8"title").value_or(u8""));
      }
    }
  }

  InlineStyleSheetOwner::InlineStyleSheetOwner() noexcept = default;

  InlineStyleSheetOwner::~InlineStyleSheetOwner() noexcept
  {
    if (_sheet)
    {
      ClearSheet();
    }
  }

  CSSStyleSheet *InlineStyleSheetOwner::Sheet() const noexcept
  {
    return _sheet.get();
  }

  bool InlineStyleSheetOwner::IsLoading() const noexcept
  {
    return _loading || (_sheet && _sheet->IsLoading());
  }

  style::Scope *InlineStyleSheetOwner::StyleScope() noexcept
  {
    return _styleScope.get();
  }

#pragma region Loading

  bool InlineStyleSheetOwner::SheetLoaded(Element &element) noexcept
  {
    if (IsLoading())
    {
      return false;
    }
    if (auto *scope = _styleScope.get())
    {
      scope->RemovePendingSheet(element);
    }
    return true;
  }

  void InlineStyleSheetOwner::StartLoadingDynamicSheet(Element &element) noexcept
  {
    if (auto *scope = _styleScope.get(); scope && !scope->HasPendingSheet(element))
    {
      scope->AddPendingSheet(element);
    }
  }

#pragma endregion

#pragma region Element changes

  void InlineStyleSheetOwner::InsertedIntoDocument(Element &element) noexcept
  {
    auto &scope = style::Scope::ForNode(element);
    _styleScope = CreateWeakPtr(&scope);
    scope.AddStyleSheetCandidateNode(element, !element.IsFinishedParsingChildren());
    if (element.IsFinishedParsingChildren())
    {
      UpdateSheetFromChildText(element);
    }
  }

  void InlineStyleSheetOwner::RemovedFromDocument(Element &element) noexcept
  {
    if (auto *scope = _styleScope.get())
    {
      if (scope->HasPendingSheet(element))
      {
        scope->RemovePendingSheet(element);
      }
      scope->RemoveStyleSheetCandidateNode(element);
    }
    if (_sheet)
    {
      ClearSheet();
    }
  }

  void InlineStyleSheetOwner::ClearDocumentData(Element &element) noexcept
  {
    if (_sheet)
    {
      _sheet->ClearOwnerNode();
    }
    if (auto *scope = _styleScope.get())
    {
      scope->RemoveStyleSheetCandidateNode(element);
    }
  }

  void InlineStyleSheetOwner::ChildrenChanged(Element &element) noexcept
  {
    if (element.IsFinishedParsingChildren() && element.IsConnected())
    {
      UpdateSheetFromChildText(element);
    }
  }

  void InlineStyleSheetOwner::FinishParsingChildren(Element &element) noexcept
  {
    if (element.IsConnected())
    {
      UpdateSheetFromChildText(element);
    }
  }

#pragma endregion

#pragma region The sheet

  void InlineStyleSheetOwner::UpdateSheetFromChildText(Element &element) noexcept
  {
    UpdateSheet(element, dom::TextAlgorithms::ChildTextContent(element));
  }

  // https://html.spec.whatwg.org/#update-a-style-block
  void InlineStyleSheetOwner::UpdateSheet(Element &element, const DOMString &text) noexcept
  {
    // 1. Let element be the style element.
    krys_debug_assert(element.IsConnected());
    auto &document = element.NodeDocument();

    // 2. If element has an associated CSS style sheet, remove the CSS style sheet in question.
    // (Unblocking rendering on the element goes with it.)
    if (auto *styleElement = DynamicDowncast<HTMLStyleElement>(element))
    {
      styleElement->UnblockRendering();
    }
    if (_sheet)
    {
      if (_sheet->IsLoading() && _styleScope)
      {
        _styleScope->RemovePendingSheet(element);
      }
      ClearSheet();
    }

    // 3. If element is not connected, then return.
    // (The callers only update a connected element.)

    // 4. If element's type attribute is present and its value is neither the empty string nor an ASCII
    //    case-insensitive match for "text/css", then return.
    if (!_contentType.View().empty() && !text::IsASCIICaselessEqual(_contentType.View(), u8"text/css"))
    {
      return;
    }

    // 5. If the Should element's inline behavior be blocked by Content Security Policy? algorithm returns
    //    "Blocked" when executed upon element, "style", and element's child text content, then return.
    // SPEC-VIOLATION(CSP): there's no Content Security Policy to ask.

    // 6. Create a CSS style sheet with the following properties: type text/css; owner node element; media
    //    the media attribute of element; title the title attribute of element, if element is in a document
    //    tree, or the empty string otherwise; alternate flag unset; origin-clean flag set; location, parent
    //    CSS style sheet and owner CSS rule null; disabled flag left at its default value; CSS rules left
    //    uninitialized.
    //    (Made below, after step 8.)
    // 7. If element contributes a script-blocking style sheet, append element to its node document's
    //    script-blocking style sheet set. (Skipped: nothing waits for scripts.)
    // 8. If element's media attribute's value matches the environment and element is potentially
    //    render-blocking, then block rendering on element.
    auto mediaQueries = css::mq::MediaQueryParser::Parse(_media.View(), document.CssParserContext());
    if (auto *styleElement = DynamicDowncast<HTMLStyleElement>(element))
    {
      styleElement->PotentiallyBlockRendering(mediaQueries);
    }

    if (auto *scope = _styleScope.get())
    {
      scope->AddPendingSheet(element);
    }

    // Another style element with the same text and context may have parsed it already.
    style::StyleSheetContentsCache::Key cacheKey {text, InlineSheetParserContext(element)};
    if (auto cachedContents = style::StyleSheetContentsCache::Singleton().Get(cacheKey))
    {
      krys_debug_assert(cachedContents->IsCacheableWithNoBaseUrlDependency());
      auto sheet = CSSStyleSheet::CreateInline(cachedContents, element);
      _sheet = sheet;
      SetUpInlineSheet(*sheet, element, krys::move(mediaQueries));

      SheetLoaded(element);
      element.NotifyLoadedSheetAndAllCriticalSubresources(false);
      return;
    }

    _loading = true;
    auto contents = css::StyleSheetContents::Create(cacheKey.second);
    auto sheet = CSSStyleSheet::CreateInline(Ref<css::StyleSheetContents>(contents), element);
    _sheet = sheet;
    SetUpInlineSheet(*sheet, element, krys::move(mediaQueries));
    contents->ParseString(text);
    _loading = false;

    // With no imports it's loaded now; otherwise once they are.
    contents->CheckLoaded();

    if (contents->IsCacheableWithNoBaseUrlDependency())
    {
      style::StyleSheetContentsCache::Singleton().Add(krys::move(cacheKey), contents);
    }
  }

  void InlineStyleSheetOwner::ClearSheet() noexcept
  {
    krys_debug_assert(_sheet);
    auto sheet = std::exchange(_sheet, nullptr);
    sheet->ClearOwnerNode();
  }

#pragma endregion
}
