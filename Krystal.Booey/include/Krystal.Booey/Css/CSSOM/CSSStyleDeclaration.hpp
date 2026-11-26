#pragma once

#include "Krystal.Booey/Css/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"

namespace krys::boo
{
  class CSSRule;
  class CSSStyleSheet;
  class Element;
  template <typename>
  class ExceptionOr;

  /// @brief Which kind of declaration block a `CSSStyleDeclaration` is, by the descriptors it holds.
  enum class StyleDeclarationType : uint8
  {
    Style,
    FontFace,
    Page,
    PositionTry,
    Function,
  };

  /// @brief A CSS declaration block as scripts see it: the declarations of a rule, of an element's `style`
  /// attribute, or of an element's computed style.
  /// @see https://drafts.csswg.org/cssom/#the-cssstyledeclaration-interface
  class CSSStyleDeclaration : public AbstractRefCountedAndCanMakeWeakPtr<CSSStyleDeclaration>
  {
    KRYS_NON_COPYABLE(CSSStyleDeclaration);

  protected:
    CSSStyleDeclaration() noexcept = default;

  public:
    ~CSSStyleDeclaration() noexcept override;

    KRYS_NODISCARD virtual StyleDeclarationType GetStyleDeclarationType() const noexcept = 0;

    /// @brief The element whose `style` attribute this is, if any.
    /// @see https://drafts.csswg.org/cssom/#cssstyledeclaration-owner-node
    KRYS_NODISCARD virtual Element *ParentElement() const noexcept
    {
      return nullptr;
    }

    /// @brief The style sheet whose rule holds the block, if any.
    KRYS_NODISCARD virtual CSSStyleSheet *ParentStyleSheet() const noexcept
    {
      return nullptr;
    }

#pragma region CSSStyleDeclaration - https://drafts.csswg.org/cssom/#the-cssstyledeclaration-interface

    /// @brief The serialisation of the block's declarations.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-csstext
    KRYS_NODISCARD virtual CSSOMString CssText() const noexcept = 0;

    /// @brief Replaces the block's declarations with those parsed from @p text.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-csstext
    virtual ExceptionOr<void> SetCssText(const CSSOMString &text) noexcept = 0;

    /// @brief The number of declarations, with shorthands counted as their longhands.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-length
    KRYS_NODISCARD virtual uint32 Length() const noexcept = 0;

    /// @brief The name of the property of the declaration at @p index, or empty past the end.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-item
    KRYS_NODISCARD virtual CSSOMString Item(uint32 index) const noexcept = 0;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }

    /// @brief The serialised value of @p propertyName, or empty if it isn't declared.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-getpropertyvalue
    KRYS_NODISCARD virtual CSSOMString GetPropertyValue(const CSSOMString &propertyName) noexcept = 0;

    /// @brief `important` if @p propertyName is declared `!important`, otherwise empty.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-getpropertypriority
    KRYS_NODISCARD virtual CSSOMString GetPropertyPriority(const CSSOMString &propertyName) noexcept = 0;

    /// @brief Sets @p propertyName to @p value parsed, with @p priority `important` or empty.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-setproperty
    virtual ExceptionOr<void> SetProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                          const CSSOMString &priority) noexcept = 0;

    /// @brief Removes @p propertyName's declaration, returning its value as it was.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-removeproperty
    virtual ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &propertyName) noexcept = 0;

    /// @brief The rule whose declarations these are, if any.
    /// @see https://drafts.csswg.org/cssom/#dom-cssstyledeclaration-parentrule
    KRYS_NODISCARD virtual CSSRule *ParentRule() const noexcept = 0;

#pragma endregion
  };
}
