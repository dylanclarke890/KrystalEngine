#pragma once

#include "Krystal.Booey/CSS/Descriptors/PropertySetCSSDescriptors.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::css
{
  class CSSPositionTryRule;

  /// @brief The descriptors of an `@position-try` rule, exposed to scripts.
  /// @see https://drafts.csswg.org/css-anchor-position/#csspositiontrydescriptors
  class CSSPositionTryDescriptors final : public PropertySetCSSDescriptors
  {
  private:
    CSSPositionTryDescriptors(MutableStyleProperties &propertySet, CSSPositionTryRule &parentRule) noexcept;

  public:
    ~CSSPositionTryDescriptors() noexcept;

    KRYS_NODISCARD static Ref<CSSPositionTryDescriptors> Create(MutableStyleProperties &propertySet,
                                                                CSSPositionTryRule &parentRule) noexcept;

    KRYS_NODISCARD StyleDeclarationType GetStyleDeclarationType() const noexcept final
    {
      return StyleDeclarationType::PositionTry;
    }

    KRYS_NODISCARD CSSOMString Margin() const noexcept;
    dom::ExceptionOr<void> SetMargin(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginTop() const noexcept;
    dom::ExceptionOr<void> SetMarginTop(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginRight() const noexcept;
    dom::ExceptionOr<void> SetMarginRight(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginBottom() const noexcept;
    dom::ExceptionOr<void> SetMarginBottom(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginLeft() const noexcept;
    dom::ExceptionOr<void> SetMarginLeft(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginBlock() const noexcept;
    dom::ExceptionOr<void> SetMarginBlock(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginBlockStart() const noexcept;
    dom::ExceptionOr<void> SetMarginBlockStart(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginBlockEnd() const noexcept;
    dom::ExceptionOr<void> SetMarginBlockEnd(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginInline() const noexcept;
    dom::ExceptionOr<void> SetMarginInline(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginInlineStart() const noexcept;
    dom::ExceptionOr<void> SetMarginInlineStart(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MarginInlineEnd() const noexcept;
    dom::ExceptionOr<void> SetMarginInlineEnd(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString Inset() const noexcept;
    dom::ExceptionOr<void> SetInset(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString InsetBlock() const noexcept;
    dom::ExceptionOr<void> SetInsetBlock(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString InsetBlockStart() const noexcept;
    dom::ExceptionOr<void> SetInsetBlockStart(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString InsetBlockEnd() const noexcept;
    dom::ExceptionOr<void> SetInsetBlockEnd(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString InsetInline() const noexcept;
    dom::ExceptionOr<void> SetInsetInline(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString InsetInlineStart() const noexcept;
    dom::ExceptionOr<void> SetInsetInlineStart(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString InsetInlineEnd() const noexcept;
    dom::ExceptionOr<void> SetInsetInlineEnd(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString Top() const noexcept;
    dom::ExceptionOr<void> SetTop(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString Left() const noexcept;
    dom::ExceptionOr<void> SetLeft(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString Right() const noexcept;
    dom::ExceptionOr<void> SetRight(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString Bottom() const noexcept;
    dom::ExceptionOr<void> SetBottom(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString Width() const noexcept;
    dom::ExceptionOr<void> SetWidth(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MinWidth() const noexcept;
    dom::ExceptionOr<void> SetMinWidth(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MaxWidth() const noexcept;
    dom::ExceptionOr<void> SetMaxWidth(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString Height() const noexcept;
    dom::ExceptionOr<void> SetHeight(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MinHeight() const noexcept;
    dom::ExceptionOr<void> SetMinHeight(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MaxHeight() const noexcept;
    dom::ExceptionOr<void> SetMaxHeight(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString BlockSize() const noexcept;
    dom::ExceptionOr<void> SetBlockSize(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MinBlockSize() const noexcept;
    dom::ExceptionOr<void> SetMinBlockSize(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MaxBlockSize() const noexcept;
    dom::ExceptionOr<void> SetMaxBlockSize(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString InlineSize() const noexcept;
    dom::ExceptionOr<void> SetInlineSize(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MinInlineSize() const noexcept;
    dom::ExceptionOr<void> SetMinInlineSize(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString MaxInlineSize() const noexcept;
    dom::ExceptionOr<void> SetMaxInlineSize(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString PlaceSelf() const noexcept;
    dom::ExceptionOr<void> SetPlaceSelf(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString AlignSelf() const noexcept;
    dom::ExceptionOr<void> SetAlignSelf(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString JustifySelf() const noexcept;
    dom::ExceptionOr<void> SetJustifySelf(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString PositionAnchor() const noexcept;
    dom::ExceptionOr<void> SetPositionAnchor(const CSSOMString &value) noexcept;
    KRYS_NODISCARD CSSOMString PositionArea() const noexcept;
    dom::ExceptionOr<void> SetPositionArea(const CSSOMString &value) noexcept;

  private:
    // Override this to invalidate all remembered last-successful position options
    // when property value changes.
    dom::ExceptionOr<void> SetPropertyInternal(PropertyId property, const CSSOMString &value,
                                               IsImportant important) noexcept override;

    KRYS_NODISCARD RuleType GetRuleType() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSPositionTryDescriptors)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleDeclaration &declaration) noexcept
  {
    return declaration.GetStyleDeclarationType() == krys::boo::css::StyleDeclarationType::PositionTry;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
