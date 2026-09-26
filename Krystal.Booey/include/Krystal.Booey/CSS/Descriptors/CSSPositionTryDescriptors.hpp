#pragma once

#include "Krystal.Booey/CSS/Descriptors/PropertySetCSSDescriptors.hpp"

namespace krys::boo::css
{
  class CSSPositionTryRule;

  struct ParserContext;

  class CSSPositionTryDescriptors final : public PropertySetCSSDescriptors
  {
  private:
    CSSPositionTryDescriptors(MutableStyleProperties &properties, CSSPositionTryRule &rule) noexcept;

  public:
    virtual ~CSSPositionTryDescriptors() noexcept;

    KRYS_NODISCARD static Ref<CSSPositionTryDescriptors> Create(MutableStyleProperties &propertySet,
                                                                CSSPositionTryRule &parentRule) noexcept
    {
      return AdoptRef(*new CSSPositionTryDescriptors(propertySet, parentRule));
    }

    KRYS_NODISCARD StyleDeclarationType DeclarationType() const noexcept final
    {
      return StyleDeclarationType::PositionTry;
    }

    KRYS_NODISCARD CSSOMString Margin() const noexcept;
    dom::ExceptionOr<void> Margin(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginTop() const noexcept;
    dom::ExceptionOr<void> MarginTop(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginRight() const noexcept;
    dom::ExceptionOr<void> MarginRight(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginBottom() const noexcept;
    dom::ExceptionOr<void> MarginBottom(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginLeft() const noexcept;
    dom::ExceptionOr<void> MarginLeft(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginBlock() const noexcept;
    dom::ExceptionOr<void> MarginBlock(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginBlockStart() const noexcept;
    dom::ExceptionOr<void> MarginBlockStart(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginBlockEnd() const noexcept;
    dom::ExceptionOr<void> MarginBlockEnd(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginInline() const noexcept;
    dom::ExceptionOr<void> MarginInline(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginInlineStart() const noexcept;
    dom::ExceptionOr<void> MarginInlineStart(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MarginInlineEnd() const noexcept;
    dom::ExceptionOr<void> MarginInlineEnd(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Inset() const noexcept;
    dom::ExceptionOr<void> Inset(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetBlock() const noexcept;
    dom::ExceptionOr<void> InsetBlock(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetBlockStart() const noexcept;
    dom::ExceptionOr<void> InsetBlockStart(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetBlockEnd() const noexcept;
    dom::ExceptionOr<void> InsetBlockEnd(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetInline() const noexcept;
    dom::ExceptionOr<void> InsetInline(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetInlineStart() const noexcept;
    dom::ExceptionOr<void> InsetInlineStart(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InsetInlineEnd() const noexcept;
    dom::ExceptionOr<void> InsetInlineEnd(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Top() const noexcept;
    dom::ExceptionOr<void> Top(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Left() const noexcept;
    dom::ExceptionOr<void> Left(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Right() const noexcept;
    dom::ExceptionOr<void> Right(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Bottom() const noexcept;
    dom::ExceptionOr<void> Bottom(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Width() const noexcept;
    dom::ExceptionOr<void> Width(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MinWidth() const noexcept;
    dom::ExceptionOr<void> MinWidth(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MaxWidth() const noexcept;
    dom::ExceptionOr<void> MaxWidth(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString Height() const noexcept;
    dom::ExceptionOr<void> Height(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MinHeight() const noexcept;
    dom::ExceptionOr<void> MinHeight(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MaxHeight() const noexcept;
    dom::ExceptionOr<void> MaxHeight(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString BlockSize() const noexcept;
    dom::ExceptionOr<void> BlockSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MinBlockSize() const noexcept;
    dom::ExceptionOr<void> MinBlockSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MaxBlockSize() const noexcept;
    dom::ExceptionOr<void> MaxBlockSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString InlineSize() const noexcept;
    dom::ExceptionOr<void> InlineSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MinInlineSize() const noexcept;
    dom::ExceptionOr<void> MinInlineSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString MaxInlineSize() const noexcept;
    dom::ExceptionOr<void> MaxInlineSize(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString PlaceSelf() const noexcept;
    dom::ExceptionOr<void> PlaceSelf(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString AlignSelf() const noexcept;
    dom::ExceptionOr<void> AlignSelf(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString JustifySelf() const noexcept;
    dom::ExceptionOr<void> JustifySelf(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString PositionAnchor() const noexcept;
    dom::ExceptionOr<void> PositionAnchor(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSOMString PositionArea() const noexcept;
    dom::ExceptionOr<void> PositionArea(const CSSOMString &value) noexcept;

  private:
    // Override this to invalidate all remembered last-successful position options
    // when property value changes.
    KRYS_NODISCARD dom::ExceptionOr<void> SetPropertyInternal(PropertyId property, const CSSOMString &value,
                                                              IsImportant isImportant) noexcept override;

    KRYS_NODISCARD css::RuleType RuleType() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CSSPositionTryDescriptors)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::CSSStyleDeclaration &declaration) noexcept
  {
    return declaration.DeclarationType() == krys::boo::css::StyleDeclarationType::PositionTry;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
