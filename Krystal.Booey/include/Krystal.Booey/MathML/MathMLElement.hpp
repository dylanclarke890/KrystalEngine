#pragma once

#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/Mixins/HTMLOrSVGOrMathMLElement.hpp"
#include "Krystal.Booey/MathML/Enums/MathMLElementInterface.hpp"

namespace krys::boo::mathml
{
  /// @see https://w3c.github.io/mathml-core/#dom-and-javascript
  /// @note Subclasses react to their attributes in `AttributeChanged`, then call their base's.
  class MathMLElement : public dom::StyledElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLElement);

  private:
    MathMLElementInterface _interface {MathMLElementInterface::None};

  public:
    explicit MathMLElement(dom::Document &document,
                           MathMLElementInterface interface = MathMLElementInterface::None,
                           dom::NodeFlags nodeFlags = dom::NodeFlags::None) noexcept;

    /// @brief The element's interface, which its tag name selects.
    KRYS_NODISCARD MathMLElementInterface Interface() const noexcept
    {
      return _interface;
    }

    KRYS_NODISCARD bool HasTagName(html::TagName name) const noexcept
    {
      return TagQualifiedName().TagName() == name;
    }

    /// @brief The number of columns an `<mtd>` spans; 1 for other elements.
    KRYS_NODISCARD uint32 ColSpan() const noexcept;
    /// @brief The number of rows an `<mtd>` spans; 1 for other elements.
    KRYS_NODISCARD uint32 RowSpan() const noexcept;

    KRYS_NODISCARD virtual bool IsMathMLToken() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsPresentationMathML() const noexcept
    {
      return false;
    }

    // MathML lengths (https://www.w3.org/TR/MathML3/chapter2.html#fund.units)
    // TeX's Math Unit is used internally for named spaces (1 mu = 1/18 em).
    // Unitless values are interpreted as a multiple of a reference value.
    enum class LengthType : uint8
    {
      Cm,
      Em,
      Ex,
      In,
      MathUnit,
      Mm,
      ParsingFailed,
      Pc,
      Percentage,
      Pt,
      Px,
      UnitLess,
    };

    struct Length
    {
      LengthType Type {LengthType::ParsingFailed};
      float Value {0};

      KRYS_NODISCARD bool operator==(const Length &) const noexcept = default;
    };

    enum class BooleanValue : uint8
    {
      True,
      False,
      Default,
    };

#pragma region GlobalEventHandlers, DocumentAndElementEventHandlers - https://html.spec.whatwg.org/#event-handler-attributes

    KRYS_GLOBAL_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)
    KRYS_DOCUMENT_AND_ELEMENT_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

#pragma region HTMLOrSVGOrMathMLElement Mixin - https://html.spec.whatwg.org/#htmlorsvgormathmlelement

    /// @see https://html.spec.whatwg.org/#dom-tabindex
    KRYS_NODISCARD int32 TabIndex() const noexcept
    {
      return html::mixins::HTMLOrSVGOrMathMLElement::TabIndex(*this);
    }

    /// @see https://html.spec.whatwg.org/#dom-tabindex
    void TabIndex(int32 value) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::TabIndex(*this, value);
    }

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    KRYS_NODISCARD bool Autofocus() const noexcept
    {
      return html::mixins::HTMLOrSVGOrMathMLElement::Autofocus(*this);
    }

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    void Autofocus(bool value) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Autofocus(*this, value);
    }

    /// @see https://html.spec.whatwg.org/#dom-focus
    void Focus(const dom::FocusOptions &options = {}) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Focus(*this, options);
    }

    /// @see https://html.spec.whatwg.org/#dom-blur
    void Blur() noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Blur(*this);
    }

#pragma endregion

  protected:
    /// @brief The attribute change steps of MathML elements, run for every change to one of the element's
    /// attributes. Overrides drop what they cached from the attribute, then call their base's.
    /// @param oldValue The previous value, or null if the attribute was added.
    /// @param value The new value, or null if the attribute was removed.
    virtual void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                  Maybe<dom::DOMStringView> value) noexcept;

    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &child) const noexcept override;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

    /// @brief The value of the attribute @p name, or null if the element doesn't have it.
    KRYS_NODISCARD Maybe<dom::DOMString> AttributeValue(const dom::QualifiedName &name) const noexcept;

  private:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value, dom::DOMStringAtom namespaceURI) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsMathMLElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
