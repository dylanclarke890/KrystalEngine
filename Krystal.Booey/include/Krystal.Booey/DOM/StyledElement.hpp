#pragma once

#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleProperties;
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

  protected:
    void OnAttributeChanged(DOMStringAtom localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, DOMStringAtom namespaceURI) noexcept override;

  private:
    /// @brief Writes the serialised inline style to the `style` attribute and clears the dirty flag.
    void SynchronizeStyleAttributeInternal() noexcept;

    void StyleAttributeChanged(Maybe<DOMStringView> newStyleString) noexcept;
    void InlineStyleChanged() noexcept;
    void SetInlineStyleFromString(DOMStringView newStyleString) noexcept;
    KRYS_NODISCARD Ref<css::MutableStyleProperties> EnsureMutableInlineStyle() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::StyledElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsStyledElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
