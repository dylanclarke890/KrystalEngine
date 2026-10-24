#include "Krystal.Booey/CSS/Descriptors/CSSPositionTryDescriptors.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSPositionTryRule.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  CSSPositionTryDescriptors::CSSPositionTryDescriptors(MutableStyleProperties &propertySet,
                                                       CSSPositionTryRule &parentRule) noexcept
      : PropertySetCSSDescriptors(propertySet, parentRule)
  {
  }

  CSSPositionTryDescriptors::~CSSPositionTryDescriptors() noexcept = default;

  Ref<CSSPositionTryDescriptors> CSSPositionTryDescriptors::Create(MutableStyleProperties &propertySet,
                                                                   CSSPositionTryRule &parentRule) noexcept
  {
    return AdoptRef(*new CSSPositionTryDescriptors(propertySet, parentRule));
  }

  RuleType CSSPositionTryDescriptors::GetRuleType() const noexcept
  {
    return RuleType::PositionTry;
  }

#pragma region Descriptors

  // @position-try 'margin'
  CSSOMString CSSPositionTryDescriptors::Margin() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Margin);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMargin(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Margin, value, IsImportant(false));
  }

  // @position-try 'margin-top'
  CSSOMString CSSPositionTryDescriptors::MarginTop() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginTop);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMarginTop(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginTop, value, IsImportant(false));
  }

  // @position-try 'margin-right'
  CSSOMString CSSPositionTryDescriptors::MarginRight() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginRight);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMarginRight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginRight, value, IsImportant(false));
  }

  // @position-try 'margin-bottom'
  CSSOMString CSSPositionTryDescriptors::MarginBottom() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginBottom);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMarginBottom(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginBottom, value, IsImportant(false));
  }

  // @position-try 'margin-left'
  CSSOMString CSSPositionTryDescriptors::MarginLeft() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginLeft);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMarginLeft(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginLeft, value, IsImportant(false));
  }

  // @position-try 'margin-block'
  CSSOMString CSSPositionTryDescriptors::MarginBlock() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginBlock);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMarginBlock(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginBlock, value, IsImportant(false));
  }

  // @position-try 'margin-block-start'
  CSSOMString CSSPositionTryDescriptors::MarginBlockStart() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginBlockStart);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMarginBlockStart(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginBlockStart, value, IsImportant(false));
  }

  // @position-try 'margin-block-end'
  CSSOMString CSSPositionTryDescriptors::MarginBlockEnd() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginBlockEnd);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMarginBlockEnd(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginBlockEnd, value, IsImportant(false));
  }

  // @position-try 'margin-inline'
  CSSOMString CSSPositionTryDescriptors::MarginInline() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginInline);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMarginInline(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginInline, value, IsImportant(false));
  }

  // @position-try 'margin-inline-start'
  CSSOMString CSSPositionTryDescriptors::MarginInlineStart() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginInlineStart);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMarginInlineStart(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginInlineStart, value, IsImportant(false));
  }

  // @position-try 'margin-inline-end'
  CSSOMString CSSPositionTryDescriptors::MarginInlineEnd() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MarginInlineEnd);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMarginInlineEnd(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MarginInlineEnd, value, IsImportant(false));
  }

  // @position-try 'inset'
  CSSOMString CSSPositionTryDescriptors::Inset() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Inset);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetInset(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Inset, value, IsImportant(false));
  }

  // @position-try 'inset-block'
  CSSOMString CSSPositionTryDescriptors::InsetBlock() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::InsetBlock);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetInsetBlock(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::InsetBlock, value, IsImportant(false));
  }

  // @position-try 'inset-block-start'
  CSSOMString CSSPositionTryDescriptors::InsetBlockStart() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::InsetBlockStart);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetInsetBlockStart(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::InsetBlockStart, value, IsImportant(false));
  }

  // @position-try 'inset-block-end'
  CSSOMString CSSPositionTryDescriptors::InsetBlockEnd() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::InsetBlockEnd);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetInsetBlockEnd(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::InsetBlockEnd, value, IsImportant(false));
  }

  // @position-try 'inset-inline'
  CSSOMString CSSPositionTryDescriptors::InsetInline() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::InsetInline);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetInsetInline(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::InsetInline, value, IsImportant(false));
  }

  // @position-try 'inset-inline-start'
  CSSOMString CSSPositionTryDescriptors::InsetInlineStart() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::InsetInlineStart);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetInsetInlineStart(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::InsetInlineStart, value, IsImportant(false));
  }

  // @position-try 'inset-inline-end'
  CSSOMString CSSPositionTryDescriptors::InsetInlineEnd() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::InsetInlineEnd);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetInsetInlineEnd(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::InsetInlineEnd, value, IsImportant(false));
  }

  // @position-try 'top'
  CSSOMString CSSPositionTryDescriptors::Top() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Top);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetTop(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Top, value, IsImportant(false));
  }

  // @position-try 'left'
  CSSOMString CSSPositionTryDescriptors::Left() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Left);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetLeft(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Left, value, IsImportant(false));
  }

  // @position-try 'right'
  CSSOMString CSSPositionTryDescriptors::Right() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Right);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetRight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Right, value, IsImportant(false));
  }

  // @position-try 'bottom'
  CSSOMString CSSPositionTryDescriptors::Bottom() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Bottom);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetBottom(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Bottom, value, IsImportant(false));
  }

  // @position-try 'width'
  CSSOMString CSSPositionTryDescriptors::Width() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Width);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetWidth(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Width, value, IsImportant(false));
  }

  // @position-try 'min-width'
  CSSOMString CSSPositionTryDescriptors::MinWidth() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MinWidth);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMinWidth(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MinWidth, value, IsImportant(false));
  }

  // @position-try 'max-width'
  CSSOMString CSSPositionTryDescriptors::MaxWidth() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MaxWidth);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMaxWidth(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MaxWidth, value, IsImportant(false));
  }

  // @position-try 'height'
  CSSOMString CSSPositionTryDescriptors::Height() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::Height);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetHeight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::Height, value, IsImportant(false));
  }

  // @position-try 'min-height'
  CSSOMString CSSPositionTryDescriptors::MinHeight() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MinHeight);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMinHeight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MinHeight, value, IsImportant(false));
  }

  // @position-try 'max-height'
  CSSOMString CSSPositionTryDescriptors::MaxHeight() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MaxHeight);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMaxHeight(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MaxHeight, value, IsImportant(false));
  }

  // @position-try 'block-size'
  CSSOMString CSSPositionTryDescriptors::BlockSize() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::BlockSize);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetBlockSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::BlockSize, value, IsImportant(false));
  }

  // @position-try 'min-block-size'
  CSSOMString CSSPositionTryDescriptors::MinBlockSize() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MinBlockSize);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMinBlockSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MinBlockSize, value, IsImportant(false));
  }

  // @position-try 'max-block-size'
  CSSOMString CSSPositionTryDescriptors::MaxBlockSize() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MaxBlockSize);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMaxBlockSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MaxBlockSize, value, IsImportant(false));
  }

  // @position-try 'inline-size'
  CSSOMString CSSPositionTryDescriptors::InlineSize() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::InlineSize);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetInlineSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::InlineSize, value, IsImportant(false));
  }

  // @position-try 'min-inline-size'
  CSSOMString CSSPositionTryDescriptors::MinInlineSize() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MinInlineSize);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMinInlineSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MinInlineSize, value, IsImportant(false));
  }

  // @position-try 'max-inline-size'
  CSSOMString CSSPositionTryDescriptors::MaxInlineSize() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::MaxInlineSize);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetMaxInlineSize(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::MaxInlineSize, value, IsImportant(false));
  }

  // @position-try 'place-self'
  CSSOMString CSSPositionTryDescriptors::PlaceSelf() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::PlaceSelf);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetPlaceSelf(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::PlaceSelf, value, IsImportant(false));
  }

  // @position-try 'align-self'
  CSSOMString CSSPositionTryDescriptors::AlignSelf() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::AlignSelf);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetAlignSelf(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::AlignSelf, value, IsImportant(false));
  }

  // @position-try 'justify-self'
  CSSOMString CSSPositionTryDescriptors::JustifySelf() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::JustifySelf);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetJustifySelf(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::JustifySelf, value, IsImportant(false));
  }

  // @position-try 'position-anchor'
  CSSOMString CSSPositionTryDescriptors::PositionAnchor() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::PositionAnchor);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetPositionAnchor(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::PositionAnchor, value, IsImportant(false));
  }

  // @position-try 'position-area'
  CSSOMString CSSPositionTryDescriptors::PositionArea() const noexcept
  {
    return GetPropertyValueInternal(PropertyId::PositionArea);
  }

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetPositionArea(const CSSOMString &value) noexcept
  {
    return SetPropertyInternal(PropertyId::PositionArea, value, IsImportant(false));
  }

#pragma endregion

  dom::ExceptionOr<void> CSSPositionTryDescriptors::SetPropertyInternal(PropertyId property,
                                                                        const CSSOMString &value,
                                                                        IsImportant important) noexcept
  {
    auto result = PropertySetCSSDescriptors::SetPropertyInternal(property, value, important);

    auto invalidateAllRememberedLastSuccessfulPositionOptions = [&]()
    {
      auto *parentRule = _parentRule.get();
      krys_debug_assert(parentRule);
      if (!parentRule)
      {
        return;
      }

      auto *parentStyleSheet = parentRule->ParentStyleSheet();
      krys_debug_assert(parentStyleSheet);
      if (!parentStyleSheet)
      {
        return;
      }

      if (auto *parentStyleSheetScope = parentStyleSheet->StyleScope())
      {
        // FIXME: only invalidate elements that use this position-try rule in position-try-fallbacks
        parentStyleSheetScope->SetLastSuccessfulPositionOptionIndexMap({});
      }
    };

    invalidateAllRememberedLastSuccessfulPositionOptions();

    return result;
  }
}
