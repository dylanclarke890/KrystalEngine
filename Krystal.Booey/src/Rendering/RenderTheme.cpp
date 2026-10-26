#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/CSS/Values/Color/ContrastColorResolver.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Graphics/Controls/ButtonPart.hpp"
#include "Krystal.Booey/Graphics/Controls/ColorWellPart.hpp"
#include "Krystal.Booey/Graphics/Controls/InnerSpinButtonPart.hpp"
#include "Krystal.Booey/Graphics/Controls/MenuListButtonPart.hpp"
#include "Krystal.Booey/Graphics/Controls/MenuListPart.hpp"
#include "Krystal.Booey/Graphics/Controls/MeterPart.hpp"
#include "Krystal.Booey/Graphics/Controls/ProgressBarPart.hpp"
#include "Krystal.Booey/Graphics/Controls/SearchFieldCancelButtonPart.hpp"
#include "Krystal.Booey/Graphics/Controls/SearchFieldPart.hpp"
#include "Krystal.Booey/Graphics/Controls/SearchFieldResultsPart.hpp"
#include "Krystal.Booey/Graphics/Controls/SliderThumbPart.hpp"
#include "Krystal.Booey/Graphics/Controls/SliderTrackPart.hpp"
#include "Krystal.Booey/Graphics/Controls/TextAreaPart.hpp"
#include "Krystal.Booey/Graphics/Controls/TextFieldPart.hpp"
#include "Krystal.Booey/Graphics/Controls/ToggleButtonPart.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/StringTruncator.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLMeterElement.hpp"
#include "Krystal.Booey/HTML/HTMLProgressElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/HTML/Shadow/SliderThumbElement.hpp"
#include "Krystal.Booey/HTML/Shadow/SpinButtonElement.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderMeter.hpp"
#include "Krystal.Booey/Rendering/RenderProgress.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"
#include <cmath>

namespace krys::boo
{
  using css::ValueId;

  RenderTheme::RenderTheme() noexcept = default;
  RenderTheme::~RenderTheme() noexcept = default;

  StyleAppearance RenderTheme::AdjustAppearanceForElement(RenderStyle &style, const RenderStyle &parentStyle,
                                                          const dom::Element *element,
                                                          StyleAppearance autoAppearance) const noexcept
  {
    if (!element)
    {
      style.SetUsedAppearance(StyleAppearance::None);
      return StyleAppearance::None;
    }

    // Each user agent part for input type='color' controls
    // should use StyleAppearance::None if their parent is
    // using primitive appearance.

    if ((autoAppearance == StyleAppearance::ColorWellSwatch
         || autoAppearance == StyleAppearance::ColorWellSwatchOverlay
         || autoAppearance == StyleAppearance::ColorWellSwatchWrapper)
        && (parentStyle.UsedAppearance() == StyleAppearance::None))
    {
      style.SetUsedAppearance(StyleAppearance::None);
      return StyleAppearance::None;
    }

    auto appearance = style.UsedAppearance();
    if (appearance == autoAppearance)
    {
      return appearance;
    }

    // Aliases of 'auto'.
    // https://drafts.csswg.org/css-ui-4/#typedef-appearance-compat-auto
    if (appearance == StyleAppearance::Auto || appearance == StyleAppearance::SearchField
        || appearance == StyleAppearance::TextArea || appearance == StyleAppearance::Checkbox
        || appearance == StyleAppearance::Radio || appearance == StyleAppearance::Listbox
        || appearance == StyleAppearance::Meter || appearance == StyleAppearance::ProgressBar
        || appearance == StyleAppearance::SquareButton || appearance == StyleAppearance::PushButton
        || appearance == StyleAppearance::SliderHorizontal || appearance == StyleAppearance::Menulist)
    {
      style.SetUsedAppearance(autoAppearance);
      return autoAppearance;
    }

    // The following keywords should work well for some element types
    // even if their default appearances are different from the keywords.
    if (appearance == StyleAppearance::Button)
    {
      if (autoAppearance == StyleAppearance::PushButton || autoAppearance == StyleAppearance::SquareButton)
      {
        return appearance;
      }
      style.SetUsedAppearance(autoAppearance);
      return autoAppearance;
    }

    if (appearance == StyleAppearance::MenulistButton)
    {
      if (autoAppearance == StyleAppearance::Menulist)
      {
        return appearance;
      }
      style.SetUsedAppearance(autoAppearance);
      return autoAppearance;
    }

    auto *inputElement = DynamicDowncast<html::HTMLInputElement>(element);

    if (appearance == StyleAppearance::TextField)
    {
      if (inputElement && inputElement->IsSearchField())
      {
        return appearance;
      }
      style.SetUsedAppearance(autoAppearance);
      return autoAppearance;
    }

    if (appearance == StyleAppearance::SliderVertical)
    {
      if (inputElement && inputElement->IsRangeControl())
      {
        return appearance;
      }
      style.SetUsedAppearance(autoAppearance);
      return autoAppearance;
    }

    return appearance;
  }

  bool RenderTheme::HasAppearanceForElementTypeFromUAStyle(const dom::Element &element) noexcept
  {
    // NOTE: This is just a legacy hard-coded list of elements that have some appearance value in html.css
    // FIXME: Remove when devolvable widgets are universally enabled.
    // FIXME: Also the `::-webkit-list-button` part of user agent shadow trees, with datalists.
    if (!Is<html::HTMLElement>(element))
    {
      return false;
    }
    switch (element.TagQualifiedName().TagName())
    {
      case html::TagName::input:
      case html::TagName::textarea:
      case html::TagName::button:
      case html::TagName::progress:
      case html::TagName::select:
      case html::TagName::meter:    return true;
      default:                      return false;
    }
  }

  void RenderTheme::AdjustStyle(RenderStyle &style, const RenderStyle &parentStyle,
                                const dom::Element *element) noexcept
  {
    auto autoAppearance = AutoAppearanceForElement(style, element);
    auto appearance = AdjustAppearanceForElement(style, parentStyle, element, autoAppearance);
    if (appearance == StyleAppearance::None || appearance == StyleAppearance::Base)
    {
      return;
    }

    // Force inline and table display styles to be inline-block (except for table- which is block)
    auto display = style.Display();
    if (display == DisplayType::Inline || display == DisplayType::InlineTable
        || display == DisplayType::TableRowGroup || display == DisplayType::TableHeaderGroup
        || display == DisplayType::TableFooterGroup || display == DisplayType::TableRow
        || display == DisplayType::TableColumnGroup || display == DisplayType::TableColumn
        || display == DisplayType::TableCell || display == DisplayType::TableCaption)
    {
      style.SetEffectiveDisplay(DisplayType::InlineBlock);
    }
    else if (display == DisplayType::ListItem || display == DisplayType::Table)
    {
      style.SetEffectiveDisplay(DisplayType::Block);
    }

    // FIXME: Devolvable widgets (devolvableWidgetsEnabled, isDevolvableWidget), behind a setting that is off.
    bool hasAppearanceFromUAStyle = element && HasAppearanceForElementTypeFromUAStyle(*element);
    bool widgetHasNativeAppearanceDisabled = hasAppearanceFromUAStyle && IsControlStyled(style);

    if (widgetHasNativeAppearanceDisabled)
    {
      switch (appearance)
      {
        case StyleAppearance::Menulist: appearance = StyleAppearance::MenulistButton; break;
        default:                        appearance = StyleAppearance::None; break;
      }

      style.SetUsedAppearance(appearance);
    }

    if (appearance == StyleAppearance::SearchField
        && SearchFieldShouldAppearAsTextField(style, element->NodeDocument().GetSettings()))
    {
      appearance = StyleAppearance::TextField;
      style.SetUsedAppearance(appearance);
    }

    if (!hasAppearanceFromUAStyle && autoAppearance == StyleAppearance::None
        && !style.BorderAndBackgroundEqual(RenderStyle::DefaultStyleSingleton()))
    {
      style.SetUsedAppearance(StyleAppearance::None);
    }

    if (!style.HasUsedAppearance())
    {
      return;
    }

    if (!SupportsBoxShadow(style))
    {
      style.SetBoxShadow(css::keywords::None {});
    }

    switch (appearance)
    {
      case StyleAppearance::Checkbox:               return AdjustCheckboxStyle(style, element);
      case StyleAppearance::Radio:                  return AdjustRadioStyle(style, element);
      case StyleAppearance::ColorWell:              return AdjustColorWellStyle(style, element);
      case StyleAppearance::ColorWellSwatch:        return AdjustColorWellSwatchStyle(style, element);
      case StyleAppearance::ColorWellSwatchOverlay: return AdjustColorWellSwatchOverlayStyle(style, element);
      case StyleAppearance::ColorWellSwatchWrapper: return AdjustColorWellSwatchWrapperStyle(style, element);
      case StyleAppearance::PushButton:
      case StyleAppearance::SquareButton:
      case StyleAppearance::DefaultButton:
      case StyleAppearance::Button:                 return AdjustButtonStyle(style, element);
      case StyleAppearance::InnerSpinButton:        return AdjustInnerSpinButtonStyle(style, element);
      case StyleAppearance::TextField:              return AdjustTextFieldStyle(style, element);
      case StyleAppearance::TextArea:               return AdjustTextAreaStyle(style, element);
      case StyleAppearance::Menulist:               return AdjustMenuListStyle(style, element);
      case StyleAppearance::MenulistButton:         return AdjustMenuListButtonStyle(style, element);
      case StyleAppearance::SliderHorizontal:
      case StyleAppearance::SliderVertical:         return AdjustSliderTrackStyle(style, element);
      case StyleAppearance::SliderThumbHorizontal:
      case StyleAppearance::SliderThumbVertical:    return AdjustSliderThumbStyle(style, element);
      case StyleAppearance::SearchField:            return AdjustSearchFieldStyle(style, element);
      case StyleAppearance::SearchFieldCancelButton:
        return AdjustSearchFieldCancelButtonStyle(style, element);
      case StyleAppearance::SearchFieldDecoration:
        return AdjustSearchFieldDecorationPartStyle(style, element);
      case StyleAppearance::SearchFieldResultsDecoration:
        return AdjustSearchFieldResultsDecorationPartStyle(style, element);
      case StyleAppearance::SearchFieldResultsButton:
        return AdjustSearchFieldResultsButtonStyle(style, element);
      case StyleAppearance::ProgressBar: return AdjustProgressBarStyle(style, element);
      case StyleAppearance::Meter:       return AdjustMeterStyle(style, element);
      case StyleAppearance::ListButton:  return AdjustListButtonStyle(style, element);
      default:                           break;
    }
  }

