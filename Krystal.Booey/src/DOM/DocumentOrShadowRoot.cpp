#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheetObservableArray.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::dom
{
  DocumentOrShadowRoot::DocumentOrShadowRoot(ContainerNode &rootNode,
                                             RefPtr<html::CustomElementRegistry> &&registry) noexcept
      : _rootNode(rootNode), _customElementRegistry(krys::move(registry))
  {
  }

  DocumentOrShadowRoot::~DocumentOrShadowRoot() noexcept = default;

#pragma region DocumentOrShadowRoot Mixin (DOM)

  RefPtr<html::CustomElementRegistry> DocumentOrShadowRoot::CustomElementRegistry() const noexcept
  {
    return _customElementRegistry;
  }

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (HTML)

  RefPtr<Element> DocumentOrShadowRoot::ActiveElement() const noexcept
  {
    // Let candidate be this's node document's focused area's DOM anchor.
    auto &document = _rootNode.NodeDocument();
    Node *candidate = document.FocusedElement();
    if (!candidate)
    {
      candidate = &document;
    }

    // Set candidate to the result of retargeting candidate against this.
    auto *retargeted = DynamicDowncast<Node>(EventTargetAlgorithms::Retarget(candidate, _rootNode));
    if (!retargeted)
    {
      return nullptr;
    }

    // If candidate's root is not this, then return null.
    if (&TreeQueries::Root(*retargeted) != &_rootNode)
    {
      return nullptr;
    }

    // If candidate is not a Document object, then return candidate.
    if (!Is<Document>(*retargeted))
    {
      return ShareRefPtr(Downcast<Element>(retargeted));
    }

    // If candidate has a body element, then return that body element.
    if (auto body = document.Body())
    {
      return body;
    }

    // If candidate's document element is non-null, then return that document element.
    return document.DocumentElement();
  }

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (CSSOM)

  css::CSSStyleSheetObservableArray &DocumentOrShadowRoot::AdoptedStyleSheets() noexcept
  {
    if (!_adoptedStyleSheets) KRYS_UNLIKELY
    {
      _adoptedStyleSheets = CreateUnique<css::CSSStyleSheetObservableArray>(_rootNode);
    }
    return *_adoptedStyleSheets;
  }

  ExceptionOr<void>
    DocumentOrShadowRoot::SetAdoptedStyleSheets(List<Ref<css::CSSStyleSheet>> &&sheets) noexcept
  {
    if (!_adoptedStyleSheets && sheets.empty())
    {
      return {};
    }
    return AdoptedStyleSheets().SetSheets(krys::move(sheets));
  }

#pragma endregion

  Span<const Ref<css::CSSStyleSheet>> DocumentOrShadowRoot::GetAdoptedStyleSheets() const noexcept
  {
    if (!_adoptedStyleSheets)
    {
      return {};
    }
    return _adoptedStyleSheets->Sheets();
  }
}
