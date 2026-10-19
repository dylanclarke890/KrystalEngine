#pragma once

#include "Krystal.Booey/Graphics/GraphicsStyle.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/SourceBrush.hpp"
#include "Krystal.Booey/Graphics/WindRule.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief A part of a graphics context's state that can change.
  enum class GraphicsContextStateChange : uint8
  {
    FillBrush,
    FillRule,

    StrokeBrush,
    StrokeThickness,
    StrokeStyle,

    CompositeMode,
    DropShadow,
    Style,

    Alpha,
    TextDrawingMode,
    ImageInterpolationQuality,

    ShouldAntialias,
    ShouldSmoothFonts,
    ShouldSubpixelQuantizeFonts,
    ShadowsIgnoreTransforms,
    DrawLuminanceMask,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::GraphicsContextStateChange, 16uz);

namespace krys::boo
{
  /// @brief A graphics context's drawing state, with the changes made to it since they were last applied.
  class GraphicsContextState
  {
  public:
    using Change = GraphicsContextStateChange;
    using ChangeFlags = EnumSet<Change>;

    struct ChangeIndex
    {
      KRYS_NODISCARD constexpr Change ToChange() const noexcept
      {
        return static_cast<Change>(Value);
      }

      uint32 Value;
    };
    KRYS_NODISCARD static constexpr ChangeIndex ToIndex(Change change) noexcept
    {
      return {static_cast<uint32>(change)};
    }

    static constexpr ChangeFlags BasicChangeFlags {Change::StrokeThickness, Change::StrokeBrush,
                                                   Change::FillBrush};
    static constexpr ChangeFlags StrokeChangeFlags {Change::StrokeThickness, Change::StrokeBrush};

    enum class Purpose : uint8
    {
      Initial,
      SaveRestore,
      TransparencyLayer
    };

  private:
    SourceBrush _fillBrush {boo::Color::black};
    SourceBrush _strokeBrush {boo::Color::black};
    ChangeFlags _changeFlags;
    float _strokeThickness {0};
    WindRule _fillRule {WindRule::NonZero};
    boo::StrokeStyle _strokeStyle {boo::StrokeStyle::SolidStroke};

    boo::CompositeMode _compositeMode {CompositeOperator::SourceOver, BlendMode::Normal};
    Maybe<GraphicsDropShadow> _dropShadow;
    Maybe<GraphicsStyle> _style;

    float _alpha {1};
    InterpolationQuality _imageInterpolationQuality {InterpolationQuality::Default};
    TextDrawingModeFlags _textDrawingMode {boo::TextDrawingMode::Fill};

    bool _shouldAntialias {true};
    bool _shouldSmoothFonts {true};
    bool _shouldSubpixelQuantizeFonts {true};
    bool _shadowsIgnoreTransforms {false};
    bool _drawLuminanceMask {false};

    Purpose _purpose {Purpose::Initial};

  public:
    GraphicsContextState(const ChangeFlags & = {},
                         InterpolationQuality = InterpolationQuality::Default) noexcept;

    void Repurpose(Purpose) noexcept;
    KRYS_NODISCARD GraphicsContextState Clone(Purpose) const noexcept;

    KRYS_NODISCARD ChangeFlags Changes() const noexcept
    {
      return _changeFlags;
    }
    void DidApplyChanges() noexcept
    {
      _changeFlags = {};
    }

    KRYS_NODISCARD SourceBrush &FillBrush() noexcept
    {
      return _fillBrush;
    }
    KRYS_NODISCARD const SourceBrush &FillBrush() const noexcept
    {
      return _fillBrush;
    }
    void SetFillBrush(const SourceBrush &brush) noexcept
    {
      SetProperty(Change::FillBrush, &GraphicsContextState::_fillBrush, brush);
    }
    void SetFillColor(const boo::Color &color) noexcept
    {
      SetProperty(Change::FillBrush, &GraphicsContextState::_fillBrush, SourceBrush {color});
    }
    void SetFillGradient(Ref<Gradient> &&gradient, const AffineTransform &spaceTransform) noexcept
    {
      _fillBrush.SetGradient(krys::move(gradient), spaceTransform);
      _changeFlags.Add(Change::FillBrush);
    }

    KRYS_NODISCARD WindRule FillRule() const noexcept
    {
      return _fillRule;
    }
    void SetFillRule(WindRule fillRule) noexcept
    {
      SetProperty(Change::FillRule, &GraphicsContextState::_fillRule, fillRule);
    }

    KRYS_NODISCARD SourceBrush &StrokeBrush() noexcept
    {
      return _strokeBrush;
    }
    KRYS_NODISCARD const SourceBrush &StrokeBrush() const noexcept
    {
      return _strokeBrush;
    }
    void SetStrokeBrush(const SourceBrush &brush) noexcept
    {
      SetProperty(Change::StrokeBrush, &GraphicsContextState::_strokeBrush, brush);
    }
    void SetStrokeColor(const boo::Color &color) noexcept
    {
      SetProperty(Change::StrokeBrush, &GraphicsContextState::_strokeBrush, SourceBrush {color});
    }
    void SetStrokeGradient(Ref<Gradient> &&gradient, const AffineTransform &spaceTransform) noexcept
    {
      _strokeBrush.SetGradient(krys::move(gradient), spaceTransform);
      _changeFlags.Add(Change::StrokeBrush);
    }

