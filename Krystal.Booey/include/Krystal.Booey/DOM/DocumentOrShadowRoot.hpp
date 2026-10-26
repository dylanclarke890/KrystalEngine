#pragma once

#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::html
{
  class CustomElementAlgorithms;
  class CustomElementRegistry;
}

namespace krys::boo::css
{
  class CSSStyleSheet;
  class CSSStyleSheetObservableArray;
}

namespace krys::boo::style
{
  class Scope;
}

namespace krys::boo::dom
{
  class ContainerNode;
  class DocumentAlgorithms;
  class Element;
  class IdTargetObserverRegistry;
  class RadioButtonGroups;
  class NodeAlgorithms;

  template <typename>
  class ExceptionOr;

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
    ContainerNode &_rootNode;
    RefPtr<html::CustomElementRegistry> _customElementRegistry;
    UniquePtr<css::CSSStyleSheetObservableArray> _adoptedStyleSheets;
    UniquePtr<dom::IdTargetObserverRegistry> _idTargetObserverRegistry;
    UniquePtr<dom::RadioButtonGroups> _radioButtonGroups;

  protected:
    /// @brief Created by the document or shadow root's constructor.
    UniquePtr<style::Scope> _styleScope;

    /// @param rootNode The document or shadow root itself.
    explicit DocumentOrShadowRoot(ContainerNode &rootNode,
                                  RefPtr<html::CustomElementRegistry> &&registry = nullptr) noexcept;
    ~DocumentOrShadowRoot() noexcept;

  public:
    /// @brief The document or shadow root itself.
    KRYS_NODISCARD ContainerNode &RootNode() const noexcept
    {
      return _rootNode;
    }

#pragma region DocumentOrShadowRoot Mixin (DOM) - https://dom.spec.whatwg.org/#mixin-documentorshadowroot

    /// @brief Returns node's CustomElementRegistry object, if any; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-documentorshadowroot-customelementregistry
    KRYS_NODISCARD RefPtr<html::CustomElementRegistry> CustomElementRegistry() const noexcept;

    void SetCustomElementRegistry(RefPtr<html::CustomElementRegistry> &&registry) noexcept;

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (HTML) - https://html.spec.whatwg.org/#documentorshadowroot

    /// @brief Returns node's active element, if any; otherwise null.
    /// @see https://html.spec.whatwg.org/#dom-documentorshadowroot-activeelement
    KRYS_NODISCARD RefPtr<Element> ActiveElement() const noexcept;

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (CSSOM) - https://drafts.csswg.org/cssom/#extensions-to-the-document-or-shadow-root-interface

    /// @brief The constructed style sheets this document or shadow root adopts, after its other style sheets.
    /// @see https://drafts.csswg.org/cssom/#dom-documentorshadowroot-adoptedstylesheets
    KRYS_NODISCARD css::CSSStyleSheetObservableArray &AdoptedStyleSheets() noexcept;

    /// @brief Replaces the adopted style sheets.
    /// @return A `NotAllowedError` if a sheet wasn't constructed, or was constructed for another document.
    /// @see https://drafts.csswg.org/cssom/#dom-documentorshadowroot-adoptedstylesheets
    ExceptionOr<void> SetAdoptedStyleSheets(List<Ref<css::CSSStyleSheet>> &&sheets) noexcept;

#pragma endregion

#pragma region DocumentOrShadowRoot Extensions (CSSOM View) - https://drafts.csswg.org/cssom-view/#extensions-to-the-document-interface

    /// @brief The topmost element at the point (in CSS pixels, relative to the viewport), retargeted against
    /// this; null if the point is outside the viewport or hits nothing.
    /// @see https://drafts.csswg.org/cssom-view/#dom-document-elementfrompoint
    KRYS_NODISCARD RefPtr<Element> ElementFromPoint(double clientX, double clientY,
                                                    HitTestSource source = HitTestSource::User) noexcept;

    /// @brief Every element at the point, topmost first, retargeted against this.
    /// @see https://drafts.csswg.org/cssom-view/#dom-document-elementsfrompoint
    KRYS_NODISCARD List<RefPtr<Element>>
      ElementsFromPoint(double clientX, double clientY, HitTestSource source = HitTestSource::User) noexcept;

#pragma endregion

#pragma region DocumentOrShadowRoot Extensions (Pointer Lock) - https://w3c.github.io/pointerlock/#extensions-to-the-documentorshadowroot-mixin

    /// @brief The element the pointer is locked to, retargeted against this; null if there's none, it's
    /// still being locked, or it's in another document.
    /// @see https://w3c.github.io/pointerlock/#dom-documentorshadowroot-pointerlockelement
    KRYS_NODISCARD RefPtr<Element> PointerLockElement() const noexcept;

#pragma endregion

    /// @brief The node hit at @p clientPoint (in CSS pixels, relative to the viewport), if it's inside the
    /// viewport.
    /// @param localPoint Set to the point in the coordinates of the hit node's renderer.
    KRYS_NODISCARD RefPtr<Node> NodeFromPoint(const LayoutPoint &clientPoint,
                                              LayoutPoint *localPoint = nullptr,
                                              HitTestSource source = HitTestSource::User) noexcept;

    /// @brief The adopted style sheets, without creating the array.
    KRYS_NODISCARD Span<const Ref<css::CSSStyleSheet>> GetAdoptedStyleSheets() const noexcept;

    /// @brief The observers of the IDs in this tree.
    KRYS_NODISCARD dom::IdTargetObserverRegistry &IdTargetObserverRegistry() const noexcept
    {
      return *_idTargetObserverRegistry;
    }

    /// @brief The groups of the radio buttons in this tree that have no form owner.
    KRYS_NODISCARD dom::RadioButtonGroups &RadioButtonGroups() const noexcept
    {
      return *_radioButtonGroups;
    }

    /// @brief The style sheet bookkeeping of this document or shadow root.
    KRYS_NODISCARD style::Scope &StyleScope() const noexcept
    {
      return *_styleScope;
    }
  };
}
