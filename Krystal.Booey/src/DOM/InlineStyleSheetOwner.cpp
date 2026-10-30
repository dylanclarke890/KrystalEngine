#include "Krystal.Booey/DOM/InlineStyleSheetOwner.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/StyleSheetContentsCache.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <utility>

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD css::ParserContext ParserContextForElement(const Element &element) noexcept
    {
      auto &document = element.NodeDocument();
      return css::ParserContext(document, document.BaseURL(), u8"UTF-8");
    }

    KRYS_NODISCARD DOMString Title(const Element &element) noexcept
    {
      return element.GetAttribute(u8"title").value_or(u8"");
    }

    // https://html.spec.whatwg.org/multipage/semantics.html#update-a-style-block
    KRYS_NODISCARD bool IsValidCSSContentType(DOMStringAtom type) noexcept
    {
      if (type.View().empty())
      {
        return true;
      }
      return text::IsASCIICaselessEqual(type.View(), DOMStringView(u8"text/css"));
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

  void InlineStyleSheetOwner::InsertedIntoDocument(Element &element) noexcept
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

  void InlineStyleSheetOwner::FinishParsingChildren(Element &element) noexcept
  {
    if (element.IsConnected())
    {
      CreateSheetFromTextContents(element);
    }
  }

  void InlineStyleSheetOwner::CreateSheetFromTextContents(Element &element) noexcept
  {
    CreateSheet(element, TextAlgorithms::ChildTextContent(element));
  }

  void InlineStyleSheetOwner::ClearSheet() noexcept
  {
    krys_debug_assert(_sheet);
    auto sheet = std::exchange(_sheet, nullptr);
    sheet->ClearOwnerNode();
  }

  void InlineStyleSheetOwner::CreateSheet(Element &element, const DOMString &text) noexcept
  {
    krys_debug_assert(element.IsConnected());
    auto &document = element.NodeDocument();
    if (_sheet)
    {
      if (_sheet->IsLoading() && _styleScope)
      {
        _styleScope->RemovePendingSheet(element);
      }
      ClearSheet();
    }

    if (!IsValidCSSContentType(_contentType))
    {
      return;
    }

    // SPEC-VIOLATION(CSP): the sheet is created without asking the Content Security Policy whether inline
    // styles are allowed.

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
      if (!TreeQueries::IsInShadowTree(element))
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
    if (!TreeQueries::IsInShadowTree(element))
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
}
