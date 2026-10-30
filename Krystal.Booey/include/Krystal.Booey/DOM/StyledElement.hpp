#pragma once

#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleProperties;
  class ImmutableStyleProperties;
  class MutableStyleProperties;
  class StyleProperties;
  class Value;

  enum class PropertyId : uint16;
  enum class UnitType : uint8;
  enum class ValueId : uint16;
}

namespace krys::boo::dom
{
  /// @brief An element with an inline style: an element that includes `ElementCSSInlineStyle` (HTML, SVG and
  /// MathML elements). The inline style is parsed from the `style` attribute, and changes made through
  /// `Style()` are written back to the attribute lazily.
  /// @see https://drafts.csswg.org/cssom/#the-elementcssinlinestyle-mixin
  class StyledElement : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(StyledElement);

    friend class Element;

  private:
    RefPtr<css::StyleProperties> _inlineStyle;
    RefPtr<const css::ImmutableStyleProperties> _presentationalHintStyle;
    bool _presentationalHintStyleIsDirty {false};

  protected:
    StyledElement(Document &document, const QualifiedName &name, NodeFlags nodeFlags) noexcept;

  public:
    ~StyledElement() noexcept;

#pragma region ElementCSSInlineStyle - https://drafts.csswg.org/cssom/#the-elementcssinlinestyle-mixin

    /// @brief The CSSOM view of the inline style.
    /// @see https://drafts.csswg.org/cssom/#dom-elementcssinlinestyle-style
    KRYS_NODISCARD css::CSSStyleProperties &Style() noexcept;

#pragma endregion

    /// @brief The declaration block parsed from the `style` attribute, if there is one.
    KRYS_NODISCARD const css::StyleProperties *InlineStyle() const noexcept;

    /// @brief Marks the `style` attribute as out of date after a change to the inline style that the
    /// attribute's text doesn't reflect yet.
    void DirtyStyleAttribute() noexcept;

    /// @brief Marks the `style` attribute as out of date after the inline style's properties changed.
    void InvalidateStyleAttribute() noexcept;

    bool SetInlineStyleProperty(css::PropertyId property, css::ValueId identifier,
                                css::IsImportant important = css::IsImportant(false)) noexcept;
    bool SetInlineStyleProperty(css::PropertyId property, css::PropertyId identifier,
                                css::IsImportant important = css::IsImportant(false)) noexcept;
    bool SetInlineStyleProperty(css::PropertyId property, double value, css::UnitType unit,
                                css::IsImportant important = css::IsImportant(false)) noexcept;
    bool SetInlineStyleProperty(css::PropertyId property, const css::CSSOMString &value,
                                css::IsImportant important = css::IsImportant(false),
                                bool *didFailParsing = nullptr) noexcept;
    bool SetInlineStyleProperty(css::PropertyId property, Ref<css::Value> &&value,
                                css::IsImportant important = css::IsImportant(false)) noexcept;
    bool SetInlineStyleCustomProperty(const css::CSSOMStringAtom &property, const css::CSSOMString &value,
                                      css::IsImportant important = css::IsImportant(false)) noexcept;
    bool SetInlineStyleCustomProperty(Ref<css::Value> &&customPropertyValue,
                                      css::IsImportant important = css::IsImportant(false)) noexcept;
    bool RemoveInlineStyleProperty(css::PropertyId property) noexcept;
    bool RemoveInlineStyleCustomProperty(const css::CSSOMStringAtom &property) noexcept;
    void RemoveAllInlineStyleProperties() noexcept;

    /// @brief The declarations the element's presentational attributes map to, rebuilt when one changes.
    /// @see https://html.spec.whatwg.org/#presentational-hints
    KRYS_NODISCARD const css::ImmutableStyleProperties *PresentationalHintStyle() const noexcept;

    /// @brief Presentational hints that depend on several attributes or on other elements (tables and their
    /// parts share them), applied after the element's own.
    KRYS_NODISCARD virtual const css::MutableStyleProperties *
      AdditionalPresentationalHintStyle() const noexcept
    {
      return nullptr;
    }

  protected:
    /// @brief Adds the declarations the attribute maps to, if it's a presentational attribute of the element.
    virtual void CollectPresentationalHintsForAttribute(DOMStringAtom localName, DOMStringAtom namespaceURI,
                                                        DOMStringView value,
                                                        css::MutableStyleProperties &style) noexcept
    {
      (void)localName;
      (void)namespaceURI;
      (void)value;
      (void)style;
    }

    /// @brief Adds the declarations that don't come from a single attribute, after the attributes' own.
    virtual void CollectExtraStyleForPresentationalHints(css::MutableStyleProperties &style) noexcept
    {
      (void)style;
    }

    /// @brief Whether a change to the attribute changes the element's presentational hints.
    KRYS_NODISCARD virtual bool HasPresentationalHintsForAttribute(DOMStringAtom localName,
                                                                   DOMStringAtom namespaceURI) const noexcept
    {
      (void)localName;
      (void)namespaceURI;
      return false;
    }

    void AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                              css::ValueId identifier) noexcept;
    void AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                              double value, css::UnitType unit) noexcept;
    void AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                              const css::CSSOMString &value) noexcept;
    void AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                              Ref<css::Value> &&value) noexcept;

    /// @brief Marks the presentational hints out of date, for a change that isn't an attribute change.
    void InvalidatePresentationalHintStyle() noexcept;

    void OnAttributeChanged(DOMStringAtom localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, DOMStringAtom namespaceURI) noexcept override;

  private:
    /// @brief Writes the serialised inline style to the `style` attribute and clears the dirty flag.
    void SynchronizeStyleAttributeInternal() noexcept;

    /// @brief Writes the inline style back to the `style` attribute now, invalidating the style of the rules
    /// that select on it.
    void SynchronizeStyleAttributeWithInvalidation() noexcept;

    void StyleAttributeChanged(Maybe<DOMStringView> newStyleString) noexcept;
    void InlineStyleChanged() noexcept;
    void SetInlineStyleFromString(DOMStringView newStyleString) noexcept;
    KRYS_NODISCARD Ref<css::MutableStyleProperties> EnsureMutableInlineStyle() noexcept;
    void RebuildPresentationalHintStyle() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::StyledElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsStyledElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
