#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheetObservableArray.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
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
    // TODO(DOCUMENTORSHADOWROOT, HTML): return the active element.
    return nullptr;
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
