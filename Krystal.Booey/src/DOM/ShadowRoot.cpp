#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheetList.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom
{
  ShadowRoot::ShadowRoot(Document &document, RefPtr<html::CustomElementRegistry> &&registry,
                         NodeFlags flags) noexcept
      : DocumentFragment(document, flags | NodeFlags::IsShadowRoot),
        DocumentOrShadowRoot(*this, krys::move(registry))
  {
    SetNodeFlag(NodeFlags::IsInShadowTree);
    _styleScope = CreateUnique<style::Scope>(*this);
  }

  ShadowRoot::~ShadowRoot() noexcept
  {
    if (_styleSheetList)
    {
      _styleSheetList->Detach();
    }
  }

#pragma region DocumentOrShadowRoot Mixin (CSSOM)

  css::StyleSheetList &ShadowRoot::StyleSheets() noexcept
  {
    if (!_styleSheetList)
    {
      _styleSheetList = css::StyleSheetList::Create(*this);
    }
    return *_styleSheetList;
  }

#pragma endregion

  Element *ShadowRoot::Host() const noexcept
  {
    if (auto host = _host.lock())
    {
      return host.get();
    }

    return nullptr;
  }

#pragma region Extension Hooks

  void ShadowRoot::OnInsert() noexcept
  {
    DocumentFragment::OnInsert();
    if (IsConnected())
    {
      NodeDocument().DidInsertInDocumentShadowRoot(*this);
    }
    if (!GetAdoptedStyleSheets().empty())
    {
      StyleScope().DidChangeActiveStyleSheetCandidates();
    }
  }

  void ShadowRoot::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    DocumentFragment::OnRemove(isSubtreeRoot, oldAncestor);
    NodeDocument().DidRemoveInDocumentShadowRoot(*this);
  }

  void ShadowRoot::OnAdopt(Document &oldDocument) noexcept
  {
    DocumentFragment::OnAdopt(oldDocument);

    // Adopted sheets were constructed for the old document.
    (void)SetAdoptedStyleSheets({});

    // Style scopes are document specific.
    _styleScope = CreateUnique<style::Scope>(*this);
  }

#pragma endregion

  EventTarget *ShadowRoot::GetParent(Event &event) const noexcept
  {
    if (!event.Composed())
    {
      if (event._path.size() >= 1)
      {
        auto &firstItem = event._path[0];
        if (auto *invocationTargetNode = DynamicDowncast<Node>(firstItem.InvocationTarget()))
        {
          if (&TreeQueries::Root(*invocationTargetNode) == this)
          {
            return nullptr;
          }
        }
      }
    }

    if (auto host = _host.lock())
    {
      return host.get();
    }

    return nullptr;
  }
}
