#pragma once

#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo::css
{
  class ImmutableStyleProperties;
  class MutableStyleProperties;
  class StyleProperties;
  class Value;
  enum class PropertyId : uint16;
  enum class UnitType : uint8;
  enum class ValueId : uint16;
}

namespace krys::boo
{
  class CSSStyleProperties;
  class StylePropertyMap;

  /// @brief An element with an inline style: one that includes `ElementCSSInlineStyle` (HTML, SVG and
  /// MathML elements). It also gathers the style its presentational attributes map to.
  /// @note The inline style is parsed from the `style` attribute. A change made through `Style()`,
  /// `AttributeStyleMap()` or the setters below only marks the attribute out of date; its text is written
  /// when the attribute is next read, or straight away if a selector that tests `style` needs it.
  /// @see https://drafts.csswg.org/cssom/#the-elementcssinlinestyle-mixin
  class StyledElement : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(StyledElement);

    friend class Element;

  private:
    /// @brief The declarations from the `style` attribute: immutable (and shareable) until something
    /// changes them, then mutable.
    RefPtr<css::StyleProperties> _inlineStyle;
    /// @brief The declarations the presentational attributes map to, or null if they map to none.
    RefPtr<const css::ImmutableStyleProperties> _presentationalHintStyle;
    /// @brief Whether `_presentationalHintStyle` needs rebuilding before it's next read.
    bool _presentationalHintStyleIsDirty {false};

  protected:
    StyledElement(Document &document, const QualifiedName &name, dom::NodeFlags nodeFlags) noexcept;

  public:
    ~StyledElement() noexcept override;

#pragma region ElementCSSInlineStyle - https://drafts.csswg.org/cssom/#the-elementcssinlinestyle-mixin

    /// @brief The CSSOM view of the inline style.
    /// @see https://drafts.csswg.org/cssom/#dom-elementcssinlinestyle-style
    KRYS_NODISCARD CSSStyleProperties &Style() noexcept;

    /// @brief The inline style, as a property map.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-elementcssinlinestyle-attributestylemap
    KRYS_NODISCARD StylePropertyMap &AttributeStyleMap() noexcept;

#pragma endregion

#pragma region Inline style

    /// @brief The inline style's declarations, or null if the element has none yet.
    KRYS_NODISCARD const css::StyleProperties *InlineStyle() const noexcept;

    /// @brief Marks the `style` attribute out of date after the CSSOM changed the inline style in a way that
    /// doesn't affect the element's style.
    void DirtyStyleAttribute() noexcept;

    /// @brief Marks the `style` attribute out of date after the inline style's declarations changed, and
    /// invalidates the element's style.
    void InvalidateStyleAttribute() noexcept;

    /// @brief Sets @p property to the keyword @p identifier.
    /// @return true.
    bool SetInlineStyleProperty(css::PropertyId property, css::ValueId identifier,
                                css::IsImportant important = css::IsImportant::No) noexcept;

    /// @brief Sets @p property to @p value in @p unit.
    /// @return true.
    bool SetInlineStyleProperty(css::PropertyId property, double value, css::UnitType unit,
                                css::IsImportant important = css::IsImportant::No) noexcept;

    /// @brief Sets @p property to @p value, parsed in the document's context.
    /// @param didFailParsing Set to whether @p value failed to parse, if given.
    /// @return Whether the inline style changed.
    bool SetInlineStyleProperty(css::PropertyId property, const CSSOMString &value,
                                css::IsImportant important = css::IsImportant::No,
                                bool *didFailParsing = nullptr) noexcept;

    /// @brief Sets @p property to the already parsed @p value.
    /// @return true.
    bool SetInlineStyleProperty(css::PropertyId property, Ref<css::Value> &&value,
                                css::IsImportant important = css::IsImportant::No) noexcept;

    /// @brief Sets a custom property to @p customPropertyValue, a `CustomPropertyValue` naming it.
    /// @return true.
    bool SetInlineStyleCustomProperty(Ref<css::Value> &&customPropertyValue,
                                      css::IsImportant important = css::IsImportant::No) noexcept;

    /// @return Whether the inline style had @p property.
    bool RemoveInlineStyleProperty(css::PropertyId property) noexcept;

    /// @return Whether the inline style had the custom @p property.
    bool RemoveInlineStyleCustomProperty(const CSSOMStringAtom &property) noexcept;

    void RemoveAllInlineStyleProperties() noexcept;

#pragma endregion

#pragma region Presentational hints

    /// @brief The declarations the element's presentational attributes map to, rebuilt after one changes.
    /// @see https://html.spec.whatwg.org/multipage/rendering.html#presentational-hints
    KRYS_NODISCARD const css::ImmutableStyleProperties *PresentationalHintStyle() const noexcept;

    /// @brief Presentational hints that depend on several attributes or on other elements (tables and their
    /// parts share them), applied after the element's own.
    KRYS_NODISCARD virtual const css::MutableStyleProperties *
      AdditionalPresentationalHintStyle() const noexcept
    {
      return nullptr;
    }

#pragma endregion

  protected:
    /// @brief Adds the declarations the attribute maps to, if it's a presentational attribute of the element.
    virtual void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                        const DOMStringAtom &namespaceURI,
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
    KRYS_NODISCARD virtual bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept
    {
      (void)localName;
      (void)namespaceURI;
      return false;
    }

    /// @brief Adds the hint @p property: @p identifier to @p style.
    static void AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style,
                                                     css::PropertyId property,
                                                     css::ValueId identifier) noexcept;

    /// @brief Adds the hint @p property: @p value in @p unit to @p style.
    static void AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style,
                                                     css::PropertyId property, double value,
                                                     css::UnitType unit) noexcept;

    /// @brief Adds the hint @p property: @p value, parsed in the document's context, to @p style.
    void AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                              const CSSOMString &value) noexcept;

    /// @brief Adds the hint @p property: @p value to @p style.
    static void AddPropertyToPresentationalHintStyle(css::MutableStyleProperties &style,
                                                     css::PropertyId property,
                                                     Ref<css::Value> &&value) noexcept;

    /// @brief Marks the presentational hints out of date, for a change that isn't an attribute change.
    void InvalidatePresentationalHintStyle() noexcept;

    /// @brief Reparses the inline style when `style` changes, and marks the hints out of date when a
    /// presentational attribute does.
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

  private:
    /// @brief Writes the serialised inline style to the `style` attribute and clears the dirty flag.
    void SynchronizeStyleAttributeInternal() noexcept;

    /// @brief Writes the inline style back to the `style` attribute now, invalidating the style of the rules
    /// that select on it.
    void SynchronizeStyleAttributeWithInvalidation() noexcept;

    /// @brief Replaces the inline style with the `style` attribute's new value, or empties it if the
    /// attribute was removed.
    void StyleAttributeChanged(Maybe<DOMStringView> newStyleString) noexcept;

    void SetInlineStyleFromString(DOMStringView newStyleString) noexcept;

    /// @brief The inline style, made mutable (copying a shared immutable one) or created empty.
    KRYS_NODISCARD Ref<css::MutableStyleProperties> EnsureMutableInlineStyle() noexcept;

    void RebuildPresentationalHintStyle() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::StyledElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &node) noexcept
  {
    return node.IsStyledElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
