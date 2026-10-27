#include "Krystal.Booey/Rendering/RenderFileUploadControl.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/StringTruncator.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderButton.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <numbers>

namespace krys::boo
{
  namespace
  {
    constexpr int32 AfterButtonSpacing = 4;
    constexpr int32 ButtonShadowHeight = 2;
    constexpr int32 DefaultWidthNumChars = 34;

    KRYS_NODISCARD int32 NodeLogicalWidth(dom::Node *node) noexcept
    {
      return (node && node->GetRenderBox()) ? RoundToInt(node->GetRenderBox()->LogicalSize().Width()) : 0;
    }
  }

  RenderFileUploadControl::RenderFileUploadControl(html::HTMLInputElement &input,
                                                   RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::FileUploadControl, input, krys::move(style)),
        _canReceiveDroppedFiles(input.CanReceiveDroppedFiles())
  {
    krys_debug_assert(IsRenderFileUploadControl());
  }

  RenderFileUploadControl::~RenderFileUploadControl() noexcept = default;

  html::HTMLInputElement &RenderFileUploadControl::InputElement() const noexcept
  {
    return Downcast<html::HTMLInputElement>(NodeForNonAnonymous());
  }

  void RenderFileUploadControl::UpdateFromElement() noexcept
  {
    krys_debug_assert(InputElement().IsFileUpload());

    if (auto *button = UploadButton())
    {
      bool newCanReceiveDroppedFilesState = InputElement().CanReceiveDroppedFiles();
      if (_canReceiveDroppedFiles != newCanReceiveDroppedFilesState)
      {
        _canReceiveDroppedFiles = newCanReceiveDroppedFilesState;
        button->SetActive(newCanReceiveDroppedFilesState);
      }
    }

    // This only supports clearing out the files, but that's OK because for security reasons that's the only
    // change the DOM is allowed to make.
    auto *files = InputElement().Files();
    krys_debug_assert(files);
    if (files && files->IsEmpty())
    {
      Repaint();
    }
  }

  int32 RenderFileUploadControl::MaxFilenameLogicalWidth() const noexcept
  {
    return std::max(0, RoundToInt(ContentBoxLogicalWidth()) - NodeLogicalWidth(UploadButton())
                         - AfterButtonSpacing);
  }

  void RenderFileUploadControl::PaintObject(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (Style().UsedVisibility() != Visibility::Visible)
    {
      return;
    }

    if (!paintInfo.Context().PaintingDisabled())
    {
      PaintControl(paintInfo, paintOffset);
    }

    // Paint the children.
    RenderBlockFlow::PaintObject(paintInfo, paintOffset);
  }

  void RenderFileUploadControl::PaintControl(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    // Push a clip.
    GraphicsContextStateSaver stateSaver(paintInfo.Context(), false);
    if (paintInfo.Phase == PaintPhase::Foreground || paintInfo.Phase == PaintPhase::ChildBlockBackgrounds)
    {
      auto clipRect = EnclosingIntRect(LayoutRect(
        paintOffset.X() + BorderLeft(), paintOffset.Y() + BorderTop(), Width() - BorderLeft() - BorderRight(),
        Height() - BorderBottom() - BorderTop() + ButtonShadowHeight));
      if (clipRect.IsEmpty())
      {
        return;
      }
      stateSaver.Save();
      paintInfo.Context().Clip(FloatRect(clipRect));
    }

    auto writingMode = GetWritingMode();
    auto isHorizontalWritingMode = writingMode.IsHorizontal();
    auto isBlockFlipped = writingMode.IsBlockFlipped();
    auto logicalPaintOffset = isHorizontalWritingMode ? paintOffset : paintOffset.TransposedPoint();

    if (paintInfo.Phase != PaintPhase::Foreground)
    {
      return;
    }

    auto displayedFilename = FileTextValue();
    auto &font = Style().GetFontCascade();
    auto textRun = ConstructTextRun(displayedFilename, Style(), ExpansionBehavior::AllowRightOnly(),
                                    {TextRunFlag::RespectDirection, TextRunFlag::RespectDirectionOverride});
    // Determine where the filename should be placed
    LayoutUnit contentLogicalLeft = logicalPaintOffset.X() + LogicalLeftOffsetForContent();
    if (writingMode.IsLogicalLeftInlineStart())
    {
      contentLogicalLeft += TextIndentOffset();
    }
    else
    {
      contentLogicalLeft -= TextIndentOffset();
    }

    auto *button = UploadButton();
    if (!button)
    {
      return;
    }

    LayoutUnit buttonLogicalWidth(NodeLogicalWidth(button));
    LayoutUnit buttonAndIconLogicalWidth = buttonLogicalWidth + AfterButtonSpacing;
    LayoutUnit textLogicalLeft;
    if (writingMode.IsLogicalLeftInlineStart())
    {
      textLogicalLeft = contentLogicalLeft + buttonAndIconLogicalWidth;
    }
    else
    {
      textLogicalLeft = contentLogicalLeft + ContentBoxLogicalWidth() - buttonAndIconLogicalWidth
                        - LayoutUnit(font.Width(textRun));
    }

    // We want to match the button's baseline
    // FIXME: Make this work with transforms.
    auto textLogicalTop = [&]() -> float
    {
      if (auto *buttonRenderer = DynamicDowncast<RenderButton>(button->Renderer()))
      {
        if (auto *buttonTextRenderer = buttonRenderer->TextRenderer())
        {
          if (auto textBox = inline_iterator::LineLeftmostTextBoxFor(*buttonTextRenderer))
          {
            auto textVisualRect = textBox->VisualRectIgnoringBlockDirection();
            textVisualRect.SetLocation(
              buttonTextRenderer->LocalToContainerPoint(textVisualRect.Location(), this));
            textVisualRect.MoveBy(RoundPointToDevicePixels(paintOffset, GetDocument().DeviceScaleFactor()));

            auto &metrics = textBox->Style().GetFontCascade().MetricsOfPrimaryFont();

            if (!isHorizontalWritingMode)
            {
              if (isBlockFlipped)
              {
                return textVisualRect.X() - metrics.IntAscent();
              }

              return textVisualRect.X() + metrics.IntDescent();
            }

            if (isBlockFlipped)
            {
              return textVisualRect.Y() - metrics.IntDescent();
            }

            return textVisualRect.Y() + metrics.IntAscent();
          }
        }
      }
      // File upload button is display: none (see ::file-selector-button).
      return static_cast<float>(RoundToInt(MarginBoxLogicalHeight(ContainingBlock()->GetWritingMode())));
    }();

    paintInfo.Context().SetFillColor(Style().VisitedDependentColorWithColorFilter(css::PropertyId::Color));

    // Draw the filename
    {
      GraphicsContextStateSaver textStateSaver(paintInfo.Context());

      if (writingMode.IsLineOverLeft())
      {
        textLogicalLeft += LayoutUnit(font.Width(textRun));
        textLogicalTop += font.MetricsOfPrimaryFont().IntAscent();
      }

      auto textOrigin = IntPoint(RoundToInt(textLogicalLeft), RoundToInt(textLogicalTop));
      if (!isHorizontalWritingMode)
      {
        textOrigin = textOrigin.TransposedPoint();

        paintInfo.Context().Translate(FloatPoint(textOrigin));
        if (writingMode.IsLineOverLeft())
        {
          paintInfo.Context().Rotate(-std::numbers::pi_v<float> / 2);
        }
        else
        {
          paintInfo.Context().Rotate(std::numbers::pi_v<float> / 2);
        }
        paintInfo.Context().Translate(-FloatPoint(textOrigin));
      }

      // FIXME: Text of more than one direction is drawn in logical order (drawBidiText), with bidi reordering
      // of text runs.
      (void)paintInfo.Context().DrawText(font, textRun, FloatPoint(textOrigin));
    }

    // FIXME: The file icon, drawn after the button (inputElement().icon()), with the host's file icons.
  }

