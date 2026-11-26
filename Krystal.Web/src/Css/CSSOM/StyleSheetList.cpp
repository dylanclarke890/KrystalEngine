#include "Krystal.Web/Css/CSSOM/StyleSheetList.hpp"
#include "Krystal.Web/Css/CSSOM/StyleSheet.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"

namespace krys::web
{
  StyleSheetList::StyleSheetList(ContainerNode &owner) noexcept : _owner(CreateWeakPtr(&owner))
  {
  }

  StyleSheetList::~StyleSheetList() noexcept = default;

  Ref<StyleSheetList> StyleSheetList::Create(Document &document) noexcept
  {
    return AdoptRef(*new StyleSheetList(document));
  }

  Ref<StyleSheetList> StyleSheetList::Create(ShadowRoot &shadowRoot) noexcept
  {
    return AdoptRef(*new StyleSheetList(shadowRoot));
  }

  style::Scope *StyleSheetList::OwnerStyleScope() const noexcept
  {
    if (auto *document = DynamicDowncast<Document>(_owner.get()))
    {
      return &document->StyleScope();
    }
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(_owner.get()))
    {
      return &shadowRoot->StyleScope();
    }
    return nullptr;
  }

  const List<RefPtr<StyleSheet>> &StyleSheetList::StyleSheets() const noexcept
  {
    if (auto *scope = OwnerStyleScope())
    {
      return scope->StyleSheetsForStyleSheetList();
    }
    return _detachedStyleSheets;
  }

  StyleSheet *StyleSheetList::Item(uint32 index) const noexcept
  {
    const auto &sheets = StyleSheets();
    return index < sheets.size() ? sheets[index].get() : nullptr;
  }

  uint32 StyleSheetList::Length() const noexcept
  {
    return static_cast<uint32>(StyleSheets().size());
  }

  Node *StyleSheetList::OwnerNode() const noexcept
  {
    return _owner.get();
  }

  void StyleSheetList::Detach() noexcept
  {
    if (auto *scope = OwnerStyleScope())
    {
      _detachedStyleSheets = scope->StyleSheetsForStyleSheetList();
    }
    _owner = nullptr;
  }
}