  StyleAppearance RenderTheme::AutoAppearanceForElement(RenderStyle &style,
                                                        const dom::Element *elementPtr) const noexcept
  {
    if (!elementPtr)
    {
      return StyleAppearance::None;
    }

    const auto &element = *elementPtr;

    if (const auto *input = DynamicDowncast<html::HTMLInputElement>(element))
    {
      // FIXME: File inputs' buttons (isUploadButton), with file inputs.
      if (input->IsTextButton())
      {
        return StyleAppearance::Button;
      }

      if (input->IsCheckbox())
      {
        return StyleAppearance::Checkbox;
      }

      if (input->IsRadioButton())
      {
        return StyleAppearance::Radio;
      }

      if (input->IsSearchField())
      {
        return StyleAppearance::SearchField;
      }

      if (input->IsDateField() || input->IsDateTimeLocalField() || input->IsMonthField()
          || input->IsTimeField() || input->IsWeekField())
      {
        return StyleAppearance::TextField;
      }

      if (input->IsColorControl())
      {
        return StyleAppearance::ColorWell;
      }

      if (input->IsRangeControl())
      {
        return style.GetWritingMode().IsHorizontal() ? StyleAppearance::SliderHorizontal
                                                     : StyleAppearance::SliderVertical;
      }

      if (input->IsTextField())
      {
        return StyleAppearance::TextField;
      }

      // <input type=hidden|image|file>
      return StyleAppearance::None;
    }

    if (Is<html::HTMLButtonElement>(element))
    {
      return StyleAppearance::Button;
    }

    if (const auto *select = DynamicDowncast<html::HTMLSelectElement>(element))
    {
      return select->UsesMenuList() ? StyleAppearance::Menulist : StyleAppearance::Listbox;
    }

    if (Is<html::HTMLTextAreaElement>(element))
    {
      return StyleAppearance::TextArea;
    }

    if (Is<html::HTMLMeterElement>(element))
    {
      return StyleAppearance::Meter;
    }

    if (Is<html::HTMLProgressElement>(element))
    {
      return StyleAppearance::ProgressBar;
    }

    if (element.IsInUserAgentShadowTree())
    {
      const auto &part = element.UserAgentPart();

      // FIXME: The list button (webkitListButton), with datalists; the colour swatch parts, with colour
      // inputs (G14d4).

      if (part == css::UserAgentParts::WebkitSearchCancelButton())
      {
        return StyleAppearance::SearchFieldCancelButton;
      }

      if (const auto *button = DynamicDowncast<html::SearchFieldResultsButtonElement>(element))
      {
        if (!button->CanAdjustStyleForAppearance())
        {
          return StyleAppearance::None;
        }

        if (part == css::UserAgentParts::WebkitSearchDecoration())
        {
          return StyleAppearance::SearchFieldDecoration;
        }

        if (part == css::UserAgentParts::WebkitSearchResultsDecoration())
        {
          return StyleAppearance::SearchFieldResultsDecoration;
        }

        if (part == css::UserAgentParts::WebkitSearchResultsButton())
        {
          return StyleAppearance::SearchFieldResultsButton;
        }
      }

      if (part == css::UserAgentParts::WebkitSliderThumb())
      {
        return StyleAppearance::SliderThumbHorizontal;
      }

      if (part == css::UserAgentParts::WebkitInnerSpinButton())
      {
        return StyleAppearance::InnerSpinButton;
      }
    }

    return StyleAppearance::None;
  }

  namespace
  {
    void UpdateMeterPartForRenderer(MeterPart &meterPart, const RenderMeter &renderMeter) noexcept
    {
      auto &element = *renderMeter.MeterElement();
      MeterPart::GaugeRegion gaugeRegion {};

      switch (element.GetGaugeRegion())
      {
        case html::HTMLMeterElement::GaugeRegion::Optimum:
          gaugeRegion = MeterPart::GaugeRegion::Optimum;
          break;
        case html::HTMLMeterElement::GaugeRegion::Suboptimal:
          gaugeRegion = MeterPart::GaugeRegion::Suboptimal;
          break;
        case html::HTMLMeterElement::GaugeRegion::EvenLessGood:
          gaugeRegion = MeterPart::GaugeRegion::EvenLessGood;
          break;
      }

      meterPart.SetGaugeRegion(gaugeRegion);
      meterPart.SetValue(element.Value());
      meterPart.SetMinimum(element.Min());
      meterPart.SetMaximum(element.Max());
    }

    void UpdateProgressBarPartForRenderer(ProgressBarPart &progressBarPart,
                                          const RenderProgress &renderProgress) noexcept
    {
      progressBarPart.SetPosition(renderProgress.Position());
      progressBarPart.SetAnimationStartTime(renderProgress.AnimationStartTime());
    }

    void UpdateSliderTrackPartForRenderer(SliderTrackPart &sliderTrackPart,
                                          const RenderElement &renderer) noexcept
    {
      auto &input = Downcast<html::HTMLInputElement>(*renderer.GetElement());
      krys_debug_assert(input.IsRangeControl());

      IntSize thumbSize;
      if (auto *thumbRenderer = input.SliderThumbElement()->Renderer())
      {
        const auto &thumbStyle = thumbRenderer->Style();

        auto fixedWidth = thumbStyle.Width().TryFixed();
        auto fixedHeight = thumbStyle.Height().TryFixed();
        auto thumbWidth =
          fixedWidth ? static_cast<int32>(fixedWidth->ResolveZoom(thumbStyle.UsedZoomForLength())) : 0;
        auto thumbHeight =
          fixedHeight ? static_cast<int32>(fixedHeight->ResolveZoom(thumbStyle.UsedZoomForLength())) : 0;

        thumbSize = {thumbWidth, thumbHeight};
      }

      IntRect trackBounds;
      if (auto *trackRenderer = input.SliderTrackElement()->Renderer())
      {
        trackBounds = trackRenderer->AbsoluteBoundingBoxRectIgnoringTransforms();

        // We can ignoring transforms because transform is handled by the graphics context.
        auto sliderBounds = renderer.AbsoluteBoundingBoxRectIgnoringTransforms();

        // Make position relative to the transformed ancestor element.
        trackBounds.MoveBy(-sliderBounds.Location());
      }

      double minimum = input.Minimum();
      double maximum = input.Maximum();
      double thumbPosition = 0;
      if (maximum > minimum)
      {
        thumbPosition = (input.ValueAsNumber() - minimum) / (maximum - minimum);
      }

      // FIXME: The tick ratios of a data list's suggestions, with datalists.

      sliderTrackPart.SetThumbSize(thumbSize);
      sliderTrackPart.SetTrackBounds(trackBounds);
      sliderTrackPart.SetThumbPosition(thumbPosition);
      sliderTrackPart.SetTickRatios({});
    }
  }

