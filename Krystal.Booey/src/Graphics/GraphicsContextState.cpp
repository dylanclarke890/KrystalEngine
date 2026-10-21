#include "Krystal.Booey/Graphics/GraphicsContextState.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Colour/ColourTextStream.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  GraphicsContextState::GraphicsContextState(const ChangeFlags &changeFlags,
                                             InterpolationQuality imageInterpolationQuality) noexcept
      : _changeFlags(changeFlags), _imageInterpolationQuality(imageInterpolationQuality)
  {
  }

  void GraphicsContextState::Repurpose(Purpose purpose) noexcept
  {
    if (purpose == Purpose::Initial)
    {
      _changeFlags = {};
    }

    _purpose = purpose;
  }

  GraphicsContextState GraphicsContextState::Clone(Purpose purpose) const noexcept
  {
    auto clone = *this;
    clone.Repurpose(purpose);
    return clone;
  }

  void GraphicsContextState::MergeLastChanges(const GraphicsContextState &state,
                                              const Maybe<GraphicsContextState> &lastDrawingState) noexcept
  {
    for (auto change : state.Changes())
    {
      MergeSingleChange(state, ToIndex(change), lastDrawingState);
    }
  }

  void GraphicsContextState::MergeSingleChange(const GraphicsContextState &state, ChangeIndex changeIndex,
                                               const Maybe<GraphicsContextState> &lastDrawingState) noexcept
  {
    auto mergeChange = [&](auto GraphicsContextState::*property)
    {
      if (this->*property == state.*property)
      {
        return;
      }
      this->*property = state.*property;
      _changeFlags.Set(changeIndex.ToChange(),
                       !lastDrawingState || (*lastDrawingState).*property != this->*property);
    };

    switch (changeIndex.ToChange())
    {
      case Change::FillBrush:       mergeChange(&GraphicsContextState::_fillBrush); break;
      case Change::FillRule:        mergeChange(&GraphicsContextState::_fillRule); break;

      case Change::StrokeBrush:     mergeChange(&GraphicsContextState::_strokeBrush); break;
      case Change::StrokeThickness: mergeChange(&GraphicsContextState::_strokeThickness); break;
      case Change::StrokeStyle:     mergeChange(&GraphicsContextState::_strokeStyle); break;

      case Change::CompositeMode:   mergeChange(&GraphicsContextState::_compositeMode); break;
      case Change::DropShadow:      mergeChange(&GraphicsContextState::_dropShadow); break;
      case Change::Style:           mergeChange(&GraphicsContextState::_style); break;

      case Change::Alpha:           mergeChange(&GraphicsContextState::_alpha); break;
      case Change::TextDrawingMode: mergeChange(&GraphicsContextState::_textDrawingMode); break;
      case Change::ImageInterpolationQuality:
        mergeChange(&GraphicsContextState::_imageInterpolationQuality);
        break;

      case Change::ShouldAntialias:   mergeChange(&GraphicsContextState::_shouldAntialias); break;
      case Change::ShouldSmoothFonts: mergeChange(&GraphicsContextState::_shouldSmoothFonts); break;
      case Change::ShouldSubpixelQuantizeFonts:
        mergeChange(&GraphicsContextState::_shouldSubpixelQuantizeFonts);
        break;
      case Change::ShadowsIgnoreTransforms:
        mergeChange(&GraphicsContextState::_shadowsIgnoreTransforms);
        break;
      case Change::DrawLuminanceMask: mergeChange(&GraphicsContextState::_drawLuminanceMask); break;
    }
  }

  void GraphicsContextState::MergeAllChanges(const GraphicsContextState &state) noexcept
  {
    auto mergeChange = [&](Change change, auto GraphicsContextState::*property)
    {
      if (this->*property == state.*property)
      {
        return;
      }
      this->*property = state.*property;
      _changeFlags.Add(change);
    };

    mergeChange(Change::FillBrush, &GraphicsContextState::_fillBrush);
    mergeChange(Change::FillRule, &GraphicsContextState::_fillRule);

    mergeChange(Change::StrokeBrush, &GraphicsContextState::_strokeBrush);
    mergeChange(Change::StrokeThickness, &GraphicsContextState::_strokeThickness);
    mergeChange(Change::StrokeStyle, &GraphicsContextState::_strokeStyle);

    mergeChange(Change::CompositeMode, &GraphicsContextState::_compositeMode);
    mergeChange(Change::DropShadow, &GraphicsContextState::_dropShadow);
    mergeChange(Change::Style, &GraphicsContextState::_style);

    mergeChange(Change::Alpha, &GraphicsContextState::_alpha);
    mergeChange(Change::ImageInterpolationQuality, &GraphicsContextState::_imageInterpolationQuality);
    mergeChange(Change::TextDrawingMode, &GraphicsContextState::_textDrawingMode);

    mergeChange(Change::ShouldAntialias, &GraphicsContextState::_shouldAntialias);
    mergeChange(Change::ShouldSmoothFonts, &GraphicsContextState::_shouldSmoothFonts);
    mergeChange(Change::ShouldSubpixelQuantizeFonts, &GraphicsContextState::_shouldSubpixelQuantizeFonts);
    mergeChange(Change::ShadowsIgnoreTransforms, &GraphicsContextState::_shadowsIgnoreTransforms);
    mergeChange(Change::DrawLuminanceMask, &GraphicsContextState::_drawLuminanceMask);
  }

  namespace
  {
    KRYS_NODISCARD utf8_stringview StateChangeName(GraphicsContextState::Change change) noexcept
    {
      using Change = GraphicsContextState::Change;
      switch (change)
      {
        case Change::FillBrush:                   return u8"fill-brush";
        case Change::FillRule:                    return u8"fill-rule";
        case Change::StrokeBrush:                 return u8"stroke-brush";
        case Change::StrokeThickness:             return u8"stroke-thickness";
        case Change::StrokeStyle:                 return u8"stroke-style";
        case Change::CompositeMode:               return u8"composite-mode";
        case Change::DropShadow:                  return u8"drop-shadow";
        case Change::Style:                       return u8"style";
        case Change::Alpha:                       return u8"alpha";
        case Change::ImageInterpolationQuality:   return u8"image-interpolation-quality";
        case Change::TextDrawingMode:             return u8"text-drawing-mode";
        case Change::ShouldAntialias:             return u8"should-antialias";
        case Change::ShouldSmoothFonts:           return u8"should-smooth-fonts";
        case Change::ShouldSubpixelQuantizeFonts: return u8"should-subpixel-quantize-fonts";
        case Change::ShadowsIgnoreTransforms:     return u8"shadows-ignore-transforms";
        case Change::DrawLuminanceMask:           return u8"draw-luminance-mask";
      }

      krys_unreachable();
    }
  }

  TextStream &GraphicsContextState::Dump(TextStream &ts) const noexcept
  {
    auto dump = [&](Change change, auto GraphicsContextState::*property)
    {
      if (_changeFlags.Contains(change))
      {
        ts.DumpProperty(StateChangeName(change), this->*property);
      }
    };

    ts.DumpProperty(u8"change-flags", _changeFlags);

    dump(Change::FillBrush, &GraphicsContextState::_fillBrush);
    dump(Change::FillRule, &GraphicsContextState::_fillRule);

    dump(Change::StrokeBrush, &GraphicsContextState::_strokeBrush);
    dump(Change::StrokeThickness, &GraphicsContextState::_strokeThickness);
    dump(Change::StrokeStyle, &GraphicsContextState::_strokeStyle);

    dump(Change::CompositeMode, &GraphicsContextState::_compositeMode);
    dump(Change::DropShadow, &GraphicsContextState::_dropShadow);
    dump(Change::Style, &GraphicsContextState::_style);

    dump(Change::Alpha, &GraphicsContextState::_alpha);
    dump(Change::ImageInterpolationQuality, &GraphicsContextState::_imageInterpolationQuality);
    dump(Change::TextDrawingMode, &GraphicsContextState::_textDrawingMode);

    dump(Change::ShouldAntialias, &GraphicsContextState::_shouldAntialias);
    dump(Change::ShouldSmoothFonts, &GraphicsContextState::_shouldSmoothFonts);
    dump(Change::ShouldSubpixelQuantizeFonts, &GraphicsContextState::_shouldSubpixelQuantizeFonts);
    dump(Change::ShadowsIgnoreTransforms, &GraphicsContextState::_shadowsIgnoreTransforms);
    dump(Change::DrawLuminanceMask, &GraphicsContextState::_drawLuminanceMask);
    return ts;
  }

  TextStream &operator<<(TextStream &ts, GraphicsContextState::Change change) noexcept
  {
    return ts << StateChangeName(change);
  }

  TextStream &operator<<(TextStream &ts, const GraphicsContextState &state) noexcept
  {
    return state.Dump(ts);
  }
}
