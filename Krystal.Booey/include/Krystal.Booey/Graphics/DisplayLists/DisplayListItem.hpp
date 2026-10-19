#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class GraphicsContext;
}

namespace krys::boo::display_list
{
  class ApplyDeviceScaleFactor;
  class BeginTransparencyLayer;
  class BeginTransparencyLayerWithCompositeMode;
  class ClearRect;
  class Clip;
  class ClipRoundedRect;
  class ClipOut;
  class ClipOutRoundedRect;
  class ClipOutToPath;
  class ClipPath;
  class ConcatenateCTM;
  class DrawDotsForDocumentMarker;
  class DrawEllipse;
  class DrawFocusRingPath;
  class DrawFocusRingRects;
  class DrawGlyphs;
  class DrawDisplayList;
  class DrawImageBuffer;
  class DrawPatternImageBuffer;
  class DrawPlaceholder;
  class DrawLine;
  class DrawLinesForText;
  class DrawPath;
  class DrawRect;
  class EndTransparencyLayer;
  class FillCompositedRect;
  class FillEllipse;
  class FillPath;
  class FillRect;
  class FillRectWithColor;
  class FillRectWithGradient;
  class FillRectWithGradientAndSpaceTransform;
  class FillRectWithRoundedHole;
  class FillRoundedRect;
  class ResetClip;
  class Restore;
  class Rotate;
  class Save;
  class Scale;
  class SetCTM;
  class SetInlineFillColor;
  class SetInlineStroke;
  class SetLineCap;
  class SetLineDash;
  class SetLineJoin;
  class SetMiterLimit;
  class SetState;
  class StrokeEllipse;
  class StrokePath;
  class StrokeRect;
  class Translate;
  class BeginPage;
  class EndPage;
  class SetURLForRect;

  // FIXME: The image, filter and form control items (ClipToImageBuffer, DrawControlPart,
  // DrawFilteredImageBuffer, DrawImageBuffer, DrawNativeImage, DrawPatternNativeImage,
  // DrawPatternImageBuffer, DrawSystemImage) come with images.
  using Item =
    Variant<ApplyDeviceScaleFactor, BeginTransparencyLayer, BeginTransparencyLayerWithCompositeMode,
            ClearRect, Clip, ClipRoundedRect, ClipOut, ClipOutRoundedRect, ClipOutToPath, ClipPath,
            ConcatenateCTM, DrawDotsForDocumentMarker, DrawEllipse, DrawFocusRingPath, DrawFocusRingRects,
            DrawGlyphs, DrawDisplayList, DrawImageBuffer, DrawPatternImageBuffer, DrawPlaceholder, DrawLine,
            DrawLinesForText, DrawPath, DrawRect, EndTransparencyLayer, FillCompositedRect, FillEllipse,
            FillPath, FillRect, FillRectWithColor, FillRectWithGradient,
            FillRectWithGradientAndSpaceTransform, FillRectWithRoundedHole, FillRoundedRect, ResetClip,
            Restore, Rotate, Save, Scale, SetCTM, SetInlineFillColor, SetInlineStroke, SetLineCap,
            SetLineDash, SetLineJoin, SetMiterLimit, SetState, StrokeEllipse, StrokePath, StrokeRect,
            Translate, BeginPage, EndPage, SetURLForRect>;

  enum class AsTextFlag : uint8
  {
    IncludePlatformOperations,
    IncludeResourceIdentifiers,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::display_list::AsTextFlag, 2uz);

namespace krys::boo::display_list
{
  void ApplyItem(GraphicsContext &, const Item &) noexcept;
  KRYS_NODISCARD bool ShouldDumpItem(const Item &, EnumSet<AsTextFlag>) noexcept;
  void DumpItem(TextStream &, const Item &, EnumSet<AsTextFlag>) noexcept;
  TextStream &operator<<(TextStream &, const Item &) noexcept;
}