  void RenderFileUploadControl::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                              LayoutUnit &maxLogicalWidth) const noexcept
  {
    if (ShouldApplySizeOrInlineSizeContainment())
    {
      if (auto logicalWidth = ExplicitIntrinsicInnerLogicalWidth())
      {
        minLogicalWidth = *logicalWidth;
        maxLogicalWidth = *logicalWidth;
      }
      return;
    }
    // Figure out how big the filename space needs to be for a given number of characters (using "0" as the
    // nominal character).
    auto &font = Style().GetFontCascade();
    float minDefaultLabelWidth =
      DefaultWidthNumChars
      * font.Width(ConstructTextRun(u8"0", Style(), ExpansionBehavior::AllowRightOnly()));

    auto label = RenderTheme::Singleton().FileListDefaultLabel(InputElement().Multiple());
    float defaultLabelWidth =
      font.Width(ConstructTextRun(label, Style(), ExpansionBehavior::AllowRightOnly()));
    if (auto *button = UploadButton())
    {
      if (auto *buttonRenderer = DynamicDowncast<RenderBox>(button->Renderer()))
      {
        defaultLabelWidth += buttonRenderer->MaxPreferredLogicalWidth() + AfterButtonSpacing;
      }
    }
    maxLogicalWidth =
      LayoutUnit(static_cast<int32>(std::ceil(std::max(minDefaultLabelWidth, defaultLabelWidth))));

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

  void RenderFileUploadControl::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

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

  dom::PositionWithAffinity RenderFileUploadControl::PositionForPoint(const LayoutPoint &,
                                                                      HitTestSource) noexcept
  {
    return {};
  }

  html::HTMLInputElement *RenderFileUploadControl::UploadButton() const noexcept
  {
    krys_debug_assert(InputElement().UserAgentShadowRoot());
    auto *root = InputElement().UserAgentShadowRoot();
    return root ? DynamicDowncast<html::HTMLInputElement>(root->FirstChild()) : nullptr;
  }

  dom::DOMString RenderFileUploadControl::ButtonValue() const noexcept
  {
    if (auto *button = UploadButton())
    {
      return button->Value();
    }

    return {};
  }

  utf8_string RenderFileUploadControl::FileTextValue() const noexcept
  {
    auto &input = InputElement();
    auto *files = input.Files();
    if (!files)
    {
      return {};
    }
    auto maxWidth = static_cast<float>(MaxFilenameLogicalWidth());
    if (files->Length() && !input.DisplayString().empty())
    {
      if (files->Length() == 1)
      {
        return StringTruncator::CenterTruncate(input.DisplayString(), maxWidth, Style().GetFontCascade());
      }

      return StringTruncator::RightTruncate(input.DisplayString(), maxWidth, Style().GetFontCascade());
    }
    return RenderTheme::Singleton().FileListNameForWidth(files, Style().GetFontCascade(),
                                                         MaxFilenameLogicalWidth(), input.Multiple());
  }
}