    KRYS_NODISCARD float StrokeThickness() const noexcept
    {
      return _strokeThickness;
    }
    void SetStrokeThickness(float strokeThickness) noexcept
    {
      SetProperty(Change::StrokeThickness, &GraphicsContextState::_strokeThickness, strokeThickness);
    }

    KRYS_NODISCARD boo::StrokeStyle GetStrokeStyle() const noexcept
    {
      return _strokeStyle;
    }
    void SetStrokeStyle(boo::StrokeStyle strokeStyle) noexcept
    {
      SetProperty(Change::StrokeStyle, &GraphicsContextState::_strokeStyle, strokeStyle);
    }

    KRYS_NODISCARD const boo::CompositeMode &GetCompositeMode() const noexcept
    {
      return _compositeMode;
    }
    void SetCompositeMode(boo::CompositeMode compositeMode) noexcept
    {
      SetProperty(Change::CompositeMode, &GraphicsContextState::_compositeMode, compositeMode);
    }

    KRYS_NODISCARD const Maybe<GraphicsDropShadow> &DropShadow() const noexcept
    {
      return _dropShadow;
    }
    void SetDropShadow(const Maybe<GraphicsDropShadow> &dropShadow) noexcept
    {
      SetProperty(Change::DropShadow, &GraphicsContextState::_dropShadow, dropShadow);
    }

    KRYS_NODISCARD const Maybe<GraphicsStyle> &Style() const noexcept
    {
      return _style;
    }
    void SetStyle(const Maybe<GraphicsStyle> &style) noexcept
    {
      SetProperty(Change::Style, &GraphicsContextState::_style, style);
    }

    KRYS_NODISCARD float Alpha() const noexcept
    {
      return _alpha;
    }
    void SetAlpha(float alpha) noexcept
    {
      SetProperty(Change::Alpha, &GraphicsContextState::_alpha, alpha);
    }

    KRYS_NODISCARD InterpolationQuality ImageInterpolationQuality() const noexcept
    {
      return _imageInterpolationQuality;
    }
    void SetImageInterpolationQuality(InterpolationQuality imageInterpolationQuality) noexcept
    {
      SetProperty(Change::ImageInterpolationQuality, &GraphicsContextState::_imageInterpolationQuality,
                  imageInterpolationQuality);
    }

    KRYS_NODISCARD TextDrawingModeFlags GetTextDrawingMode() const noexcept
    {
      return _textDrawingMode;
    }
    void SetTextDrawingMode(TextDrawingModeFlags textDrawingMode) noexcept
    {
      SetProperty(Change::TextDrawingMode, &GraphicsContextState::_textDrawingMode, textDrawingMode);
    }

    KRYS_NODISCARD bool ShouldAntialias() const noexcept
    {
      return _shouldAntialias;
    }
    void SetShouldAntialias(bool shouldAntialias) noexcept
    {
      SetProperty(Change::ShouldAntialias, &GraphicsContextState::_shouldAntialias, shouldAntialias);
    }

    KRYS_NODISCARD bool ShouldSmoothFonts() const noexcept
    {
      return _shouldSmoothFonts;
    }
    void SetShouldSmoothFonts(bool shouldSmoothFonts) noexcept
    {
      SetProperty(Change::ShouldSmoothFonts, &GraphicsContextState::_shouldSmoothFonts, shouldSmoothFonts);
    }

    KRYS_NODISCARD bool ShouldSubpixelQuantizeFonts() const noexcept
    {
      return _shouldSubpixelQuantizeFonts;
    }
    void SetShouldSubpixelQuantizeFonts(bool shouldSubpixelQuantizeFonts) noexcept
    {
      SetProperty(Change::ShouldSubpixelQuantizeFonts, &GraphicsContextState::_shouldSubpixelQuantizeFonts,
                  shouldSubpixelQuantizeFonts);
    }

    KRYS_NODISCARD bool ShadowsIgnoreTransforms() const noexcept
    {
      return _shadowsIgnoreTransforms;
    }
    void SetShadowsIgnoreTransforms(bool shadowsIgnoreTransforms) noexcept
    {
      SetProperty(Change::ShadowsIgnoreTransforms, &GraphicsContextState::_shadowsIgnoreTransforms,
                  shadowsIgnoreTransforms);
    }

    KRYS_NODISCARD bool DrawLuminanceMask() const noexcept
    {
      return _drawLuminanceMask;
    }
    void SetDrawLuminanceMask(bool drawLuminanceMask) noexcept
    {
      SetProperty(Change::DrawLuminanceMask, &GraphicsContextState::_drawLuminanceMask, drawLuminanceMask);
    }

    void MergeLastChanges(const GraphicsContextState &,
                          const Maybe<GraphicsContextState> &lastDrawingState = std::nullopt) noexcept;
    void MergeSingleChange(const GraphicsContextState &, ChangeIndex,
                           const Maybe<GraphicsContextState> &lastDrawingState = std::nullopt) noexcept;
    void MergeAllChanges(const GraphicsContextState &) noexcept;

    KRYS_NODISCARD Purpose GetPurpose() const noexcept
    {
      return _purpose;
    }

    TextStream &Dump(TextStream &) const noexcept;

  private:
    template <typename T>
    void SetProperty(Change change, T GraphicsContextState::*property, const T &value) noexcept
    {
      if (this->*property == value)
      {
        return;
      }
      this->*property = value;
      _changeFlags.Add(change);
    }
  };

  TextStream &operator<<(TextStream &ts, GraphicsContextState::Change change) noexcept;
  TextStream &operator<<(TextStream &ts, const GraphicsContextState &state) noexcept;
}