  RefPtr<ControlPart> RenderTheme::CreateControlPart(const RenderElement &renderer) const noexcept
  {
    auto appearance = renderer.Style().UsedAppearance();

    switch (appearance)
    {
      case StyleAppearance::None:
      case StyleAppearance::Auto:
      case StyleAppearance::Base:                         break;

      case StyleAppearance::Checkbox:
      case StyleAppearance::Radio:                        return ToggleButtonPart::Create(appearance);

      case StyleAppearance::PushButton:
      case StyleAppearance::SquareButton:
      case StyleAppearance::Button:
      case StyleAppearance::DefaultButton:                return ButtonPart::Create(appearance);

      case StyleAppearance::Menulist:                     return MenuListPart::Create();

      case StyleAppearance::MenulistButton:               return MenuListButtonPart::Create();

      case StyleAppearance::Meter:                        return MeterPart::Create();

      case StyleAppearance::ProgressBar:                  return ProgressBarPart::Create();

      case StyleAppearance::SliderHorizontal:
      case StyleAppearance::SliderVertical:               return SliderTrackPart::Create(appearance);

      case StyleAppearance::SearchField:                  return SearchFieldPart::Create();

      case StyleAppearance::Listbox:
      case StyleAppearance::TextArea:                     return TextAreaPart::Create(appearance);

      case StyleAppearance::TextField:                    return TextFieldPart::Create();

      case StyleAppearance::ColorWellSwatch:
      case StyleAppearance::ColorWellSwatchOverlay:
      case StyleAppearance::ColorWellSwatchWrapper:       break;

      case StyleAppearance::ColorWell:                    return ColorWellPart::Create();

      case StyleAppearance::InnerSpinButton:              return InnerSpinButtonPart::Create();

      case StyleAppearance::ListButton:                   break;

      case StyleAppearance::SearchFieldDecoration:        break;

      case StyleAppearance::SearchFieldResultsDecoration:
      case StyleAppearance::SearchFieldResultsButton:     return SearchFieldResultsPart::Create(appearance);

      case StyleAppearance::SearchFieldCancelButton:      return SearchFieldCancelButtonPart::Create();

      case StyleAppearance::SliderThumbHorizontal:
      case StyleAppearance::SliderThumbVertical:          return SliderThumbPart::Create(appearance);

      case StyleAppearance::Switch:
      case StyleAppearance::SwitchThumb:
      case StyleAppearance::SwitchTrack:                  break;
    }

    return nullptr;
  }

  void RenderTheme::UpdateControlPartForRenderer(ControlPart &part,
                                                 const RenderElement &renderer) const noexcept
  {
    if (auto *meterPart = DynamicDowncast<MeterPart>(part))
    {
      UpdateMeterPartForRenderer(*meterPart, Downcast<RenderMeter>(renderer));
      return;
    }

    if (auto *progressBarPart = DynamicDowncast<ProgressBarPart>(part))
    {
      UpdateProgressBarPartForRenderer(*progressBarPart, Downcast<RenderProgress>(renderer));
      return;
    }

    if (auto *sliderTrackPart = DynamicDowncast<SliderTrackPart>(part))
    {
      UpdateSliderTrackPartForRenderer(*sliderTrackPart, renderer);
      return;
    }
  }

  EnumSet<ControlStyle::State>
    RenderTheme::ExtractControlStyleStatesForRendererInternal(const RenderElement &renderer) const noexcept
  {
    EnumSet<ControlStyle::State> states;
    if (IsHovered(renderer))
    {
      states.Add(ControlStyle::State::Hovered);
      if (IsSpinUpButtonPartHovered(renderer))
      {
        states.Add(ControlStyle::State::SpinUp);
      }
    }
    if (IsPressed(renderer))
    {
      states.Add(ControlStyle::State::Pressed);
      if (IsSpinUpButtonPartPressed(renderer))
      {
        states.Add(ControlStyle::State::SpinUp);
      }
    }
    if (IsFocused(renderer) && renderer.Style().GetOutlineStyle() == OutlineStyle::Auto)
    {
      states.Add(ControlStyle::State::Focused);
    }
    if (IsEnabled(renderer))
    {
      states.Add(ControlStyle::State::Enabled);
    }
    if (IsChecked(renderer))
    {
      states.Add(ControlStyle::State::Checked);
    }
    if (IsDefault(renderer))
    {
      states.Add(ControlStyle::State::Default);
    }
    if (IsWindowActive(renderer))
    {
      states.Add(ControlStyle::State::WindowActive);
    }
    if (IsIndeterminate(renderer))
    {
      states.Add(ControlStyle::State::Indeterminate);
    }
    if (IsPresenting(renderer))
    {
      states.Add(ControlStyle::State::Presenting);
    }
    if (UseFormSemanticContext())
    {
      states.Add(ControlStyle::State::FormSemanticContext);
    }
    if (renderer.UseDarkAppearance())
    {
      states.Add(ControlStyle::State::DarkAppearance);
    }
    if (renderer.GetWritingMode().IsInlineFlipped())
    {
      states.Add(ControlStyle::State::InlineFlippedWritingMode);
    }
    if (SupportsLargeFormControls())
    {
      states.Add(ControlStyle::State::LargeControls);
    }
    if (IsReadOnlyControl(renderer))
    {
      states.Add(ControlStyle::State::ReadOnly);
    }
    if (HasListButton(renderer))
    {
      states.Add(ControlStyle::State::ListButton);
      if (HasListButtonPressed(renderer))
      {
        states.Add(ControlStyle::State::ListButtonPressed);
      }
    }
    if (!renderer.GetWritingMode().IsHorizontal())
    {
      states.Add(ControlStyle::State::VerticalWritingMode);
    }
    return states;
  }

  namespace
  {
    KRYS_NODISCARD const RenderElement *
      EffectiveRendererForAppearance(const RenderObject &renderObject) noexcept
    {
      auto *renderer = DynamicDowncast<RenderElement>(renderObject);
      if (!renderer)
      {
        krys_debug_assert(false);
        return {};
      }

      auto type = renderer->Style().UsedAppearance();
      if (type == StyleAppearance::SearchFieldCancelButton)
      {
        auto *element = renderer->GetElement();
        dom::Node *input = dom::TreeQueries::ShadowHost(*element);
        if (!input)
        {
          input = element;
        }

        return DynamicDowncast<RenderBox>(input->Renderer());
      }
      return renderer;
    }
  }

  EnumSet<ControlStyle::State>
    RenderTheme::ExtractControlStyleStatesForRenderer(const RenderObject &renderObject) const noexcept
  {
    if (const auto *renderer = EffectiveRendererForAppearance(renderObject))
    {
      return ExtractControlStyleStatesForRendererInternal(*renderer);
    }
    return {};
  }

  ControlStyle RenderTheme::ExtractControlStyleForRenderer(const RenderElement &renderObject) const noexcept
  {
    const auto *renderer = EffectiveRendererForAppearance(renderObject);
    if (!renderer)
    {
      return {};
    }

    const auto &style = renderer->Style();
    return {
      ExtractControlStyleStatesForRendererInternal(*renderer),
      style.ComputedFontSize(),
      style.UsedZoom(),
      style.UsedAccentColor(renderObject.StyleColorOptions()),
      style.VisitedDependentColorWithColorFilter(css::PropertyId::Color),
      style::Evaluate<FloatBoxExtent>(style.BorderWidth(), style::ZoomNeeded {}),
    };
  }

  bool RenderTheme::Paint(const RenderBox &box, ControlPart &part, const PaintInfo &paintInfo,
                          const LayoutRect &rect) noexcept
  {
    // If painting is disabled, but we aren't updating control tints, then just bail.
    // If we are updating control tints, just schedule a repaint if the theme supports tinting
    // for that control.
    if (paintInfo.Context().InvalidatingControlTints())
    {
      if (ControlSupportsTints(box))
      {
        box.Repaint();
      }
      return false;
    }

    if (paintInfo.Context().PaintingDisabled())
    {
      return false;
    }

    UpdateControlPartForRenderer(part, box);

    float deviceScaleFactor = box.GetDocument().DeviceScaleFactor();
    auto zoomedRect = SnapRectToDevicePixels(rect, deviceScaleFactor);
    auto borderShape = BorderShape::ShapeForBorderRect(box.Style(), LayoutRect(zoomedRect));
    auto controlStyle = ExtractControlStyleForRenderer(box);
    auto &context = paintInfo.Context();

    context.DrawControlPart(part, borderShape.DeprecatedPixelSnappedRoundedRect(deviceScaleFactor),
                            deviceScaleFactor, controlStyle);
    return false;
  }

