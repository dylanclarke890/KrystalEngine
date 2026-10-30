#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Core/StyleAppearance.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Controls/ControlStyle.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Platform/PopupMenuStyle.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/LineWidth.hpp"
#include "Krystal.Booey/Style/Values/Box/Padding.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorOptions.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MinimumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  namespace dom
  {
    class Element;
  }

  namespace html
  {
    class HTMLElement;
    class HTMLInputElement;
  }

  class ControlPart;
  class FileList;
  class FontCascade;
  class FontCascadeDescription;
  class GraphicsContext;
  struct PaintInfo;
  class RenderBox;
  class RenderElement;
  class RenderLayerModelObject;
  class RenderMeter;
  class RenderObject;
  class RenderProgress;
  class RenderStyle;
  class RenderText;
  struct Settings;
  enum class DocumentMarkerLineStyleMode : uint8;

  /// @brief Decides which elements have a native appearance, adjusts their styles and sizes, and paints them;
  /// also the platform's colours (selections, focus rings, system colours).
  class RenderTheme
  {
    KRYS_NON_COPY_MOVABLE(RenderTheme);

  protected:
    struct ColorCache
    {
      HashMap<int32, Color> SystemStyleColors;

      Color SystemLinkColor;
      Color SystemActiveLinkColor;
      Color SystemVisitedLinkColor;
      Color SystemFocusRingColor;
      Color SystemControlAccentColor;

      Color ActiveSelectionBackgroundColor;
      Color InactiveSelectionBackgroundColor;
      Color ActiveSelectionForegroundColor;
      Color InactiveSelectionForegroundColor;

      Color ActiveListBoxSelectionBackgroundColor;
      Color InactiveListBoxSelectionBackgroundColor;
      Color ActiveListBoxSelectionForegroundColor;
      Color InactiveListBoxSelectionForegroundColor;

      Color TextSearchHighlightColor;
      Color AnnotationHighlightBackgroundColor;
      Color AnnotationHighlightForegroundColor;

      Color DefaultButtonTextColor;
      Color DefaultSubmitButtonTextColor;

      Color SpellingMarkerColor;
      Color DictationAlternativesMarkerColor;
      Color AutocorrectionReplacementMarkerColor;
      Color GrammarMarkerColor;
    };

  private:
    mutable HashMap<uint8, ColorCache> _colorCacheMap;

    bool _useFormSemanticContext {false};

  protected:
    RenderTheme() noexcept;
    virtual ~RenderTheme() noexcept;

  public:
    enum class InnerSpinButtonLayout : uint8
    {
      Vertical,
      HorizontalUpLeft,
      HorizontalUpRight
    };

    /// @brief The platform's theme.
    KRYS_NODISCARD static RenderTheme &Singleton() noexcept;

    virtual void PurgeCaches() noexcept;

    // This method is called whenever style has been computed for an element and the appearance
    // property has been set to a value other than "none".  The theme should map in all of the appropriate
    // metrics and defaults given the contents of the style.  This includes sophisticated operations like
    // selection of control size based off the font, the disabling of appearance when CSS properties that
    // disable native appearance are set, or if the appearance is not supported by the theme.
    void AdjustStyle(RenderStyle &, const RenderStyle &parentStyle, const dom::Element *) noexcept;

    KRYS_NODISCARD virtual bool CanCreateControlPartForRenderer(const RenderElement &) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool CanCreateControlPartForBorderOnly(const RenderElement &) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool CanCreateControlPartForDecorations(const RenderElement &) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD RefPtr<ControlPart> CreateControlPart(const RenderElement &) const noexcept;

    void UpdateControlPartForRenderer(ControlPart &, const RenderElement &) const noexcept;

    // These methods are called to paint the widget as a background of the RenderElement. A widget's
    // foreground, e.g., the text of a button, is always rendered by the engine itself. The boolean return
    // value indicates whether the CSS border/background should also be painted.
    bool Paint(const RenderBox &, ControlPart &, const PaintInfo &, const LayoutRect &) noexcept;
    bool Paint(const RenderBox &, const PaintInfo &, const LayoutRect &) noexcept;

    bool PaintBorderOnly(const RenderBox &, const PaintInfo &) noexcept;
    void PaintDecorations(const RenderBox &, const PaintInfo &, const LayoutRect &) noexcept;

    // The remaining methods should be implemented by the platform-specific portion of the theme.

    /// @brief The theme's own user agent style sheet, added to the default style sheets.
    KRYS_NODISCARD virtual utf8_stringview ExtraDefaultStyleSheet() noexcept
    {
      return {};
    }

    KRYS_NODISCARD virtual LayoutRect AdjustedPaintRect(const RenderBox &,
                                                        const LayoutRect &paintRect) const noexcept
    {
      return paintRect;
    }

    // A method to obtain the baseline position for a "leaf" control.  This will only be used if a baseline
    // position cannot be determined by examining child content. Checkboxes and radio buttons are examples of
    // controls that need to do this.
    KRYS_NODISCARD virtual int32 BaselinePosition(const RenderBox &) const noexcept;

    // A method for asking if a control is a container or not.  Leaf controls have to have some special
    // behavior (like the baseline position API above).
    KRYS_NODISCARD bool IsControlContainer(StyleAppearance) const noexcept;

    // A method asking if the control changes its tint when the window has focus or not.
    KRYS_NODISCARD virtual bool ControlSupportsTints(const RenderElement &) const noexcept
    {
      return false;
    }

    // Whether or not the control has been styled enough by the author to disable the native appearance.
    KRYS_NODISCARD virtual bool IsControlStyled(const RenderStyle &) const noexcept;

    // A general method asking if any control tinting is supported at all.
    KRYS_NODISCARD virtual bool SupportsControlTints() const noexcept
    {
      return false;
    }

    // Some controls may spill out of their containers (e.g., the check on a macOS checkbox). When these
    // controls repaint, the theme needs to communicate this inflated rect to the engine so that it can
    // invalidate the whole control.
    virtual void InflateRectForControlRenderer(const RenderElement &, FloatRect &) noexcept
    {
    }

    virtual void AdjustRepaintRect(const RenderBox &, FloatRect &) noexcept
    {
    }

    // A method asking if the theme is able to draw the focus ring.
    KRYS_NODISCARD virtual bool SupportsFocusRing(const RenderElement &, const RenderStyle &) const noexcept;

    // A method asking if the theme's controls actually care about redrawing when hovered.
    KRYS_NODISCARD virtual bool SupportsHover() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool SupportsBoxShadow(const RenderStyle &) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool UseFormSemanticContext() const noexcept
    {
      return _useFormSemanticContext;
    }

    void SetUseFormSemanticContext(bool value) noexcept
    {
      _useFormSemanticContext = value;
    }

    KRYS_NODISCARD virtual bool SupportsLargeFormControls() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool SearchFieldShouldAppearAsTextField(const RenderStyle &,
                                                                   const Settings &) const noexcept
    {
      return false;
    }

    // Text selection colors.
    KRYS_NODISCARD Color ActiveSelectionBackgroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD Color InactiveSelectionBackgroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color TransformSelectionBackgroundColor(const Color &,
                                                                   style::ColorOptions) const noexcept;
    KRYS_NODISCARD Color ActiveSelectionForegroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD Color InactiveSelectionForegroundColor(style::ColorOptions) const noexcept;

    // List box selection colors
    KRYS_NODISCARD Color ActiveListBoxSelectionBackgroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD Color ActiveListBoxSelectionForegroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD Color InactiveListBoxSelectionBackgroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD Color InactiveListBoxSelectionForegroundColor(style::ColorOptions) const noexcept;

    // Highlighting color for search matches.
    KRYS_NODISCARD Color TextSearchHighlightColor(style::ColorOptions) const noexcept;

    // Default highlighting color for app highlights.
    KRYS_NODISCARD Color AnnotationHighlightBackgroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD Color AnnotationHighlightForegroundColor(style::ColorOptions) const noexcept;

    KRYS_NODISCARD Color DefaultButtonTextColor(style::ColorOptions) const noexcept;

    KRYS_NODISCARD Color DatePlaceholderTextColor(const Color &textColor,
                                                  const Color &backgroundColor) const noexcept;

    KRYS_NODISCARD Color DocumentMarkerLineColor(const RenderText &,
                                                 DocumentMarkerLineStyleMode) const noexcept;

    KRYS_NODISCARD Color FocusRingColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color PlatformFocusRingColor(style::ColorOptions) const noexcept
    {
      return Color::black;
    }

    KRYS_NODISCARD static float PlatformFocusRingWidth() noexcept
    {
      return 3;
    }

    KRYS_NODISCARD static float PlatformFocusRingOffset(float outlineWidth) noexcept
    {
      return std::max<float>(outlineWidth - PlatformFocusRingWidth(), 0);
    }

    virtual void PlatformColorsDidChange() noexcept;

    /// @brief How long the caret stays on and off as it blinks; none if it doesn't blink.
    KRYS_NODISCARD virtual Maybe<MonotonicTime::duration> CaretBlinkInterval() const noexcept
    {
      return std::chrono::milliseconds(500);
    }

    // System fonts and colors for CSS.
    KRYS_NODISCARD virtual Color SystemColor(css::ValueId, style::ColorOptions) const noexcept;

    KRYS_NODISCARD virtual int32 MinimumMenuListSize(const RenderStyle &) const noexcept
    {
      return 0;
    }

    virtual void AdjustSliderThumbSize(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    KRYS_NODISCARD virtual style::PaddingBox PopupInternalPaddingBox(const RenderStyle &) const noexcept;

    KRYS_NODISCARD virtual PopupMenuStyle::Size PopupMenuSize(const RenderStyle &, IntRect &) const noexcept
    {
      return PopupMenuStyle::Size::Normal;
    }

    KRYS_NODISCARD virtual ScrollbarWidth ScrollbarWidthStyleForPart(StyleAppearance) noexcept
    {
      return ScrollbarWidth::Auto;
    }

    KRYS_NODISCARD virtual MonotonicTime::duration
      AnimationRepeatIntervalForProgressBar(const RenderProgress &) const noexcept
    {
      return {};
    }

    KRYS_NODISCARD virtual MonotonicTime::duration AnimationDurationForProgressBar() const noexcept
    {
      return {};
    }

    KRYS_NODISCARD virtual IntRect ProgressBarRectForBounds(const RenderProgress &,
                                                            const IntRect &bounds) const noexcept
    {
      return bounds;
    }

    KRYS_NODISCARD virtual FloatSize MeterSizeForBounds(const RenderMeter &,
                                                        const FloatRect &) const noexcept;
    KRYS_NODISCARD virtual bool SupportsMeter(StyleAppearance) const noexcept
    {
      return false;
    }

    // Returns the threshold distance for snapping to a slider tick mark.
    KRYS_NODISCARD virtual LayoutUnit SliderTickSnappingThreshold() const noexcept
    {
      return {};
    }

    // Returns size of one slider tick mark for a horizontal track.
    // For vertical tracks we rotate it and use it. i.e. Width is always length along the track.
    KRYS_NODISCARD virtual IntSize SliderTickSize() const noexcept
    {
      return {};
    }

    // Returns the distance of slider tick origin from the slider track center.
    KRYS_NODISCARD virtual int32 SliderTickOffsetFromTrackCenter() const noexcept
    {
      return 0;
    }

    virtual void PaintSliderTicks(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept;

    KRYS_NODISCARD virtual bool ShouldHaveSpinButton(const html::HTMLInputElement &) const noexcept;
    KRYS_NODISCARD virtual bool ShouldHaveCapsLockIndicator(const html::HTMLInputElement &) const noexcept
    {
      return false;
    }

    virtual void CreateColorWellSwatchSubtree(html::HTMLElement &) noexcept
    {
    }

    virtual void SetColorWellSwatchBackground(html::HTMLElement &, Color) noexcept;

    // Functions for <select> elements.
    KRYS_NODISCARD virtual bool DelegatesMenuListRendering() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool PopsMenuByArrowKeys() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool PopsMenuBySpaceOrReturn() const noexcept
    {
      return false;
    }

    /// @brief What a file input shows when it has no files.
    KRYS_NODISCARD virtual utf8_string FileListDefaultLabel(bool multipleFilesAllowed) const noexcept;
    /// @brief What a file input shows for @p fileList, truncated to @p width in @p font.
    KRYS_NODISCARD virtual utf8_string FileListNameForWidth(const FileList *fileList, const FontCascade &font,
                                                            int32 width,
                                                            bool multipleFilesAllowed) const noexcept;

    KRYS_NODISCARD virtual InnerSpinButtonLayout
      GetInnerSpinButtonLayout(const RenderElement &) const noexcept
    {
      return InnerSpinButtonLayout::Vertical;
    }

    KRYS_NODISCARD EnumSet<ControlStyle::State>
      ExtractControlStyleStatesForRenderer(const RenderObject &) const noexcept;

    virtual void PaintPlatformResizer(const RenderLayerModelObject &, GraphicsContext &,
                                      const LayoutRect &resizerCornerRect) noexcept;
    virtual void PaintPlatformResizerFrame(const RenderLayerModelObject &, GraphicsContext &,
                                           const LayoutRect &resizerAbsRect) noexcept;

    // NOTE: This is just a legacy hard-coded list of elements that have some appearance value in html.css
    // FIXME: Remove when devolvable widgets are universally enabled.
    KRYS_NODISCARD static bool HasAppearanceForElementTypeFromUAStyle(const dom::Element &) noexcept;

    virtual void AdjustTextControlInnerContainerStyle(RenderStyle &, const RenderStyle &,
                                                      const dom::Element *) const noexcept
    {
    }

    virtual void AdjustTextControlInnerPlaceholderStyle(RenderStyle &, const RenderStyle &,
                                                        const dom::Element *) const noexcept
    {
    }

    virtual void AdjustTextControlInnerTextStyle(RenderStyle &, const RenderStyle &,
                                                 const dom::Element *) const noexcept
    {
    }

    KRYS_NODISCARD virtual Color SubmitButtonTextColor(const RenderText &) const noexcept
    {
      return Color::black;
    }

    KRYS_NODISCARD virtual bool MayNeedBleedAvoidance(const RenderStyle &) const noexcept
    {
      return true;
    }

    KRYS_NODISCARD virtual float
      AdjustedMaximumLogicalWidthForControl(const RenderStyle &, const dom::Element &,
                                            float maximumLogicalWidth) const noexcept
    {
      return maximumLogicalWidth;
    }

    KRYS_NODISCARD bool IsWindowActive(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsChecked(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsIndeterminate(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsEnabled(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsFocused(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsPressed(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsSpinUpButtonPartPressed(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsHovered(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsSpinUpButtonPartHovered(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsPresenting(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsReadOnlyControl(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool IsDefault(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool HasListButton(const RenderElement &) const noexcept;
    KRYS_NODISCARD bool HasListButtonPressed(const RenderElement &) const noexcept;

  protected:
    KRYS_NODISCARD ControlStyle ExtractControlStyleForRenderer(const RenderElement &) const noexcept;

    KRYS_NODISCARD virtual bool CanPaint(const PaintInfo &, const Settings &, StyleAppearance) const noexcept
    {
      return true;
    }

    // The platform selection color.
    KRYS_NODISCARD virtual Color PlatformActiveSelectionBackgroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color PlatformInactiveSelectionBackgroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color PlatformActiveSelectionForegroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color PlatformInactiveSelectionForegroundColor(style::ColorOptions) const noexcept;

    KRYS_NODISCARD virtual Color
      PlatformActiveListBoxSelectionBackgroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color
      PlatformInactiveListBoxSelectionBackgroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color
      PlatformActiveListBoxSelectionForegroundColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color
      PlatformInactiveListBoxSelectionForegroundColor(style::ColorOptions) const noexcept;

    KRYS_NODISCARD virtual Color PlatformTextSearchHighlightColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color
      PlatformAnnotationHighlightBackgroundColor(style::ColorOptions) const noexcept;

    KRYS_NODISCARD virtual Color PlatformDefaultButtonTextColor(style::ColorOptions) const noexcept;

    KRYS_NODISCARD virtual Color PlatformSpellingMarkerColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color PlatformDictationAlternativesMarkerColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color
      PlatformAutocorrectionReplacementMarkerColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD virtual Color PlatformGrammarMarkerColor(style::ColorOptions) const noexcept;

    KRYS_NODISCARD virtual bool SupportsSelectionForegroundColors(style::ColorOptions) const noexcept
    {
      return true;
    }

    KRYS_NODISCARD virtual bool SupportsListBoxSelectionForegroundColors(style::ColorOptions) const noexcept
    {
      return true;
    }

    // Methods for each appearance value.
    virtual void AdjustCheckboxStyle(RenderStyle &, const dom::Element *) const noexcept;
    virtual bool PaintCheckbox(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustRadioStyle(RenderStyle &, const dom::Element *) const noexcept;
    virtual bool PaintRadio(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustButtonStyle(RenderStyle &, const dom::Element *) const noexcept;
    virtual bool PaintButton(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustColorWellStyle(RenderStyle &, const dom::Element *) const noexcept;
    virtual bool PaintColorWell(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void PaintColorWellDecorations(const RenderElement &, const PaintInfo &,
                                           const FloatRect &) noexcept
    {
    }

    virtual void AdjustColorWellSwatchStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual void AdjustColorWellSwatchOverlayStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual void AdjustColorWellSwatchWrapperStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual bool PaintColorWellSwatch(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustInnerSpinButtonStyle(RenderStyle &, const dom::Element *) const noexcept;
    virtual bool PaintInnerSpinButton(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustTextFieldStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual bool PaintTextField(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void PaintTextFieldDecorations(const RenderBox &, const PaintInfo &, const FloatRect &) noexcept
    {
    }

    virtual void AdjustTextAreaStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual bool PaintTextArea(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void PaintTextAreaDecorations(const RenderBox &, const PaintInfo &, const FloatRect &) noexcept
    {
    }

    virtual void AdjustMenuListStyle(RenderStyle &, const dom::Element *) const noexcept;
    virtual bool PaintMenuList(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void PaintMenuListDecorations(const RenderElement &, const PaintInfo &,
                                          const FloatRect &) noexcept
    {
    }

    virtual void AdjustMenuListButtonStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual void PaintMenuListButtonDecorations(const RenderBox &, const PaintInfo &,
                                                const FloatRect &) noexcept
    {
    }

    virtual bool PaintMenuListButton(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustMeterStyle(RenderStyle &, const dom::Element *) const noexcept;
    virtual bool PaintMeter(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustListButtonStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual bool PaintListButton(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustProgressBarStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual bool PaintProgressBar(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustSliderTrackStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual bool PaintSliderTrack(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustSliderThumbStyle(RenderStyle &, const dom::Element *) const noexcept;
    virtual bool PaintSliderThumb(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustSearchFieldStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual bool PaintSearchField(const RenderElement &, const PaintInfo &, const FloatRect &) noexcept
    {
      return true;
    }

    virtual void PaintSearchFieldDecorations(const RenderBox &, const PaintInfo &, const FloatRect &) noexcept
    {
    }

    virtual void AdjustSearchFieldCancelButtonStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual bool PaintSearchFieldCancelButton(const RenderBox &, const PaintInfo &,
                                              const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustSearchFieldDecorationPartStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual bool PaintSearchFieldDecorationPart(const RenderElement &, const PaintInfo &,
                                                const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustSearchFieldResultsDecorationPartStyle(RenderStyle &,
                                                             const dom::Element *) const noexcept
    {
    }

    virtual bool PaintSearchFieldResultsDecorationPart(const RenderBox &, const PaintInfo &,
                                                       const FloatRect &) noexcept
    {
      return true;
    }

    virtual void AdjustSearchFieldResultsButtonStyle(RenderStyle &, const dom::Element *) const noexcept
    {
    }

    virtual bool PaintSearchFieldResultsButton(const RenderBox &, const PaintInfo &,
                                               const FloatRect &) noexcept
    {
      return true;
    }

    // The font description result should have a zoomed font size.
    KRYS_NODISCARD virtual Maybe<FontCascadeDescription> ControlFont(StyleAppearance, const FontCascade &,
                                                                     float) const noexcept;

    KRYS_NODISCARD virtual style::PaddingBox ControlPadding(StyleAppearance, const style::PaddingBox &,
                                                            float zoomFactor) const noexcept;

    // The size here is in zoomed coordinates already. If a new size is returned, it also needs to be in
    // zoomed coordinates.
    KRYS_NODISCARD virtual style::PreferredSizePair ControlSize(StyleAppearance, const FontCascade &,
                                                                const style::PreferredSizePair &,
                                                                float zoomFactor) const noexcept;

    // Returns the minimum size for a control in zoomed coordinates.
    KRYS_NODISCARD style::MinimumSizePair MinimumControlSize(StyleAppearance, const FontCascade &,
                                                             const style::MinimumSizePair &,
                                                             const style::PreferredSizePair &,
                                                             float zoomFactor) const noexcept;

    // Allows the theme to modify the existing border.
    KRYS_NODISCARD virtual style::LineWidthBox ControlBorder(StyleAppearance, const FontCascade &,
                                                             const style::LineWidthBox &zoomedBox,
                                                             float zoomFactor,
                                                             const dom::Element *) const noexcept;

    // Whether or not whitespace: pre should be forced on always.
    KRYS_NODISCARD virtual bool ControlRequiresPreWhiteSpace(StyleAppearance) const noexcept
    {
      return false;
    }

    // Used when we want to set computed style on a form control. Before Evaluation Time Zoom, we were
    // setting the zoomed size on the computed style. In order to make sure this behavior remains if
    // the flag is off, we return the used zoom value, which was being used before, when the flag
    // is disabled and 1.0f when it is enabled so that we do not modify the value.
    KRYS_NODISCARD float UsedZoomForComputedStyle(const RenderStyle &) const noexcept;

    KRYS_NODISCARD virtual ColorCache &GetColorCache(style::ColorOptions) const noexcept;

    KRYS_NODISCARD virtual Color AutocorrectionReplacementMarkerColor(const RenderText &) const noexcept;

    KRYS_NODISCARD virtual style::MinimumSizePair MinimumControlSize(StyleAppearance, const FontCascade &,
                                                                     const style::MinimumSizePair &,
                                                                     float zoomFactor) const noexcept;

  private:
    KRYS_NODISCARD EnumSet<ControlStyle::State>
      ExtractControlStyleStatesForRendererInternal(const RenderElement &) const noexcept;

    void AdjustButtonOrCheckboxOrColorWellOrInnerSpinButtonOrRadioStyle(RenderStyle &,
                                                                        const dom::Element *) const noexcept;

    KRYS_NODISCARD StyleAppearance AutoAppearanceForElement(RenderStyle &,
                                                            const dom::Element *) const noexcept;
    KRYS_NODISCARD StyleAppearance AdjustAppearanceForElement(RenderStyle &, const RenderStyle &parentStyle,
                                                              const dom::Element *,
                                                              StyleAppearance) const noexcept;

    KRYS_NODISCARD Color SpellingMarkerColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD Color DictationAlternativesMarkerColor(style::ColorOptions) const noexcept;
    KRYS_NODISCARD Color GrammarMarkerColor(style::ColorOptions) const noexcept;
  };
}
