#include "Krystal.Booey/Rendering/RenderMenuList.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/Core/UnicodeHelpers.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/HTML/HTMLHRElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/PopupMenu.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    constexpr int32 EndOfLinePadding = 2;
  }

  RenderMenuList::RenderMenuList(html::HTMLSelectElement &element, RenderStyle &&style) noexcept
      : RenderFlexibleBox(Type::MenuList, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMenuList());
  }

  // Do not add any code in below destructor. Add it to WillBeDestroyed() instead.
  RenderMenuList::~RenderMenuList() noexcept = default;

  void RenderMenuList::WillBeDestroyed() noexcept
  {
    if (_popup)
    {
      _popup->DisconnectClient();
    }
    _popup = nullptr;

    RenderFlexibleBox::WillBeDestroyed();
  }

  void RenderMenuList::SetInnerRenderer(RenderBlock &innerRenderer) noexcept
  {
    krys_debug_assert(!_innerBlock.get());
    _innerBlock = WeakPtr<RenderBlock>::WithRef(innerRenderer);
    AdjustInnerStyle();
  }

  void RenderMenuList::AdjustInnerStyle() noexcept
  {
    auto &innerStyle = _innerBlock->MutableStyle();
    innerStyle.SetFlexGrow(style::FlexGrow {1.0f});
    innerStyle.SetFlexShrink(style::FlexShrink {1.0f});
    // min-width: 0; is needed for correct shrinking.
    if (IsHorizontalWritingMode())
    {
      innerStyle.SetMinWidth(style::MinimumSize::Fixed {0});
    }
    else
    {
      innerStyle.SetMinHeight(style::MinimumSize::Fixed {0});
    }
    // Use margin:auto instead of align-items:center to get safe centering, i.e. when the content overflows,
    // treat it the same as align-items: flex-start. But we only do that for the cases where html.css would
    // otherwise use center.
    if (Style().GetAlignItems().IsCenter())
    {
      innerStyle.SetMarginBefore(css::keywords::Auto {});
      innerStyle.SetMarginAfter(css::keywords::Auto {});
      innerStyle.SetAlignSelf(css::keywords::FlexStart {});
    }

    auto paddingBox = RenderTheme::Singleton().PopupInternalPaddingBox(Style());
    if (!GetWritingMode().IsHorizontal())
    {
      paddingBox = {paddingBox.Left(), paddingBox.Top(), paddingBox.Right(), paddingBox.Bottom()};
    }
    innerStyle.SetPaddingBox(krys::move(paddingBox));

    // A document without a page behaves as with the default chrome client.
    auto *page = GetDocument().GetPage();
    if (page && page->GetChromeClient().SelectItemWritingDirectionIsNatural())
    {
      // Items in the popup will not respect the CSS text-align and direction properties, so we must adjust
      // our own style to match.
      innerStyle.SetTextAlign(style::TextAlign::Left);
      auto direction = TextDirection::LTR;
      if (_buttonText && BaseTextDirection(_buttonText->Text()) == TextDirection::RTL)
      {
        direction = TextDirection::RTL;
      }
      (void)innerStyle.SetDirection(direction);
    }
    else if (_optionStyle
             && (!page || page->GetChromeClient().SelectItemAlignmentFollowsMenuWritingDirection()))
    {
      if (_optionStyle->GetWritingMode().BidiDirection() != innerStyle.GetWritingMode().BidiDirection()
          || _optionStyle->GetUnicodeBidi() != innerStyle.GetUnicodeBidi())
      {
        _innerBlock->SetNeedsLayoutAndPreferredWidthsUpdate();
      }
      innerStyle.SetTextAlign(GetWritingMode().IsBidiLTR() ? style::TextAlign::Left
                                                           : style::TextAlign::Right);
      (void)innerStyle.SetDirection(_optionStyle->GetWritingMode().BidiDirection());
      innerStyle.SetUnicodeBidi(_optionStyle->GetUnicodeBidi());
    }

    if (_innerBlock && _innerBlock->LayoutBox())
    {
      if (auto *inlineFormattingContextRoot = DynamicDowncast<RenderBlockFlow>(*_innerBlock);
          inlineFormattingContextRoot && inlineFormattingContextRoot->InlineLayout())
      {
        (void)inlineFormattingContextRoot->InlineLayout()->RootStyleWillChange(*inlineFormattingContextRoot,
                                                                               innerStyle);
      }
      if (auto *lineLayout = layout_integration::LineLayout::Containing(*_innerBlock))
      {
        (void)lineLayout->StyleWillChange(*_innerBlock, innerStyle, style::DifferenceResult::Layout);
      }
      layout_integration::LineLayout::UpdateStyle(*_innerBlock);
      for (auto *child = _innerBlock->FirstChild(); child; child = child->NextSibling())
      {
        if (auto *text = DynamicDowncast<RenderText>(*child))
        {
          layout_integration::LineLayout::UpdateStyle(*text);
        }
      }
    }
  }

  html::HTMLSelectElement &RenderMenuList::SelectElement() const noexcept
  {
    return Downcast<html::HTMLSelectElement>(NodeForNonAnonymous());
  }

  void RenderMenuList::DidAttachChild(RenderObject &, RenderObject *) noexcept
  {
    // FIXME: Tell the accessibility object cache, once there is accessibility.
  }

  void RenderMenuList::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlock::StyleDidChange(diff, oldStyle);

    // RenderBlock handled updating the anonymous block's style.
    if (_innerBlock)
    {
      AdjustInnerStyle();
    }

    bool fontChanged = !oldStyle || !oldStyle->FontCascadeEqual(Style());
    if (fontChanged)
    {
      UpdateOptionsWidth();
      _needsOptionsWidthUpdate = false;
    }
  }

  void RenderMenuList::UpdateOptionsWidth() noexcept
  {
    float maxOptionWidth = 0;
    for (auto &item : SelectElement().ListItems())
    {
      auto *option = DynamicDowncast<html::HTMLOptionElement>(item.get());
      if (!option)
      {
        continue;
      }

      auto text = ApplyTextTransform(Style(), option->TextIndentedToRespectGroupLabel());
      if (!text.empty())
      {
        auto &font = Style().GetFontCascade();
        auto run = RenderBlock::ConstructTextRun(text, Style());
        maxOptionWidth = std::max(maxOptionWidth, font.Width(run));
      }
    }

    auto width = static_cast<int32>(std::ceil(maxOptionWidth));
    if (_optionsWidth == width)
    {
      return;
    }

    _optionsWidth = width;
    if (Parent())
    {
      SetNeedsLayoutAndPreferredWidthsUpdate();
    }
  }

  void RenderMenuList::UpdateFromElement() noexcept
  {
    if (_needsOptionsWidthUpdate)
    {
      UpdateOptionsWidth();
      _needsOptionsWidthUpdate = false;
    }

    if (_popupIsVisible)
    {
      _popup->UpdateFromElement();
    }
    else
    {
      SetTextFromOption(SelectElement().SelectedIndex());
    }
  }

  void RenderMenuList::SetTextFromOption(int32 optionIndex) noexcept
  {
    auto &listItems = SelectElement().ListItems();
    auto size = static_cast<int32>(listItems.size());

    int32 i = SelectElement().OptionToListIndex(optionIndex);
    utf8_string text;
    if (i >= 0 && i < size)
    {
      if (auto *option = DynamicDowncast<html::HTMLOptionElement>(listItems[static_cast<size_t>(i)].get()))
      {
        text = option->TextIndentedToRespectGroupLabel();
        auto *style = option->ComputedStyle();
        _optionStyle = style ? RenderStyle::ClonePtr(*style) : nullptr;
      }
    }

    SetText(utf8_string(infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(text)));
    DidUpdateActiveOption(optionIndex);
  }

  void RenderMenuList::SetText(const utf8_string &s) noexcept
  {
    utf8_string textToUse = s.empty() ? utf8_string(u8"\n") : s;

    if (_buttonText)
    {
      _buttonText->SetText(krys::move(textToUse), true);
    }
    else
    {
      auto newButtonText = CreateRenderer<RenderText>(Type::Text, GetDocument(), krys::move(textToUse));
      _buttonText = WeakPtr<RenderText>::WithRef(*newButtonText);
      // FIXME: This mutation should go through the normal RenderTreeBuilder path.
      if (RenderTreeBuilder::Current())
      {
        RenderTreeBuilder::Current()->Attach(*this, krys::move(newButtonText));
      }
      else
      {
        RenderTreeBuilder(*GetDocument().GetRenderView()).Attach(*this, krys::move(newButtonText));
      }
    }

    AdjustInnerStyle();
  }

  utf8_string RenderMenuList::Text() const noexcept
  {
    return _buttonText ? _buttonText->Text() : utf8_string();
  }

  LayoutRect RenderMenuList::ControlClipRect(const LayoutPoint &additionalOffset) const noexcept
  {
    // Clip to the intersection of the content box and the content box for the inner box. This will leave room
    // for the arrows which sit in the inner box padding, and if the inner box ever spills out of the outer
    // box, that will get clipped too.
    LayoutRect outerBox(additionalOffset.X() + BorderLeft() + PaddingLeft(),
                        additionalOffset.Y() + BorderTop() + PaddingTop(), ContentBoxWidth(),
                        ContentBoxHeight());

    LayoutRect innerBox(additionalOffset.X() + _innerBlock->X() + _innerBlock->PaddingLeft(),
                        additionalOffset.Y() + _innerBlock->Y() + _innerBlock->PaddingTop(),
                        _innerBlock->ContentBoxWidth(), _innerBlock->ContentBoxHeight());

    return Intersection(outerBox, innerBox);
  }

  void RenderMenuList::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                     LayoutUnit &maxLogicalWidth) const noexcept
  {
    if (Style().GetFieldSizing() == FieldSizing::Content)
    {
      RenderFlexibleBox::ComputeIntrinsicLogicalWidths(minLogicalWidth, maxLogicalWidth);
      return;
    }

    auto &theme = RenderTheme::Singleton();
    maxLogicalWidth =
      LayoutUnit(ShouldApplySizeContainment() ? theme.MinimumMenuListSize(Style())
                                              : std::max(_optionsWidth, theme.MinimumMenuListSize(Style())));
    maxLogicalWidth += _innerBlock->PaddingStart() + _innerBlock->PaddingEnd();
    if (ShouldApplySizeOrInlineSizeContainment())
    {
      if (auto logicalWidth = ExplicitIntrinsicInnerLogicalWidth())
      {
        maxLogicalWidth = *logicalWidth;
      }
    }

    auto &logicalWidth = Style().LogicalWidth();
    if (logicalWidth.IsCalculated())
    {
      minLogicalWidth =
        std::max(0_lu, style::Evaluate<LayoutUnit>(logicalWidth, 0_lu, Style().UsedZoomForLength()));
    }
    else if (!logicalWidth.IsPercent())
    {
      minLogicalWidth = maxLogicalWidth;
    }
  }

  void RenderMenuList::ComputePreferredLogicalWidths() noexcept
  {
    if (Style().GetFieldSizing() == FieldSizing::Content)
    {
      RenderFlexibleBox::ComputePreferredLogicalWidths();
      return;
    }

    _minPreferredLogicalWidth = 0_lu;
    _maxPreferredLogicalWidth = 0_lu;

    if (auto fixedLogicalWidth = Style().LogicalWidth().TryFixed();
        fixedLogicalWidth && fixedLogicalWidth->IsPositive())
    {
      _minPreferredLogicalWidth = _maxPreferredLogicalWidth = AdjustContentBoxLogicalWidthForBoxSizing(
        LayoutUnit {fixedLogicalWidth->ResolveZoom(Style().UsedZoomForLength())});
    }
    else
    {
      ComputeIntrinsicLogicalWidths(_minPreferredLogicalWidth, _maxPreferredLogicalWidth);
    }

    RenderBox::ComputePreferredLogicalWidths(Style().LogicalMinWidth(), Style().LogicalMaxWidth(),
                                             GetWritingMode().IsHorizontal()
                                               ? HorizontalBorderAndPaddingExtent()
                                               : VerticalBorderAndPaddingExtent());

    ClearNeedsPreferredWidthsUpdate();
  }

  void RenderMenuList::ShowPopup() noexcept
  {
    if (_popupIsVisible)
    {
      return;
    }

    krys_debug_assert(_innerBlock);
    if (!_popup)
    {
      if (auto *page = GetDocument().GetPage())
      {
        _popup = page->GetChromeClient().CreatePopupMenu(*this);
      }
    }
    if (!_popup)
    {
      return;
    }
    _popupIsVisible = true;

    // Compute the top left taking transforms into account, but use the actual width of the element to size
    // the popup.
    FloatPoint absTopLeft = LocalToAbsolute(FloatPoint(), {MapCoordinatesMode::UseTransforms});
    IntRect absBounds = AbsoluteBoundingBoxRectIgnoringTransforms();
    absBounds.SetLocation(RoundedIntPoint(absTopLeft));
    auto &select = SelectElement();
    // May destroy `this`.
    RefPtr<PopupMenu> popup = _popup;
    popup->Show(absBounds, View().FrameView(), select.OptionToListIndex(select.SelectedIndex()));
  }

  void RenderMenuList::HidePopup() noexcept
  {
    if (_popup)
    {
      _popup->Hide();
    }
  }

  void RenderMenuList::ValueChanged(uint32 listIndex, bool fireOnChange) noexcept
  {
    // FIXME: Check that a page navigation has not occurred while the popup was up, once there is navigation.
    auto &select = SelectElement();
    select.OptionSelectedByUser(select.ListToOptionIndex(static_cast<int32>(listIndex)), fireOnChange);
  }

  void RenderMenuList::ListBoxSelectItem(int32 listIndex, bool allowMultiplySelections, bool shift,
                                         bool fireOnChangeNow) noexcept
  {
    SelectElement().ListBoxSelectItem(listIndex, allowMultiplySelections, shift, fireOnChangeNow);
  }

  bool RenderMenuList::Multiple() const noexcept
  {
    return SelectElement().Multiple();
  }

  void RenderMenuList::DidSetSelectedIndex(int32 listIndex) noexcept
  {
    DidUpdateActiveOption(SelectElement().ListToOptionIndex(listIndex));
  }

  void RenderMenuList::DidUpdateActiveOption(int32 optionIndex) noexcept
  {
    if (_lastActiveIndex == optionIndex)
    {
      return;
    }
    _lastActiveIndex = optionIndex;

    // FIXME: Tell the accessibility object cache the selected option changed, once there is accessibility.
  }

  utf8_string RenderMenuList::ItemText(uint32 listIndex) const noexcept
  {
    auto &listItems = SelectElement().ListItems();
    if (listIndex >= listItems.size())
    {
      return {};
    }

    utf8_string itemString;
    auto &element = *listItems[listIndex];
    if (auto *optGroup = DynamicDowncast<html::HTMLOptGroupElement>(element))
    {
      itemString = optGroup->GroupLabelText();
    }
    else if (auto *option = DynamicDowncast<html::HTMLOptionElement>(element))
    {
      itemString = option->TextIndentedToRespectGroupLabel();
    }

    return ApplyTextTransform(Style(), krys::move(itemString));
  }

  utf8_string RenderMenuList::ItemLabel(uint32) const noexcept
  {
    return {};
  }

  utf8_string RenderMenuList::ItemIcon(uint32) const noexcept
  {
    return {};
  }

  utf8_string RenderMenuList::ItemAccessibilityText(uint32 listIndex) const noexcept
  {
    // Allow the accessible name be changed if necessary.
    auto &listItems = SelectElement().ListItems();
    if (listIndex >= listItems.size())
    {
      return {};
    }
    return listItems[listIndex]->GetAttribute(u8"aria-label").value_or(dom::DOMString());
  }

  utf8_string RenderMenuList::ItemToolTip(uint32 listIndex) const noexcept
  {
    auto &listItems = SelectElement().ListItems();
    if (listIndex >= listItems.size())
    {
      return {};
    }
    return listItems[listIndex]->Title();
  }

  bool RenderMenuList::ItemIsEnabled(uint32 listIndex) const noexcept
  {
    auto &listItems = SelectElement().ListItems();
    if (listIndex >= listItems.size())
    {
      return false;
    }

    auto *element = listItems[listIndex].get();
    if (!Is<html::HTMLOptionElement>(*element))
    {
      return false;
    }

    bool groupEnabled = true;
    if (auto *parentElement = element->ParentElement())
    {
      if (Is<html::HTMLOptGroupElement>(*parentElement))
      {
        groupEnabled = !Downcast<html::HTMLOptGroupElement>(*parentElement).IsDisabledFormControl();
      }
    }
    if (!groupEnabled)
    {
      return false;
    }

    return !element->IsDisabledFormControl();
  }

  PopupMenuStyle RenderMenuList::ItemStyle(uint32 listIndex) const noexcept
  {
    auto &listItems = SelectElement().ListItems();
    if (listIndex >= listItems.size())
    {
      // If we are making an out of bounds access, then we want to use the style of a different option element
      // (index 0). However, if there isn't an option element before at index 0, we fall back to the menu's
      // style.
      if (!listIndex)
      {
        return MenuStyle();
      }

      // Try to retrieve the style of an option element we know exists (index 0).
      listIndex = 0;
    }

    auto *element = listItems[listIndex].get();

    Color itemBackgroundColor;
    bool itemHasCustomBackgroundColor = false;
    GetItemBackgroundColor(listIndex, itemBackgroundColor, itemHasCustomBackgroundColor);

    auto *style = element->ComputedStyle();
    if (!style)
    {
      return MenuStyle();
    }

    return PopupMenuStyle(
      style->VisitedDependentColorWithColorFilter(css::PropertyId::Color), itemBackgroundColor,
      style->GetFontCascade(), element->GetAttribute(u8"lang").value_or(dom::DOMString()),
      style->GetVisibility() == Visibility::Visible, style->Display() == DisplayType::None, true,
      style->GetWritingMode().BidiDirection(), IsOverride(style->GetUnicodeBidi()),
      itemHasCustomBackgroundColor ? PopupMenuStyle::BackgroundColorType::CustomBackgroundColor
                                   : PopupMenuStyle::BackgroundColorType::DefaultBackgroundColor);
  }

  void RenderMenuList::GetItemBackgroundColor(uint32 listIndex, Color &itemBackgroundColor,
                                              bool &itemHasCustomBackgroundColor) const noexcept
  {
    auto &listItems = SelectElement().ListItems();
    if (listIndex >= listItems.size())
    {
      itemBackgroundColor = Style().VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
      itemHasCustomBackgroundColor = false;
      return;
    }

    auto *element = listItems[listIndex].get();

    Color backgroundColor;
    if (auto *style = element->ComputedStyle())
    {
      backgroundColor = style->VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
    }

    itemHasCustomBackgroundColor = backgroundColor.IsValid() && backgroundColor.IsVisible();
    // If the item has an opaque background color, return that.
    if (backgroundColor.IsOpaque())
    {
      itemBackgroundColor = backgroundColor;
      return;
    }

    // Otherwise, the item's background is overlayed on top of the menu background.
    backgroundColor = BlendSourceOver(
      Style().VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor), backgroundColor);
    if (backgroundColor.IsOpaque())
    {
      itemBackgroundColor = backgroundColor;
      return;
    }

    // If the menu background is not opaque, then add an opaque white background behind.
    itemBackgroundColor = BlendSourceOver(Color::white, backgroundColor);
  }

  PopupMenuStyle RenderMenuList::MenuStyle() const noexcept
  {
    auto &styleToUse = _innerBlock ? _innerBlock->Style() : Style();
    auto absBounds = AbsoluteBoundingBoxRectIgnoringTransforms();
    return PopupMenuStyle(
      styleToUse.VisitedDependentColorWithColorFilter(css::PropertyId::Color),
      styleToUse.VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor),
      styleToUse.GetFontCascade(), {}, styleToUse.UsedVisibility() == Visibility::Visible,
      styleToUse.Display() == DisplayType::None,
      Style().HasUsedAppearance() && Style().UsedAppearance() == StyleAppearance::Menulist,
      Style().GetWritingMode().BidiDirection(), IsOverride(Style().GetUnicodeBidi()),
      PopupMenuStyle::BackgroundColorType::DefaultBackgroundColor, PopupMenuStyle::PopupMenuType::SelectPopup,
      RenderTheme::Singleton().PopupMenuSize(styleToUse, absBounds));
  }

  int32 RenderMenuList::ClientInsetLeft() const noexcept
  {
    return 0;
  }

  int32 RenderMenuList::ClientInsetRight() const noexcept
  {
    return 0;
  }

  LayoutUnit RenderMenuList::ClientPaddingLeft() const noexcept
  {
    if ((Style().UsedAppearance() == StyleAppearance::Menulist
         || Style().UsedAppearance() == StyleAppearance::MenulistButton)
        && GetWritingMode().IsBidiRTL())
    {
      // For these appearance values, the theme applies padding to leave room for the drop-down button. But
      // leaving room for the button inside the popup menu itself looks strange, so we return a small default
      // padding to avoid having a large empty space appear on the side of the popup menu.
      return LayoutUnit(EndOfLinePadding);
    }

    // If the appearance isn't a menu list, then the select is styled (non-native), so we want to return the
    // user specified padding.
    return PaddingLeft() + _innerBlock->PaddingLeft();
  }

  LayoutUnit RenderMenuList::ClientPaddingRight() const noexcept
  {
    if ((Style().UsedAppearance() == StyleAppearance::Menulist
         || Style().UsedAppearance() == StyleAppearance::MenulistButton)
        && Style().GetWritingMode().IsBidiLTR())
    {
      return LayoutUnit(EndOfLinePadding);
    }

    return PaddingRight() + _innerBlock->PaddingRight();
  }

  int32 RenderMenuList::ListSize() const noexcept
  {
    return static_cast<int32>(SelectElement().ListItems().size());
  }

  int32 RenderMenuList::SelectedIndex() const noexcept
  {
    auto &select = SelectElement();
    return select.OptionToListIndex(select.SelectedIndex());
  }

  void RenderMenuList::PopupDidHide() noexcept
  {
    _popupIsVisible = false;
  }

  bool RenderMenuList::ItemIsSeparator(uint32 listIndex) const noexcept
  {
    auto &listItems = SelectElement().ListItems();
    return listIndex < listItems.size() && Is<html::HTMLHRElement>(*listItems[listIndex]);
  }

  bool RenderMenuList::ItemIsLabel(uint32 listIndex) const noexcept
  {
    auto &listItems = SelectElement().ListItems();
    return listIndex < listItems.size() && Is<html::HTMLOptGroupElement>(*listItems[listIndex]);
  }

  bool RenderMenuList::ItemIsSelected(uint32 listIndex) const noexcept
  {
    auto &listItems = SelectElement().ListItems();
    if (listIndex >= listItems.size())
    {
      return false;
    }
    auto *option = DynamicDowncast<html::HTMLOptionElement>(listItems[listIndex].get());
    return option && option->Selected();
  }

  void RenderMenuList::SetTextFromItem(uint32 listIndex) noexcept
  {
    SetTextFromOption(SelectElement().ListToOptionIndex(static_cast<int32>(listIndex)));
  }
}
