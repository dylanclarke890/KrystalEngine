#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::dom
{
  class Element;

  template <typename>
  class ExceptionOr;
}

namespace krys::boo::css
{
  class CSSRule;
  class CSSStyleSheet;

  enum class StyleDeclarationType : uint8
  {
    Style,
    FontFace,
    Page,
    PositionTry,
    Function
  };

  /// @brief A declaration block exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#the-cssstyledeclaration-interface
  // FIXME: Settings() comes with Settings.
  class CSSStyleDeclaration : public AbstractRefCountedAndCanMakeWeakPtr<CSSStyleDeclaration>
  {
    KRYS_NON_COPYABLE(CSSStyleDeclaration);

  protected:
    CSSStyleDeclaration() noexcept = default;

  public:
    virtual ~CSSStyleDeclaration() noexcept;

    KRYS_NODISCARD virtual StyleDeclarationType GetStyleDeclarationType() const noexcept = 0;

    /// @brief The element whose `style` attribute this is, if any.
    KRYS_NODISCARD virtual dom::Element *ParentElement() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual CSSRule *ParentRule() const noexcept = 0;
    KRYS_NODISCARD virtual CSSRule *CssRules() const noexcept = 0;
    KRYS_NODISCARD virtual CSSOMString CssText() const noexcept = 0;
    virtual dom::ExceptionOr<void> SetCssText(const CSSOMString &text) noexcept = 0;
    KRYS_NODISCARD virtual uint32 Length() const noexcept = 0;
    KRYS_NODISCARD virtual CSSOMString Item(uint32 index) const noexcept = 0;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }

    KRYS_NODISCARD virtual CSSOMString GetPropertyValue(const CSSOMString &propertyName) noexcept = 0;
    KRYS_NODISCARD virtual CSSOMString GetPropertyPriority(const CSSOMString &propertyName) noexcept = 0;
    KRYS_NODISCARD virtual CSSOMString GetPropertyShorthand(const CSSOMString &propertyName) noexcept = 0;
    KRYS_NODISCARD virtual bool IsPropertyImplicit(const CSSOMString &propertyName) noexcept = 0;
    virtual dom::ExceptionOr<void> SetProperty(const CSSOMString &propertyName, const CSSOMString &value,
                                               const CSSOMString &priority) noexcept = 0;
    virtual dom::ExceptionOr<CSSOMString> RemoveProperty(const CSSOMString &propertyName) noexcept = 0;

    KRYS_NODISCARD virtual CSSStyleSheet *ParentStyleSheet() const noexcept
    {
      return nullptr;
    }
  };
}
