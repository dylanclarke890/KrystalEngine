#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::dom
{
  DocumentOrShadowRoot::DocumentOrShadowRoot(RefPtr<html::CustomElementRegistry> &&registry) noexcept
      : _customElementRegistry(krys::move(registry))
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
}
