#include "Krystal.Booey/CSS/StyleSheetList.hpp"
#include "Krystal.Booey/CSS/StyleSheet.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  StyleSheetList::StyleSheetList(dom::Document &document) noexcept : _document(CreateWeakPtr(&document))
  {
  }

  StyleSheetList::StyleSheetList(dom::ShadowRoot &shadowRoot) noexcept
      : _shadowRoot(CreateWeakPtr(&shadowRoot))
  {
  }

  StyleSheetList::~StyleSheetList() noexcept = default;

  Ref<StyleSheetList> StyleSheetList::Create(dom::Document &document) noexcept
  {
    return AdoptRef(*new StyleSheetList(document));
  }

  Ref<StyleSheetList> StyleSheetList::Create(dom::ShadowRoot &shadowRoot) noexcept
  {
    return AdoptRef(*new StyleSheetList(shadowRoot));
  }

  const List<RefPtr<StyleSheet>> &StyleSheetList::StyleSheets() const noexcept
  {
    if (auto *document = _document.get())
    {
      return document->StyleScope().StyleSheetsForStyleSheetList();
    }
    if (auto *shadowRoot = _shadowRoot.get())
    {
      return shadowRoot->StyleScope().StyleSheetsForStyleSheetList();
    }
    return _detachedStyleSheets;
  }

  dom::Node *StyleSheetList::OwnerNode() const noexcept
  {
    if (auto *document = _document.get())
    {
      return document;
    }
    return _shadowRoot.get();
  }

  void StyleSheetList::Detach() noexcept
  {
    if (auto *document = _document.get())
    {
      krys_debug_assert(!_shadowRoot);
      _detachedStyleSheets = document->StyleScope().StyleSheetsForStyleSheetList();
      _document = nullptr;
    }
    else if (auto *shadowRoot = _shadowRoot.get())
    {
      krys_debug_assert(!_document);
      _detachedStyleSheets = shadowRoot->StyleScope().StyleSheetsForStyleSheetList();
      _shadowRoot = nullptr;
    }
    else
    {
      krys_unreachable();
    }
  }

  uint32 StyleSheetList::Length() const noexcept
  {
    return static_cast<uint32>(StyleSheets().size());
  }

  StyleSheet *StyleSheetList::Item(uint32 index) noexcept
  {
    auto &sheets = StyleSheets();
    return index < sheets.size() ? sheets[index].get() : nullptr;
  }
}
