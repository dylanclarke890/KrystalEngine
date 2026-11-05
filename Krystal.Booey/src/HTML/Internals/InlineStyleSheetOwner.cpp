#include "Krystal.Booey/HTML/Internals/InlineStyleSheetOwner.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Style/Rules/StyleSheetContentsCache.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <utility>

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD css::ParserContext ParserContextForElement(const dom::Element &element) noexcept
    {
      auto &document = element.NodeDocument();
      return css::ParserContext(document, document.BaseURL(), u8"UTF-8");
    }

    KRYS_NODISCARD dom::DOMString Title(const dom::Element &element) noexcept
    {
      return element.GetAttribute(u8"title").value_or(u8"");
    }

    // https://html.spec.whatwg.org/multipage/semantics.html#update-a-style-block
    KRYS_NODISCARD bool IsValidCSSContentType(dom::DOMStringAtom type) noexcept
    {
      if (type.View().empty())
      {
        return true;
      }
      return text::IsASCIICaselessEqual(type.View(), dom::DOMStringView(u8"text/css"));
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

  css::CSSStyleSheet *InlineStyleSheetOwner::Sheet() const noexcept
  {
    return _sheet.get();
  }

  style::Scope *InlineStyleSheetOwner::StyleScope() noexcept
  {
    return _styleScope.get();
  }

  void InlineStyleSheetOwner::InsertedIntoDocument(dom::Element &element) noexcept
  {
    auto &scope = style::Scope::ForNode(element);
    _styleScope = CreateWeakPtr(&scope);
    scope.AddStyleSheetCandidateNode(element, !element.IsFinishedParsingChildren());

    if (!element.IsFinishedParsingChildren())
    {
      return;
    }
    CreateSheetFromTextContents(element);
  }

  void InlineStyleSheetOwner::RemovedFromDocument(dom::Element &element) noexcept
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

  void InlineStyleSheetOwner::ClearDocumentData(dom::Element &element) noexcept
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

  void InlineStyleSheetOwner::ChildrenChanged(dom::Element &element) noexcept
  {
    if (!element.IsFinishedParsingChildren())
    {
      return;
    }
    if (!element.IsConnected())
    {
      return;
    }
    CreateSheetFromTextContents(element);
  }

  void InlineStyleSheetOwner::FinishParsingChildren(dom::Element &element) noexcept
  {
    if (element.IsConnected())
    {
      CreateSheetFromTextContents(element);
    }
  }

  void InlineStyleSheetOwner::CreateSheetFromTextContents(dom::Element &element) noexcept
  {
    CreateSheet(element, dom::TextAlgorithms::ChildTextContent(element));
  }

  void InlineStyleSheetOwner::ClearSheet() noexcept
  {
    krys_debug_assert(_sheet);
    auto sheet = std::exchange(_sheet, nullptr);
    sheet->ClearOwnerNode();
  }

  void InlineStyleSheetOwner::CreateSheet(dom::Element &element, const dom::DOMString &text) noexcept
  {
    // The update a style block algorithm (from step 2; the callers check step 3, that it's connected):
    // https://html.spec.whatwg.org/#update-a-style-block
    // 1. Let element be the style element.
    krys_debug_assert(element.IsConnected());
    auto &document = element.NodeDocument();

    // 2. If element has an associated CSS style sheet, remove the CSS style sheet in question.
    if (_sheet)
    {
      if (_sheet->IsLoading() && _styleScope)
      {
        _styleScope->RemovePendingSheet(element);
      }
      ClearSheet();
    }

    // 3. If element is not connected, then return.
    // 4. If element's type attribute is present and its value is neither the empty string nor an ASCII
    //    case-insensitive match for "text/css", then return.
    if (!IsValidCSSContentType(_contentType))
    {
      return;
    }

    // 5. If the Should element's inline behavior be blocked by Content Security Policy? algorithm returns
    //    "Blocked" when executed upon element, "style", and element's child text content, then return.
    // SPEC-VIOLATION(CSP): the sheet is created without asking the Content Security Policy whether inline
    // styles are allowed.

    // 6. Create a CSS style sheet with the following properties: type text/css; owner node element; media
    //    the media attribute of element; title the title attribute of element, if element is in a document
    //    tree, or the empty string otherwise; alternate flag unset; origin-clean flag set; location, parent
    //    CSS style sheet and owner CSS rule null; disabled flag left at its default value; CSS rules left
    //    uninitialized.
    // 7. If element contributes a script-blocking style sheet, append element to its node document's
    //    script-blocking style sheet set. (Skipped: nothing waits for scripts.)
    // 8. If element's media attribute's value matches the environment and element is potentially
    //    render-blocking, then block rendering on element.
    // FIXME: Blocking rendering, once render-blocked documents aren't rendered.

    auto mediaQueries = css::mq::MediaQueryParser::Parse(_media.View(), document.CssParserContext());

    if (auto *scope = _styleScope.get())
    {
      scope->AddPendingSheet(element);
    }

    style::StyleSheetContentsCache::Key cacheKey {text, ParserContextForElement(element)};
    if (auto cachedSheet = style::StyleSheetContentsCache::Singleton().Get(cacheKey))
    {
      krys_debug_assert(cachedSheet->IsCacheableWithNoBaseURLDependency());
      auto sheet = css::CSSStyleSheet::CreateInline(cachedSheet, element);
      _sheet = sheet;
      sheet->SetMediaQueries(krys::move(mediaQueries));
      if (!dom::TreeQueries::IsInShadowTree(element))
      {
        sheet->SetTitle(Title(element));
      }

      SheetLoaded(element);
      element.NotifyLoadedSheetAndAllCriticalSubresources(false);
      return;
    }

    _loading = true;

    auto contents = css::StyleSheetContents::Create(cacheKey.second);

    auto sheet = css::CSSStyleSheet::CreateInline(Ref<css::StyleSheetContents>(contents), element);
    _sheet = sheet;
    sheet->SetMediaQueries(krys::move(mediaQueries));
    if (!dom::TreeQueries::IsInShadowTree(element))
    {
      sheet->SetTitle(Title(element));
    }

    contents->ParseString(text);

    _loading = false;

    contents->CheckLoaded();

    if (contents->IsCacheableWithNoBaseURLDependency())
    {
      style::StyleSheetContentsCache::Singleton().Add(krys::move(cacheKey), contents);
    }
  }

  bool InlineStyleSheetOwner::IsLoading() const noexcept
  {
    if (_loading)
    {
      return true;
    }
    return _sheet && _sheet->IsLoading();
  }

  bool InlineStyleSheetOwner::SheetLoaded(dom::Element &element) noexcept
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

  void InlineStyleSheetOwner::StartLoadingDynamicSheet(dom::Element &element) noexcept
  {
    if (auto *scope = _styleScope.get(); scope && !scope->HasPendingSheet(element))
    {
      scope->AddPendingSheet(element);
    }
  }
}
