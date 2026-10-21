#include "Krystal.Booey/Rendering/NinePieceImagePainter.hpp"
#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImage.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::boo
{
  namespace
  {
    // Used for array indexing, so not an enum class.
    enum ImagePiece
    {
      MinPiece = 0,
      TopLeftPiece = MinPiece,
      LeftPiece,
      BottomLeftPiece,
      TopRightPiece,
      RightPiece,
      BottomRightPiece,
      TopPiece,
      BottomPiece,
      MiddlePiece,
      MaxPiece
    };

    using PieceRects = Array<FloatRect, MaxPiece>;
    using PieceScales = Array<FloatSize, MaxPiece>;

    KRYS_NODISCARD bool IsCornerPiece(ImagePiece piece) noexcept
    {
      return piece == TopLeftPiece || piece == TopRightPiece || piece == BottomLeftPiece
             || piece == BottomRightPiece;
    }

    KRYS_NODISCARD bool IsHorizontalPiece(ImagePiece piece) noexcept
    {
      return piece == TopPiece || piece == BottomPiece || piece == MiddlePiece;
    }

    KRYS_NODISCARD bool IsVerticalPiece(ImagePiece piece) noexcept
    {
      return piece == LeftPiece || piece == RightPiece || piece == MiddlePiece;
    }

    template <typename TWidthValue>
    KRYS_NODISCARD LayoutUnit ComputeSlice(const TWidthValue &length, LayoutUnit width, LayoutUnit slice,
                                           LayoutUnit extent) noexcept
    {
      return length.SwitchOn([&](const typename TWidthValue::LengthPercentage &value)
                             { return style::Evaluate<LayoutUnit>(value, extent, style::ZoomNeeded {}); },
                             [&](const typename TWidthValue::Number &value)
                             { return LayoutUnit {value.Value * width.ToFloat()}; },
                             [&](const css::keywords::Auto &) { return slice; });
    }

    template <typename TWidthValues>
    KRYS_NODISCARD LayoutBoxExtent ComputeSlices(const LayoutSize &size, const TWidthValues &widths,
                                                 const FloatBoxExtent &borderWidths,
                                                 const LayoutBoxExtent &slices) noexcept
    {
      return {
        ComputeSlice(widths.Values.Top(), LayoutUnit(borderWidths.Top()), slices.Top(), size.Height()),
        ComputeSlice(widths.Values.Right(), LayoutUnit(borderWidths.Right()), slices.Right(), size.Width()),
        ComputeSlice(widths.Values.Bottom(), LayoutUnit(borderWidths.Bottom()), slices.Bottom(),
                     size.Height()),
        ComputeSlice(widths.Values.Left(), LayoutUnit(borderWidths.Left()), slices.Left(), size.Width()),
      };
    }

    template <typename TSliceValue>
    KRYS_NODISCARD LayoutUnit EvaluateSlice(const TSliceValue &value, LayoutUnit extent) noexcept
    {
      return value.SwitchOn([&](const typename TSliceValue::Number &number)
                            { return LayoutUnit(number.Value); },
                            [&](const typename TSliceValue::Percentage &percentage)
                            { return LayoutUnit(percentage.Value / 100.f * extent.ToFloat()); });
    }

    template <typename TSliceValues>
    KRYS_NODISCARD LayoutBoxExtent ComputeSlices(const LayoutSize &size, const TSliceValues &slices,
                                                 int32 scaleFactor) noexcept
    {
      return {
        std::min(size.Height(), EvaluateSlice(slices.Values.Top(), size.Height())) * scaleFactor,
        std::min(size.Width(), EvaluateSlice(slices.Values.Right(), size.Width())) * scaleFactor,
        std::min(size.Height(), EvaluateSlice(slices.Values.Bottom(), size.Height())) * scaleFactor,
        std::min(size.Width(), EvaluateSlice(slices.Values.Left(), size.Width())) * scaleFactor,
      };
    }

    void ScaleSlicesIfNeeded(const LayoutSize &size, LayoutBoxExtent &slices,
                             float deviceScaleFactor) noexcept
    {
      LayoutUnit width = std::max(LayoutUnit(1 / deviceScaleFactor), slices.Left() + slices.Right());
      LayoutUnit height = std::max(LayoutUnit(1 / deviceScaleFactor), slices.Top() + slices.Bottom());

      float sliceScaleFactor =
        std::min(size.Width().ToFloat() / width.ToFloat(), size.Height().ToFloat() / height.ToFloat());

      if (sliceScaleFactor >= 1)
      {
        return;
      }

      // All slices are reduced by multiplying them by sliceScaleFactor.
      slices.Top() *= sliceScaleFactor;
      slices.Right() *= sliceScaleFactor;
      slices.Bottom() *= sliceScaleFactor;
      slices.Left() *= sliceScaleFactor;
    }

    KRYS_NODISCARD bool IsEmptyPieceRect(ImagePiece piece, const PieceRects &destinationRects,
                                         const PieceRects &sourceRects) noexcept
    {
      return destinationRects[piece].IsEmpty() || sourceRects[piece].IsEmpty();
    }

    KRYS_NODISCARD PieceRects ComputeNineRects(const FloatRect &outer, const LayoutBoxExtent &slices,
                                               float deviceScaleFactor) noexcept
    {
      FloatRect inner = outer;
      inner.Move(slices.Left().ToFloat(), slices.Top().ToFloat());
      inner.Contract((slices.Left() + slices.Right()).ToFloat(), (slices.Top() + slices.Bottom()).ToFloat());
      krys_debug_assert(outer.Contains(inner));

      PieceRects rects {};

      auto outerX = LayoutUnit(outer.X());
      auto outerY = LayoutUnit(outer.Y());
      auto innerX = LayoutUnit(inner.X());
      auto innerY = LayoutUnit(inner.Y());
      auto innerMaxX = LayoutUnit(inner.MaxX());
      auto innerMaxY = LayoutUnit(inner.MaxY());
      auto innerHeight = LayoutUnit(inner.Height());
      auto innerWidth = LayoutUnit(inner.Width());

      rects[TopLeftPiece] =
        SnapRectToDevicePixels(outerX, outerY, slices.Left(), slices.Top(), deviceScaleFactor);
      rects[BottomLeftPiece] =
        SnapRectToDevicePixels(outerX, innerMaxY, slices.Left(), slices.Bottom(), deviceScaleFactor);
      rects[LeftPiece] =
        SnapRectToDevicePixels(outerX, innerY, slices.Left(), innerHeight, deviceScaleFactor);

      rects[TopRightPiece] =
        SnapRectToDevicePixels(innerMaxX, outerY, slices.Right(), slices.Top(), deviceScaleFactor);
      rects[BottomRightPiece] =
        SnapRectToDevicePixels(innerMaxX, innerMaxY, slices.Right(), slices.Bottom(), deviceScaleFactor);
      rects[RightPiece] =
        SnapRectToDevicePixels(innerMaxX, innerY, slices.Right(), innerHeight, deviceScaleFactor);

      rects[TopPiece] = SnapRectToDevicePixels(innerX, outerY, innerWidth, slices.Top(), deviceScaleFactor);
      rects[BottomPiece] =
        SnapRectToDevicePixels(innerX, innerMaxY, innerWidth, slices.Bottom(), deviceScaleFactor);

      rects[MiddlePiece] = SnapRectToDevicePixels(innerX, innerY, innerWidth, innerHeight, deviceScaleFactor);

      return rects;
    }

    KRYS_NODISCARD FloatSize ComputeSideTileScale(ImagePiece piece, const PieceRects &destinationRects,
                                                  const PieceRects &sourceRects) noexcept
    {
      krys_debug_assert(!IsCornerPiece(piece) && piece != MiddlePiece);
      if (IsEmptyPieceRect(piece, destinationRects, sourceRects))
      {
        return FloatSize(1, 1);
      }

      float scale;
      if (IsHorizontalPiece(piece))
      {
        scale = destinationRects[piece].Height() / sourceRects[piece].Height();
      }
      else
      {
        scale = destinationRects[piece].Width() / sourceRects[piece].Width();
      }

      return FloatSize(scale, scale);
    }

    KRYS_NODISCARD FloatSize ComputeMiddleTileScale(const PieceScales &scales,
                                                    const PieceRects &destinationRects,
                                                    const PieceRects &sourceRects, NinePieceImageRule hRule,
                                                    NinePieceImageRule vRule) noexcept
    {
      FloatSize scale(1, 1);
      if (IsEmptyPieceRect(MiddlePiece, destinationRects, sourceRects))
      {
        return scale;
      }

      // Unlike the side pieces, the middle piece can have "stretch" specified in one axis but not the other.
      // In fact the side pieces don't even use the scale factor unless they have a rule other than "stretch".
      if (hRule == NinePieceImageRule::Stretch)
      {
        scale.SetWidth(destinationRects[MiddlePiece].Width() / sourceRects[MiddlePiece].Width());
      }
      else if (!IsEmptyPieceRect(TopPiece, destinationRects, sourceRects))
      {
        scale.SetWidth(scales[TopPiece].Width());
      }
      else if (!IsEmptyPieceRect(BottomPiece, destinationRects, sourceRects))
      {
        scale.SetWidth(scales[BottomPiece].Width());
      }

      if (vRule == NinePieceImageRule::Stretch)
      {
        scale.SetHeight(destinationRects[MiddlePiece].Height() / sourceRects[MiddlePiece].Height());
      }
      else if (!IsEmptyPieceRect(LeftPiece, destinationRects, sourceRects))
      {
        scale.SetHeight(scales[LeftPiece].Height());
      }
      else if (!IsEmptyPieceRect(RightPiece, destinationRects, sourceRects))
      {
        scale.SetHeight(scales[RightPiece].Height());
      }

      return scale;
    }

    KRYS_NODISCARD PieceScales ComputeTileScales(const PieceRects &destinationRects,
                                                 const PieceRects &sourceRects, NinePieceImageRule hRule,
                                                 NinePieceImageRule vRule) noexcept
    {
      PieceScales scales;
      scales.fill(FloatSize(1, 1));

      scales[TopPiece] = ComputeSideTileScale(TopPiece, destinationRects, sourceRects);
      scales[RightPiece] = ComputeSideTileScale(RightPiece, destinationRects, sourceRects);
      scales[BottomPiece] = ComputeSideTileScale(BottomPiece, destinationRects, sourceRects);
      scales[LeftPiece] = ComputeSideTileScale(LeftPiece, destinationRects, sourceRects);

      scales[MiddlePiece] = ComputeMiddleTileScale(scales, destinationRects, sourceRects, hRule, vRule);

      return scales;
    }

    /// The interpolation an image is drawn with by `image-rendering`.
    /// FIXME: The image quality controller (ImageQualityController::interpolationQualityFromStyle) comes with
    /// bitmap images; this is its mapping.
    KRYS_NODISCARD InterpolationQuality InterpolationQualityFromStyle(const RenderStyle &style) noexcept
    {
      switch (style.GetImageRendering())
      {
        case ImageRendering::OptimizeSpeed:   return InterpolationQuality::Low;
        case ImageRendering::CrispEdges:
        case ImageRendering::Pixelated:       return InterpolationQuality::DoNotInterpolate;
        case ImageRendering::OptimizeQuality:
        case ImageRendering::Auto:            break;
      }
      return InterpolationQuality::Default;
    }

    KRYS_NODISCARD Image::TileRule ToTileRule(NinePieceImageRule rule) noexcept
    {
      return static_cast<Image::TileRule>(rule);
    }

    template <typename T>
    void PaintNinePieceImage(const T &ninePieceImage, GraphicsContext &graphicsContext,
                             const RenderElement *renderer, const RenderStyle &style,
                             const LayoutRect &destination, const LayoutSize &source, float deviceScaleFactor,
                             ImagePaintingOptions options) noexcept
    {
      auto styleImage = ninePieceImage.Source.TryStyleImage();
      krys_debug_assert(styleImage);
      krys_debug_assert(styleImage->IsLoaded(renderer));

      auto sourceSlices =
        ComputeSlices(source, ninePieceImage.Slice, static_cast<int32>(styleImage->ImageScaleFactor()));
      auto destinationSlices = ComputeSlices(
        destination.Size(), ninePieceImage.Width,
        style::Evaluate<FloatBoxExtent>(style.BorderWidth(), style::ZoomNeeded {}), sourceSlices);

      ScaleSlicesIfNeeded(destination.Size(), destinationSlices, deviceScaleFactor);

      auto destinationRects = ComputeNineRects(FloatRect(destination), destinationSlices, deviceScaleFactor);
      auto sourceRects =
        ComputeNineRects(FloatRect(FloatPoint(), FloatSize(source)), sourceSlices, deviceScaleFactor);

      auto tileScales =
        ComputeTileScales(destinationRects, sourceRects, ninePieceImage.Repeat.HorizontalRule(),
                          ninePieceImage.Repeat.VerticalRule());

      RefPtr image = styleImage->GetImage(renderer, FloatSize(source), graphicsContext);
      if (!image)
      {
        return;
      }

      InterpolationQualityMaintainer interpolationMaintainer(graphicsContext,
                                                             InterpolationQualityFromStyle(style));

      for (int32 index = MinPiece; index < MaxPiece; ++index)
      {
        auto piece = static_cast<ImagePiece>(index);
        if ((piece == MiddlePiece && !ninePieceImage.Slice.Fill)
            || IsEmptyPieceRect(piece, destinationRects, sourceRects))
        {
          continue;
        }

        if (IsCornerPiece(piece))
        {
          (void)graphicsContext.DrawImage(*image, destinationRects[piece], sourceRects[piece], options);
          continue;
        }

        auto hRule =
          IsHorizontalPiece(piece) ? ToTileRule(ninePieceImage.Repeat.HorizontalRule()) : Image::StretchTile;
        auto vRule =
          IsVerticalPiece(piece) ? ToTileRule(ninePieceImage.Repeat.VerticalRule()) : Image::StretchTile;

        (void)graphicsContext.DrawTiledImage(*image, destinationRects[piece], sourceRects[piece],
                                             tileScales[piece], hRule, vRule, options);
      }
    }
  }

  void NinePieceImagePainter::Paint(const style::BorderImage &ninePieceImage,
                                    GraphicsContext &graphicsContext, const RenderElement *renderer,
                                    const RenderStyle &style, const LayoutRect &destination,
                                    const LayoutSize &source, float deviceScaleFactor,
                                    ImagePaintingOptions options) noexcept
  {
    PaintNinePieceImage(ninePieceImage, graphicsContext, renderer, style, destination, source,
                        deviceScaleFactor, options);
  }

  void NinePieceImagePainter::Paint(const style::MaskBorder &ninePieceImage, GraphicsContext &graphicsContext,
                                    const RenderElement *renderer, const RenderStyle &style,
                                    const LayoutRect &destination, const LayoutSize &source,
                                    float deviceScaleFactor, ImagePaintingOptions options) noexcept
  {
    PaintNinePieceImage(ninePieceImage, graphicsContext, renderer, style, destination, source,
                        deviceScaleFactor, options);
  }
}