  bool RenderTheme::Paint(const RenderBox &box, const PaintInfo &paintInfo, const LayoutRect &rect) noexcept
  {
    // If painting is disabled, but we aren't updating control tints, then just bail.
    // If we are updating control tints, just schedule a repaint if the theme supports tinting
    // for that control.
    if (paintInfo.Context().InvalidatingControlTints())
    {
      if (ControlSupportsTints(box))
      {
        box.Repaint();
      }
      return false;
    }
    if (paintInfo.Context().PaintingDisabled())
    {
      return false;
    }

    auto appearance = box.Style().UsedAppearance();

    if (!CanPaint(paintInfo, box.GetDocument().GetSettings(), appearance)) KRYS_UNLIKELY
    {
      return false;
    }

    float deviceScaleFactor = box.GetDocument().DeviceScaleFactor();
    FloatRect devicePixelSnappedRect = SnapRectToDevicePixels(rect, deviceScaleFactor);

    switch (appearance)
    {
      case StyleAppearance::Checkbox:  return PaintCheckbox(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::Radio:     return PaintRadio(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::ColorWell: return PaintColorWell(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::ColorWellSwatch:
        return PaintColorWellSwatch(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::PushButton:
      case StyleAppearance::SquareButton:
      case StyleAppearance::DefaultButton:
      case StyleAppearance::Button:        return PaintButton(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::Menulist:      return PaintMenuList(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::MenulistButton:
        return PaintMenuListButton(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::Meter:            return PaintMeter(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::ProgressBar:      return PaintProgressBar(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::SliderHorizontal:
      case StyleAppearance::SliderVertical:   return PaintSliderTrack(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::SliderThumbHorizontal:
      case StyleAppearance::SliderThumbVertical:
        return PaintSliderThumb(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::TextField: return PaintTextField(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::TextArea:  return PaintTextArea(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::Listbox:   return true;
      case StyleAppearance::InnerSpinButton:
        return PaintInnerSpinButton(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::SearchField: return PaintSearchField(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::SearchFieldCancelButton:
        return PaintSearchFieldCancelButton(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::SearchFieldDecoration:
        return PaintSearchFieldDecorationPart(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::SearchFieldResultsDecoration:
        return PaintSearchFieldResultsDecorationPart(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::SearchFieldResultsButton:
        return PaintSearchFieldResultsButton(box, paintInfo, devicePixelSnappedRect);
      case StyleAppearance::ListButton: return PaintListButton(box, paintInfo, devicePixelSnappedRect);
      default:                          break;
    }

    return true; // We don't support the appearance, so let the normal background/border paint.
  }

  bool RenderTheme::PaintBorderOnly(const RenderBox &box, const PaintInfo &paintInfo) noexcept
  {
    if (paintInfo.Context().PaintingDisabled())
    {
      return false;
    }

    // Call the appropriate paint method based off the appearance value.
    switch (box.Style().UsedAppearance())
    {
      case StyleAppearance::TextField:
      case StyleAppearance::Listbox:
      case StyleAppearance::TextArea:
      case StyleAppearance::MenulistButton:
      case StyleAppearance::SearchField:    return true;
      default:                              break;
    }

    return false;
  }

  void RenderTheme::PaintDecorations(const RenderBox &box, const PaintInfo &paintInfo,
                                     const LayoutRect &rect) noexcept
  {
    if (paintInfo.Context().PaintingDisabled())
    {
      return;
    }

    FloatRect devicePixelSnappedRect = SnapRectToDevicePixels(rect, box.GetDocument().DeviceScaleFactor());

    // Call the appropriate paint method based off the appearance value.
    switch (box.Style().UsedAppearance())
    {
      case StyleAppearance::MenulistButton:
        PaintMenuListButtonDecorations(box, paintInfo, devicePixelSnappedRect);
        break;
      case StyleAppearance::TextField:
        PaintTextFieldDecorations(box, paintInfo, devicePixelSnappedRect);
        break;
      case StyleAppearance::TextArea: PaintTextAreaDecorations(box, paintInfo, devicePixelSnappedRect); break;
      case StyleAppearance::ColorWell:
        PaintColorWellDecorations(box, paintInfo, devicePixelSnappedRect);
        break;
      case StyleAppearance::Menulist: PaintMenuListDecorations(box, paintInfo, devicePixelSnappedRect); break;
      case StyleAppearance::SliderThumbHorizontal:
      case StyleAppearance::SearchField:
        PaintSearchFieldDecorations(box, paintInfo, devicePixelSnappedRect);
        break;
      default: break;
    }
  }

  Color RenderTheme::ActiveSelectionBackgroundColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.ActiveSelectionBackgroundColor.IsValid())
    {
      cache.ActiveSelectionBackgroundColor =
        TransformSelectionBackgroundColor(PlatformActiveSelectionBackgroundColor(options), options);
    }
    return cache.ActiveSelectionBackgroundColor;
  }

  Color RenderTheme::InactiveSelectionBackgroundColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.InactiveSelectionBackgroundColor.IsValid())
    {
      cache.InactiveSelectionBackgroundColor =
        TransformSelectionBackgroundColor(PlatformInactiveSelectionBackgroundColor(options), options);
    }
    return cache.InactiveSelectionBackgroundColor;
  }

  Color RenderTheme::TransformSelectionBackgroundColor(const Color &color, style::ColorOptions) const noexcept
  {
    return BlendWithWhite(color);
  }

  Color RenderTheme::ActiveSelectionForegroundColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.ActiveSelectionForegroundColor.IsValid() && SupportsSelectionForegroundColors(options))
    {
      cache.ActiveSelectionForegroundColor = PlatformActiveSelectionForegroundColor(options);
    }
    return cache.ActiveSelectionForegroundColor;
  }

  Color RenderTheme::InactiveSelectionForegroundColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.InactiveSelectionForegroundColor.IsValid() && SupportsSelectionForegroundColors(options))
    {
      cache.InactiveSelectionForegroundColor = PlatformInactiveSelectionForegroundColor(options);
    }
    return cache.InactiveSelectionForegroundColor;
  }

  Color RenderTheme::ActiveListBoxSelectionBackgroundColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.ActiveListBoxSelectionBackgroundColor.IsValid())
    {
      cache.ActiveListBoxSelectionBackgroundColor = PlatformActiveListBoxSelectionBackgroundColor(options);
    }
    return cache.ActiveListBoxSelectionBackgroundColor;
  }

  Color RenderTheme::InactiveListBoxSelectionBackgroundColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.InactiveListBoxSelectionBackgroundColor.IsValid())
    {
      cache.InactiveListBoxSelectionBackgroundColor =
        PlatformInactiveListBoxSelectionBackgroundColor(options);
    }
    return cache.InactiveListBoxSelectionBackgroundColor;
  }

  Color RenderTheme::ActiveListBoxSelectionForegroundColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.ActiveListBoxSelectionForegroundColor.IsValid()
        && SupportsListBoxSelectionForegroundColors(options))
    {
      cache.ActiveListBoxSelectionForegroundColor = PlatformActiveListBoxSelectionForegroundColor(options);
    }
    return cache.ActiveListBoxSelectionForegroundColor;
  }

  Color RenderTheme::InactiveListBoxSelectionForegroundColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.InactiveListBoxSelectionForegroundColor.IsValid()
        && SupportsListBoxSelectionForegroundColors(options))
    {
      cache.InactiveListBoxSelectionForegroundColor =
        PlatformInactiveListBoxSelectionForegroundColor(options);
    }
    return cache.InactiveListBoxSelectionForegroundColor;
  }

  Color RenderTheme::PlatformActiveSelectionBackgroundColor(style::ColorOptions) const noexcept
  {
    // Use a blue color by default if the platform theme doesn't define anything.
    return Color::blue;
  }

  Color RenderTheme::PlatformActiveSelectionForegroundColor(style::ColorOptions) const noexcept
  {
    // Use a white color by default if the platform theme doesn't define anything.
    return Color::white;
  }

  Color RenderTheme::PlatformInactiveSelectionBackgroundColor(style::ColorOptions) const noexcept
  {
    // Use a grey color by default if the platform theme doesn't define anything.
    // This color matches Firefox's inactive color.
    return SRGBA<uint8> {176, 176, 176};
  }

  Color RenderTheme::PlatformInactiveSelectionForegroundColor(style::ColorOptions) const noexcept
  {
    // Use a black color by default.
    return Color::black;
  }

  Color RenderTheme::PlatformActiveListBoxSelectionBackgroundColor(style::ColorOptions options) const noexcept
  {
    return PlatformActiveSelectionBackgroundColor(options);
  }

  Color RenderTheme::PlatformActiveListBoxSelectionForegroundColor(style::ColorOptions options) const noexcept
  {
    return PlatformActiveSelectionForegroundColor(options);
  }

  Color
    RenderTheme::PlatformInactiveListBoxSelectionBackgroundColor(style::ColorOptions options) const noexcept
  {
    return PlatformInactiveSelectionBackgroundColor(options);
  }

  Color
    RenderTheme::PlatformInactiveListBoxSelectionForegroundColor(style::ColorOptions options) const noexcept
  {
    return PlatformInactiveSelectionForegroundColor(options);
  }

  int32 RenderTheme::BaselinePosition(const RenderBox &box) const noexcept
  {
    return box.IsHorizontalWritingMode() ? box.Height().ToInt() : LayoutUnit(box.Width() / 2.0f).ToInt();
  }

  bool RenderTheme::IsControlContainer(StyleAppearance appearance) const noexcept
  {
    // There are more leaves than this, but we'll patch this function as we add support for
    // more controls.
    return appearance != StyleAppearance::Checkbox && appearance != StyleAppearance::Radio;
  }

  bool RenderTheme::IsControlStyled(const RenderStyle &style) const noexcept
  {
    switch (style.UsedAppearance())
    {
      case StyleAppearance::PushButton:
      case StyleAppearance::SquareButton:
      case StyleAppearance::ColorWell:
      case StyleAppearance::DefaultButton:
      case StyleAppearance::Button:
      case StyleAppearance::Listbox:
      case StyleAppearance::Menulist:
      case StyleAppearance::ProgressBar:
      case StyleAppearance::Meter:
      // FIXME: SearchFieldPart should be included here when making search fields style-able.
      case StyleAppearance::TextField:
      case StyleAppearance::TextArea:
        // Test the style to see if the UA border and background match.
        return style.NativeAppearanceDisabled();
      default: return false;
    }
  }

  bool RenderTheme::SupportsFocusRing(const RenderElement &, const RenderStyle &style) const noexcept
  {
    return style.HasUsedAppearance() && style.UsedAppearance() != StyleAppearance::TextField
           && style.UsedAppearance() != StyleAppearance::TextArea
           && style.UsedAppearance() != StyleAppearance::MenulistButton
           && style.UsedAppearance() != StyleAppearance::Listbox;
  }

  bool RenderTheme::IsWindowActive(const RenderElement &renderer) const noexcept
  {
    return renderer.GetDocument().IsFocusedAndActive();
  }

  bool RenderTheme::IsChecked(const RenderElement &renderer) const noexcept
  {
    auto *element = DynamicDowncast<html::HTMLInputElement>(renderer.GetElement());
    return element && element->MatchesCheckedPseudoClass();
  }

  bool RenderTheme::IsIndeterminate(const RenderElement &renderer) const noexcept
  {
    // This does not currently support multiple elements and therefore radio buttons are excluded.
    // FIXME: However, what about <progress>?
    auto *input = DynamicDowncast<html::HTMLInputElement>(renderer.GetElement());
    return input && input->IsCheckbox() && css::MatchesIndeterminatePseudoClass(*input);
  }

  bool RenderTheme::IsEnabled(const RenderElement &renderer) const noexcept
  {
    auto *element = renderer.GetElement();
    return element && !element->IsDisabledFormControl();
  }

  bool RenderTheme::IsFocused(const RenderElement &renderer) const noexcept
  {
    auto *element = renderer.GetElement();
    if (!element)
    {
      return false;
    }

    // FIXME: This should be part of RenderTheme::extractControlStyleForRenderer().
    dom::Element *delegate = element;
    if (auto *sliderThumb = DynamicDowncast<html::SliderThumbElement>(element))
    {
      delegate = sliderThumb->HostInput();
    }

    auto &document = delegate->NodeDocument();
    return delegate == document.FocusedElement() && document.Selection().IsFocusedAndActive();
  }

  bool RenderTheme::IsPressed(const RenderElement &renderer) const noexcept
  {
    auto *element = renderer.GetElement();
    return element && element->Active();
  }

  bool RenderTheme::IsSpinUpButtonPartPressed(const RenderElement &renderer) const noexcept
  {
    if (auto *spinButton = DynamicDowncast<html::SpinButtonElement>(renderer.GetElement()))
    {
      return spinButton->Active() && spinButton->GetUpDownState() == html::SpinButtonElement::UpDownState::Up;
    }
    return false;
  }

  bool RenderTheme::IsReadOnlyControl(const RenderElement &renderer) const noexcept
  {
    if (auto *element = renderer.GetElement())
    {
      return Is<html::HTMLFormControlElement>(*element) && !css::MatchesReadWritePseudoClass(*element);
    }
    return false;
  }

  bool RenderTheme::IsHovered(const RenderElement &renderer) const noexcept
  {
    if (auto *spinButton = DynamicDowncast<html::SpinButtonElement>(renderer.GetElement()))
    {
      return spinButton->Hovered()
             && spinButton->GetUpDownState() != html::SpinButtonElement::UpDownState::Indeterminate;
    }
    if (auto *element = renderer.GetElement())
    {
      return element->Hovered();
    }
    return false;
  }

  bool RenderTheme::IsSpinUpButtonPartHovered(const RenderElement &renderer) const noexcept
  {
    if (auto *spinButton = DynamicDowncast<html::SpinButtonElement>(renderer.GetElement()))
    {
      return spinButton->GetUpDownState() == html::SpinButtonElement::UpDownState::Up;
    }
    return false;
  }

  bool RenderTheme::IsPresenting(const RenderElement &) const noexcept
  {
    // FIXME: Inputs presenting a picker (isPresentingAttachedView), with the pickers (G14d4).
    return false;
  }

  bool RenderTheme::IsDefault(const RenderElement &renderer) const noexcept
  {
    // A button should only have the default appearance if the page is active
    if (!IsWindowActive(renderer))
    {
      return false;
    }

    return renderer.Style().UsedAppearance() == StyleAppearance::DefaultButton;
  }

  bool RenderTheme::HasListButton(const RenderElement &) const noexcept
  {
    // FIXME: Inputs with data lists (hasDataList), with datalists.
    return false;
  }

  bool RenderTheme::HasListButtonPressed(const RenderElement &) const noexcept
  {
    // FIXME: The data list button (dataListButtonElement), with datalists.
    return false;
  }

  Maybe<FontCascadeDescription> RenderTheme::ControlFont(StyleAppearance, const FontCascade &,
                                                         float) const noexcept
  {
    return null;
  }

  style::PaddingBox RenderTheme::ControlPadding(StyleAppearance appearance, const style::PaddingBox &padding,
                                                float) const noexcept
  {
    switch (appearance)
    {
      case StyleAppearance::Menulist:
      case StyleAppearance::MenulistButton:
      case StyleAppearance::Checkbox:
      case StyleAppearance::Radio:          return style::PaddingBox {style::PaddingEdge::Fixed {0}};
      default:                              return padding;
    }
  }

  style::PreferredSizePair RenderTheme::ControlSize(StyleAppearance, const FontCascade &,
                                                    const style::PreferredSizePair &zoomedSize,
                                                    float) const noexcept
  {
    return zoomedSize;
  }

  style::MinimumSizePair RenderTheme::MinimumControlSize(StyleAppearance, const FontCascade &,
                                                         const style::MinimumSizePair &, float) const noexcept
  {
    return {style::MinimumSize::Fixed {0}, style::MinimumSize::Fixed {0}};
  }

  style::MinimumSizePair RenderTheme::MinimumControlSize(StyleAppearance appearance,
                                                         const FontCascade &fontCascade,
                                                         const style::MinimumSizePair &minSize,
                                                         const style::PreferredSizePair &preferredSize,
                                                         float zoom) const noexcept
  {
    auto minimumControlSize = MinimumControlSize(appearance, fontCascade, minSize, zoom);

    auto resultWidth = minimumControlSize.Width();
    auto resultHeight = minimumControlSize.Height();

    // Other StyleAppearance types are composed controls with shadow subtree.
    if (appearance == StyleAppearance::Radio || appearance == StyleAppearance::Checkbox)
    {
      if (minSize.Width().IsIntrinsicOrLegacyIntrinsicOrAuto())
      {
        resultWidth = preferredSize.Width().AsMinimumSize();
      }
      if (minSize.Height().IsIntrinsicOrLegacyIntrinsicOrAuto())
      {
        resultHeight = preferredSize.Height().AsMinimumSize();
      }
    }

    return {krys::move(resultWidth), krys::move(resultHeight)};
  }

  style::LineWidthBox RenderTheme::ControlBorder(StyleAppearance appearance, const FontCascade &,
                                                 const style::LineWidthBox &zoomedBox, float,
                                                 const dom::Element *) const noexcept
  {
    switch (appearance)
    {
      case StyleAppearance::PushButton:
      case StyleAppearance::Menulist:
      case StyleAppearance::SearchField:
      case StyleAppearance::Checkbox:
      case StyleAppearance::Radio:       return style::LineWidthBox {style::LineWidth {0_css_px}};
      default:                           return zoomedBox;
    }
  }

  float RenderTheme::UsedZoomForComputedStyle(const RenderStyle &renderStyle) const noexcept
  {
    return renderStyle.EvaluationTimeZoomEnabled() ? 1.0f : renderStyle.UsedZoom();
  }

  namespace
  {
    // Raises a minimum size to the theme's, as the original code did; the comparisons between fixed and
    // percentage sizes make little sense but match the existing behaviour.
    // FIXME: The min-width/min-height value should use `calc-size()` when supported to make non-specified
    // overrides work.
    KRYS_NODISCARD bool ShouldOverrideMinimumSize(const style::MinimumSize &override,
                                                  const style::MinimumSize &original,
                                                  style::ZoomFactor usedZoomForLength) noexcept
    {
      if (auto fixedOverride = override.TryFixed())
      {
        if (auto fixedOriginal = original.TryFixed())
        {
          return fixedOverride->ResolveZoom(usedZoomForLength)
                 > fixedOriginal->ResolveZoom(usedZoomForLength);
        }
        if (auto percentageOriginal = original.TryPercentage())
        {
          return fixedOverride->ResolveZoom(usedZoomForLength) > percentageOriginal->Value;
        }
        return fixedOverride->IsPositive();
      }
      if (auto percentageOverride = override.TryPercentage())
      {
        if (auto fixedOriginal = original.TryFixed())
        {
          return percentageOverride->Value > fixedOriginal->ResolveZoom(usedZoomForLength);
        }
        if (auto percentageOriginal = original.TryPercentage())
        {
          return percentageOverride->Value > percentageOriginal->Value;
        }
        return percentageOverride->IsPositive();
      }
      return false;
    }
  }

  // FIXME: iOS does not use this so arguably this should be better abstracted. Or maybe we should
  // investigate if we can bring the various ports closer together.
  void RenderTheme::AdjustButtonOrCheckboxOrColorWellOrInnerSpinButtonOrRadioStyle(
    RenderStyle &style, const dom::Element *element) const noexcept
  {
    auto appearance = style.UsedAppearance();
    const auto &fontCascade = style.GetFontCascade();

    auto borderBox = ControlBorder(appearance, fontCascade, style.BorderWidth(), style.UsedZoom(), element);

    auto supportsVerticalWritingMode = [](StyleAppearance appearance)
    {
      return appearance == StyleAppearance::Button || appearance == StyleAppearance::ColorWell
             || appearance == StyleAppearance::DefaultButton || appearance == StyleAppearance::SquareButton
             || appearance == StyleAppearance::PushButton;
    };
    // Transpose for vertical writing mode:
    if (!style.GetWritingMode().IsHorizontal() && supportsVerticalWritingMode(appearance))
    {
      borderBox =
        style::LineWidthBox {borderBox.Left(), borderBox.Top(), borderBox.Right(), borderBox.Bottom()};
    }

    if (style::Evaluate<float>(borderBox.Top(), style::ZoomNeeded {})
        != style::Evaluate<int32>(style.BorderTopWidth(), style::ZoomNeeded {}))
    {
      if (!borderBox.Top().IsZero())
      {
        style.SetBorderTopWidth(borderBox.Top());
      }
      else
      {
        style.ResetBorderTop();
      }
    }
    if (style::Evaluate<float>(borderBox.Right(), style::ZoomNeeded {})
        != style::Evaluate<int32>(style.BorderRightWidth(), style::ZoomNeeded {}))
    {
      if (!borderBox.Right().IsZero())
      {
        style.SetBorderRightWidth(borderBox.Right());
      }
      else
      {
        style.ResetBorderRight();
      }
    }
    if (style::Evaluate<float>(borderBox.Bottom(), style::ZoomNeeded {})
        != style::Evaluate<int32>(style.BorderBottomWidth(), style::ZoomNeeded {}))
    {
      style.SetBorderBottomWidth(borderBox.Bottom());
      if (!borderBox.Bottom().IsZero())
      {
        style.SetBorderBottomWidth(borderBox.Bottom());
      }
      else
      {
        style.ResetBorderBottom();
      }
    }
    if (style::Evaluate<float>(borderBox.Left(), style::ZoomNeeded {})
        != style::Evaluate<int32>(style.BorderLeftWidth(), style::ZoomNeeded {}))
    {
      style.SetBorderLeftWidth(borderBox.Left());
      if (!borderBox.Left().IsZero())
      {
        style.SetBorderLeftWidth(borderBox.Left());
      }
      else
      {
        style.ResetBorderLeft();
      }
    }

    // Padding
    auto paddingBox = ControlPadding(appearance, style.GetPaddingBox(), style.UsedZoom());
    if (paddingBox != style.GetPaddingBox())
    {
      style.SetPaddingBox(krys::move(paddingBox));
    }

    // Whitespace
    if (ControlRequiresPreWhiteSpace(appearance))
    {
      style.SetWhiteSpaceCollapse(WhiteSpaceCollapse::Preserve);
      style.SetTextWrapMode(TextWrapMode::NoWrap);
    }

    // Width / Height
    // The width and height here are affected by the zoom.
    // FIXME: Check is flawed, since it doesn't take min-width/max-width into account.
    auto zoomForDeterminingControlSize = UsedZoomForComputedStyle(style);
    auto controlSize =
      ControlSize(appearance, fontCascade, {style.Width(), style.Height()}, zoomForDeterminingControlSize);
    if (controlSize.Width() != style.Width())
    {
      style.SetWidth(style::PreferredSize {controlSize.Width()});
    }
    if (controlSize.Height() != style.Height())
    {
      style.SetHeight(style::PreferredSize {controlSize.Height()});
    }

    // Min-Width / Min-Height
    auto minimumControlSize =
      MinimumControlSize(appearance, fontCascade, {style.MinWidth(), style.MinHeight()},
                         {style.Width(), style.Height()}, zoomForDeterminingControlSize);

    auto usedZoomForLength = style.UsedZoomForLength();
    if (ShouldOverrideMinimumSize(minimumControlSize.Width(), style.MinWidth(), usedZoomForLength))
    {
      style.SetMinWidth(style::MinimumSize(minimumControlSize.Width()));
    }
    if (ShouldOverrideMinimumSize(minimumControlSize.Height(), style.MinHeight(), usedZoomForLength))
    {
      style.SetMinHeight(style::MinimumSize(minimumControlSize.Height()));
    }

    // Font
    if (auto controlFont = ControlFont(appearance, fontCascade, style.UsedZoom()))
    {
      // If overriding the specified font with the theme font, also override the line height with the standard
      // line height.
      style.SetLineHeight(style::ComputedStyle::InitialLineHeight());
      style.SetFontDescription(krys::move(controlFont.value()));
    }

    // Special style that tells enabled default buttons in active windows to use the ActiveButtonText color.
    // The active window part of the test has to be done at paint time since it's not triggered by a style
    // change.
    style.SetInsideDefaultButton(appearance == StyleAppearance::DefaultButton && element
                                 && !element->IsDisabledFormControl());
  }

  void RenderTheme::AdjustCheckboxStyle(RenderStyle &style, const dom::Element *element) const noexcept
  {
    AdjustButtonOrCheckboxOrColorWellOrInnerSpinButtonOrRadioStyle(style, element);
  }

  void RenderTheme::AdjustRadioStyle(RenderStyle &style, const dom::Element *element) const noexcept
  {
    AdjustButtonOrCheckboxOrColorWellOrInnerSpinButtonOrRadioStyle(style, element);
  }

  void RenderTheme::AdjustColorWellStyle(RenderStyle &style, const dom::Element *element) const noexcept
  {
    AdjustButtonOrCheckboxOrColorWellOrInnerSpinButtonOrRadioStyle(style, element);
  }

  void RenderTheme::AdjustButtonStyle(RenderStyle &style, const dom::Element *element) const noexcept
  {
    AdjustButtonOrCheckboxOrColorWellOrInnerSpinButtonOrRadioStyle(style, element);
  }

  void RenderTheme::AdjustInnerSpinButtonStyle(RenderStyle &style, const dom::Element *element) const noexcept
  {
    AdjustButtonOrCheckboxOrColorWellOrInnerSpinButtonOrRadioStyle(style, element);
  }

  void RenderTheme::AdjustMenuListStyle(RenderStyle &style, const dom::Element *) const noexcept
  {
    style.SetOverflowX(Overflow::Visible);
    style.SetOverflowY(Overflow::Visible);
  }

  void RenderTheme::AdjustMeterStyle(RenderStyle &style, const dom::Element *) const noexcept
  {
    style.SetBoxShadow(css::keywords::None {});
  }

  FloatSize RenderTheme::MeterSizeForBounds(const RenderMeter &, const FloatRect &bounds) const noexcept
  {
    return bounds.Size();
  }

  void RenderTheme::PaintSliderTicks(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
  {
    // FIXME: The ticks of a range's data list (paintSliderTicks), with datalists.
  }

  void RenderTheme::PaintPlatformResizer(const RenderLayerModelObject &renderer, GraphicsContext &context,
                                         const LayoutRect &resizerCornerRect) noexcept
  {
    // The platform's resize corner image: two diagonal lines, each a dark line with a light one below it,
    // 8 pixels square, drawn a pixel at a time.
    // FIXME: The platform image (textAreaResizeCorner), and its 2x version, once the theme has images.
    constexpr float cornerResizerSize = 8;
    auto origin =
      FloatPoint(resizerCornerRect.MaxXMaxYCorner()) - FloatSize(cornerResizerSize, cornerResizerSize);
    bool flip = renderer.Style().ShouldPlaceVerticalScrollbarOnLeft();
    if (flip)
    {
      origin =
        FloatPoint(resizerCornerRect.X().ToFloat(), resizerCornerRect.MaxY().ToFloat() - cornerResizerSize);
    }

    auto dark = Color(SRGBA<uint8> {0, 0, 0, 0x99});
    auto light = Color(SRGBA<uint8> {255, 255, 255, 0x99});
    auto pixel = [&](int32 x, int32 y, const Color &color)
    {
      auto column = flip ? static_cast<int32>(cornerResizerSize) - 1 - x : x;
      context.FillRect(FloatRect(origin.X() + column, origin.Y() + y, 1, 1), color);
    };
    for (int32 y = 0; y < 7; ++y)
    {
      // The long line, from (6, 0) to (0, 6).
      pixel(6 - y, y, dark);
      if (6 - y + 1 < 7)
      {
        pixel(6 - y + 1, y, light);
      }
      // The short line, from (6, 4) to (4, 6).
      if (y >= 4)
      {
        pixel(10 - y, y, dark);
        if (10 - y + 1 < 7)
        {
          pixel(10 - y + 1, y, light);
        }
      }
    }
  }

  void RenderTheme::PaintPlatformResizerFrame(const RenderLayerModelObject &, GraphicsContext &context,
                                              const LayoutRect &resizerAbsRect) noexcept
  {
    // Clipping will exclude the right and bottom edges of this frame.
    GraphicsContextStateSaver stateSaver(context);
    context.Clip(FloatRect(resizerAbsRect));
    LayoutRect largerCorner = resizerAbsRect;
    largerCorner.SetSize(LayoutSize(largerCorner.Width() + 1_lu, largerCorner.Height() + 1_lu));
    context.SetStrokeColor(Color(SRGBA<uint8> {217, 217, 217}));
    context.SetStrokeThickness(1.0f);
    context.SetFillColor(Color::transparentBlack);
    context.DrawRect(FloatRect(SnappedIntRect(largerCorner)));
  }

  bool RenderTheme::ShouldHaveSpinButton(const html::HTMLInputElement &inputElement) const noexcept
  {
    return inputElement.IsSteppable() && !inputElement.IsRangeControl();
  }

  void RenderTheme::SetColorWellSwatchBackground(html::HTMLElement &swatch, Color color) noexcept
  {
    if (!color.IsOpaque())
    {
      color = BlendSourceOver(Color::white, color);
    }
    (void)swatch.SetInlineStyleProperty(css::PropertyId::BackgroundColor, SerialisationForHTML(color));
  }

  void RenderTheme::AdjustSliderThumbStyle(RenderStyle &style, const dom::Element *element) const noexcept
  {
    AdjustSliderThumbSize(style, element);
  }

  style::PaddingBox RenderTheme::PopupInternalPaddingBox(const RenderStyle &) const noexcept
  {
    return style::PaddingBox {style::PaddingEdge::Fixed {0}};
  }

  void RenderTheme::PurgeCaches() noexcept
  {
    _colorCacheMap.clear();
  }

  void RenderTheme::PlatformColorsDidChange() noexcept
  {
    _colorCacheMap.clear();

    // FIXME: Restyling every page after a change in the environment
    // (Page::updateStyleForAllPagesAfterGlobalChangeInEnvironment), once pages are tracked.
  }

  RenderTheme::ColorCache &RenderTheme::GetColorCache(style::ColorOptions options) const noexcept
  {
    auto optionsIgnoringVisitedLink =
      static_cast<uint8>(options) & ~static_cast<uint8>(style::ColorOptions::ForVisitedLink);
    return _colorCacheMap[static_cast<uint8>(optionsIgnoringVisitedLink)];
  }

  namespace
  {
    KRYS_NODISCARD Color DefaultLinkColor(bool useDarkAppearance) noexcept
    {
      return useDarkAppearance ? SRGBA<uint8> {158, 158, 255} : SRGBA<uint8> {0, 0, 238};
    }

    KRYS_NODISCARD Color DefaultVisitedLinkColor(bool useDarkAppearance) noexcept
    {
      return useDarkAppearance ? SRGBA<uint8> {208, 173, 240} : SRGBA<uint8> {85, 26, 139};
    }
  }

  Color RenderTheme::SystemColor(ValueId cssValueId, style::ColorOptions options) const noexcept
  {
    auto useDarkAppearance = HasFlag(options, style::ColorOptions::UseDarkAppearance);
    auto forVisitedLink = HasFlag(options, style::ColorOptions::ForVisitedLink);

    switch (cssValueId)
    {
      // https://drafts.csswg.org/css-color-4/#valdef-system-color-canvas
      // Background of application content or documents.
      case ValueId::Canvas:      return Color::white;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-canvastext
      // Text in application content or documents.
      case ValueId::Canvastext:  return Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-linktext
      // Text in non-active, non-visited links. For light backgrounds, traditionally blue.
      case ValueId::Linktext:    return DefaultLinkColor(useDarkAppearance);

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-visitedtext
      // Text in visited links. For light backgrounds, traditionally purple.
      case ValueId::Visitedtext: return DefaultVisitedLinkColor(useDarkAppearance);

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-activetext
      // Text in active links. For light backgrounds, traditionally red.
      case ValueId::Activetext:
      case ValueId::WebkitActivelink: // Non-standard addition.
        return useDarkAppearance ? SRGBA<uint8> {255, 158, 158} : Color::red;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-buttonface
      // The face background color for push buttons.
      case ValueId::Buttonface:       return Color::lightGray;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-buttontext
      // Text on push buttons.
      case ValueId::Buttontext:       return Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-buttonborder
      // The base border color for push buttons.
      case ValueId::Buttonborder:     return Color::white;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-field
      // Background of input fields.
      case ValueId::Field:            return Color::white;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-fieldtext
      // Text in input fields.
      case ValueId::Fieldtext:        return Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-highlight
      // Background of selected text, for example from ::selection.
      case ValueId::Highlight:        return SRGBA<uint8> {181, 213, 255};

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-highlighttext
      // Text of selected text.
      case ValueId::Highlighttext:    return Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-selecteditem
      // Background of selected items, for example a selected checkbox.
      case ValueId::Selecteditem:     return Color::lightGray;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-selecteditemtext
      // Text of selected items.
      case ValueId::Selecteditemtext: return Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-mark
      // Background of text that has been specially marked (such as by the HTML mark element).
      case ValueId::Mark:             return Color::yellow;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-marktext
      // Text that has been specially marked (such as by the HTML mark element).
      case ValueId::Marktext:         return Color::black;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-graytext
      // Disabled text. (Often, but not necessarily, gray.)
      case ValueId::Graytext:         return Color::darkGray;

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-accentcolor
      // Background of accented user interface controls.
      case ValueId::Accentcolor:      return SRGBA<uint8> {0, 122, 255};

      // https://drafts.csswg.org/css-color-4/#valdef-system-color-accentcolortext
      // Text of accented user interface controls.
      case ValueId::Accentcolortext:  return Color::black;

      // Non-standard addition.
      case ValueId::Activebuttontext: return Color::black;

      // Non-standard addition.
      case ValueId::Text:             return Color::black;

      // Non-standard addition.
      case ValueId::WebkitLink:
      {
        if (forVisitedLink)
        {
          return DefaultVisitedLinkColor(useDarkAppearance);
        }
        return DefaultLinkColor(useDarkAppearance);
      }

      // Deprecated system-colors:
      // https://drafts.csswg.org/css-color-4/#deprecated-system-colors

      // https://drafts.csswg.org/css-color-4/#activeborder
      // DEPRECATED: Active window border.
      case ValueId::Activeborder:        return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#activecaption
      // DEPRECATED: Active window caption.
      case ValueId::Activecaption:       return SystemColor(ValueId::Canvastext, options);

      // https://drafts.csswg.org/css-color-4/#appworkspace
      // DEPRECATED: Background color of multiple document interface.
      case ValueId::Appworkspace:        return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#background
      // DEPRECATED: Desktop background.
      case ValueId::Background:          return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#buttonhighlight
      // DEPRECATED: The color of the border facing the light source for 3-D elements that
      // appear 3-D due to one layer of surrounding border.
      case ValueId::Buttonhighlight:     return SystemColor(ValueId::Buttonface, options);

      // https://drafts.csswg.org/css-color-4/#buttonshadow
      // DEPRECATED: The color of the border away from the light source for 3-D elements that
      // appear 3-D due to one layer of surrounding border.
      case ValueId::Buttonshadow:        return SystemColor(ValueId::Buttonface, options);

      // https://drafts.csswg.org/css-color-4/#captiontext
      // DEPRECATED: Text in caption, size box, and scrollbar arrow box.
      case ValueId::Captiontext:         return SystemColor(ValueId::Canvastext, options);

      // https://drafts.csswg.org/css-color-4/#inactiveborder
      // DEPRECATED: Inactive window border.
      case ValueId::Inactiveborder:      return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#inactivecaption
      // DEPRECATED: Inactive window caption.
      case ValueId::Inactivecaption:     return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#inactivecaptiontext
      // DEPRECATED: Color of text in an inactive caption.
      case ValueId::Inactivecaptiontext: return SystemColor(ValueId::Graytext, options);

      // https://drafts.csswg.org/css-color-4/#infobackground
      // DEPRECATED: Background color for tooltip controls.
      case ValueId::Infobackground:      return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#infotext
      // DEPRECATED: Text color for tooltip controls.
      case ValueId::Infotext:            return SystemColor(ValueId::Canvastext, options);

      // https://drafts.csswg.org/css-color-4/#menu
      // DEPRECATED: Menu background.
      case ValueId::Menu:                return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#menutext
      // DEPRECATED: Text in menus.
      case ValueId::Menutext:            return SystemColor(ValueId::Canvastext, options);

      // https://drafts.csswg.org/css-color-4/#scrollbar
      // DEPRECATED: Scroll bar gray area.
      case ValueId::Scrollbar:           return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#threeddarkshadow
      // DEPRECATED: The color of the darker (generally outer) of the two borders away from
      // thelight source for 3-D elements that appear 3-D due to two concentric layers of
      // surrounding border.
      case ValueId::Threeddarkshadow:    return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#threedface
      // DEPRECATED: The face background color for 3-D elements that appear 3-D due to two
      // concentric layers of surrounding border
      case ValueId::Threedface:          return SystemColor(ValueId::Buttonface, options);

      // https://drafts.csswg.org/css-color-4/#threedhighlight
      // DEPRECATED: The color of the lighter (generally outer) of the two borders facing
      // the light source for 3-D elements that appear 3-D due to two concentric layers of
      // surrounding border.
      case ValueId::Threedhighlight:     return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#threedlightshadow
      // DEPRECATED: The color of the darker (generally inner) of the two borders facing
      // the light source for 3-D elements that appear 3-D due to two concentric layers of
      // surrounding border
      case ValueId::Threedlightshadow:   return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#threedshadow
      // DEPRECATED: The color of the lighter (generally inner) of the two borders away
      // from the light source for 3-D elements that appear 3-D due to two concentric layers
      // of surrounding border.
      case ValueId::Threedshadow:        return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#window
      // DEPRECATED: Window background.
      case ValueId::Window:              return SystemColor(ValueId::Canvas, options);

      // https://drafts.csswg.org/css-color-4/#windowframe
      // DEPRECATED: Window frame.
      case ValueId::Windowframe:         return SystemColor(ValueId::Buttonborder, options);

      // https://drafts.csswg.org/css-color-4/#windowtext
      // DEPRECATED: Text in windows.
      case ValueId::Windowtext:          return SystemColor(ValueId::Canvastext, options);

      default:                           return {};
    }
  }

  Color RenderTheme::TextSearchHighlightColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.TextSearchHighlightColor.IsValid())
    {
      cache.TextSearchHighlightColor = PlatformTextSearchHighlightColor(options);
    }
    return cache.TextSearchHighlightColor;
  }

  Color RenderTheme::PlatformTextSearchHighlightColor(style::ColorOptions) const noexcept
  {
    return Color::yellow;
  }

  Color RenderTheme::AnnotationHighlightBackgroundColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.AnnotationHighlightBackgroundColor.IsValid())
    {
      cache.AnnotationHighlightBackgroundColor =
        TransformSelectionBackgroundColor(PlatformAnnotationHighlightBackgroundColor(options), options);
    }
    return cache.AnnotationHighlightBackgroundColor;
  }

  Color RenderTheme::PlatformAnnotationHighlightBackgroundColor(style::ColorOptions) const noexcept
  {
    return Color::yellow;
  }

  Color RenderTheme::AnnotationHighlightForegroundColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.AnnotationHighlightForegroundColor.IsValid())
    {
      cache.AnnotationHighlightForegroundColor =
        css::Resolve(css::ContrastColorResolver {AnnotationHighlightBackgroundColor(options)});
    }
    return cache.AnnotationHighlightForegroundColor;
  }

  Color RenderTheme::DefaultButtonTextColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.DefaultButtonTextColor.IsValid())
    {
      cache.DefaultButtonTextColor = PlatformDefaultButtonTextColor(options);
    }
    return cache.DefaultButtonTextColor;
  }

  Color RenderTheme::PlatformDefaultButtonTextColor(style::ColorOptions options) const noexcept
  {
    return SystemColor(ValueId::Activebuttontext, options);
  }

  // Value chosen to return dark gray for both white on black and black on white.
  constexpr float DatePlaceholderColorLightnessAdjustmentFactor = 0.66f;

  Color RenderTheme::DatePlaceholderTextColor(const Color &textColor,
                                              const Color &backgroundColor) const noexcept
  {
    // FIXME: Consider using LCHA<float> rather than HSLA<float> for better perceptual results and to avoid
    // clamping to sRGB gamut, which is what HSLA does.
    auto hsla = textColor.ToColorTypeLossy<HSLA<float>>().Resolved();
    if (textColor.Luminance() < backgroundColor.Luminance())
    {
      hsla.Lightness += DatePlaceholderColorLightnessAdjustmentFactor * (100.0f - hsla.Lightness);
    }
    else
    {
      hsla.Lightness *= DatePlaceholderColorLightnessAdjustmentFactor;
    }

    // FIXME: Consider keeping color in LCHA (if that change is made) or converting back to the initial
    // underlying color type to avoid unnecessarily clamping colors outside of sRGB.
    return ConvertColour<SRGBA<float>>(hsla);
  }

  Color RenderTheme::SpellingMarkerColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.SpellingMarkerColor.IsValid())
    {
      cache.SpellingMarkerColor = PlatformSpellingMarkerColor(options);
    }
    return cache.SpellingMarkerColor;
  }

  Color RenderTheme::PlatformSpellingMarkerColor(style::ColorOptions) const noexcept
  {
    return Color::red;
  }

  Color RenderTheme::DictationAlternativesMarkerColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.DictationAlternativesMarkerColor.IsValid())
    {
      cache.DictationAlternativesMarkerColor = PlatformDictationAlternativesMarkerColor(options);
    }
    return cache.DictationAlternativesMarkerColor;
  }

  Color RenderTheme::PlatformDictationAlternativesMarkerColor(style::ColorOptions) const noexcept
  {
    return Color::green;
  }

  Color RenderTheme::AutocorrectionReplacementMarkerColor(const RenderText &renderer) const noexcept
  {
    auto options = renderer.StyleColorOptions();
    auto &cache = GetColorCache(options);
    if (!cache.AutocorrectionReplacementMarkerColor.IsValid())
    {
      cache.AutocorrectionReplacementMarkerColor = PlatformAutocorrectionReplacementMarkerColor(options);
    }
    return cache.AutocorrectionReplacementMarkerColor;
  }

  Color RenderTheme::PlatformAutocorrectionReplacementMarkerColor(style::ColorOptions) const noexcept
  {
    return Color::green;
  }

  Color RenderTheme::GrammarMarkerColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.GrammarMarkerColor.IsValid())
    {
      cache.GrammarMarkerColor = PlatformGrammarMarkerColor(options);
    }
    return cache.GrammarMarkerColor;
  }

  Color RenderTheme::PlatformGrammarMarkerColor(style::ColorOptions) const noexcept
  {
    return Color::green;
  }

  Color RenderTheme::DocumentMarkerLineColor(const RenderText &renderer,
                                             DocumentMarkerLineStyleMode mode) const noexcept
  {
    auto options = renderer.StyleColorOptions();

    switch (mode)
    {
      case DocumentMarkerLineStyleMode::Spelling: return SpellingMarkerColor(options);
      case DocumentMarkerLineStyleMode::DictationAlternatives:
      case DocumentMarkerLineStyleMode::TextCheckingDictationPhraseWithAlternatives:
        return DictationAlternativesMarkerColor(options);
      case DocumentMarkerLineStyleMode::AutocorrectionReplacement:
        return AutocorrectionReplacementMarkerColor(renderer);
      case DocumentMarkerLineStyleMode::Grammar: return GrammarMarkerColor(options);
    }

    krys_debug_assert(false);
    return Color::transparentBlack;
  }

  utf8_string RenderTheme::FileListDefaultLabel(bool multipleFilesAllowed) const noexcept
  {
    if (multipleFilesAllowed)
    {
      return FileButtonNoFilesSelectedLabel();
    }
    return FileButtonNoFileSelectedLabel();
  }

  utf8_string RenderTheme::FileListNameForWidth(const FileList *fileList, const FontCascade &font,
                                                int32 width, bool multipleFilesAllowed) const noexcept
  {
    if (width <= 0)
    {
      return {};
    }

    utf8_string string;
    if (fileList->IsEmpty())
    {
      string = FileListDefaultLabel(multipleFilesAllowed);
    }
    else if (fileList->Length() == 1)
    {
      string = fileList->Item(0)->Name();
    }
    else
    {
      return StringTruncator::RightTruncate(MultipleFileUploadText(fileList->Length()),
                                            static_cast<float>(width), font);
    }

    return StringTruncator::CenterTruncate(string, static_cast<float>(width), font);
  }

  Color RenderTheme::FocusRingColor(style::ColorOptions options) const noexcept
  {
    auto &cache = GetColorCache(options);
    if (!cache.SystemFocusRingColor.IsValid())
    {
      cache.SystemFocusRingColor = PlatformFocusRingColor(options);
    }
    return cache.SystemFocusRingColor;
  }
}
