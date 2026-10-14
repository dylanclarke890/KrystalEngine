#pragma once

#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::html
{
  class CustomElementAlgorithms;
  class CustomElementRegistry;
}

namespace krys::boo::style
{
  class Scope;
}

namespace krys::boo::dom
{
  class DocumentAlgorithms;
  class Element;
  class NodeAlgorithms;

  /// @brief The state and members shared by documents and shadow roots, the roots of node trees.
  /// @see https://dom.spec.whatwg.org/#mixin-documentorshadowroot
  /// @see https://html.spec.whatwg.org/#documentorshadowroot
  class DocumentOrShadowRoot
  {
    friend class DocumentAlgorithms;
    friend class NodeAlgorithms;

    friend class html::CustomElementAlgorithms;

    KRYS_NON_COPY_MOVABLE(DocumentOrShadowRoot);

  private:
    RefPtr<html::CustomElementRegistry> _customElementRegistry;

  protected:
    /// @brief Created by the document or shadow root's constructor.
    UniquePtr<style::Scope> _styleScope;

    explicit DocumentOrShadowRoot(RefPtr<html::CustomElementRegistry> &&registry = nullptr) noexcept;
    ~DocumentOrShadowRoot() noexcept;

  public:
#pragma region DocumentOrShadowRoot Mixin (DOM) - https://dom.spec.whatwg.org/#mixin-documentorshadowroot

    /// @brief Returns node's CustomElementRegistry object, if any; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-documentorshadowroot-customelementregistry
    KRYS_NODISCARD RefPtr<html::CustomElementRegistry> CustomElementRegistry() const noexcept;

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (HTML) - https://html.spec.whatwg.org/#documentorshadowroot

    /// @brief Returns node's active element, if any; otherwise null.
    /// @see https://html.spec.whatwg.org/#dom-documentorshadowroot-activeelement
    KRYS_NODISCARD RefPtr<Element> ActiveElement() const noexcept;

#pragma endregion

    /// @brief The style sheet bookkeeping of this document or shadow root.
    KRYS_NODISCARD style::Scope &StyleScope() const noexcept
    {
      return *_styleScope;
    }
  };
}
