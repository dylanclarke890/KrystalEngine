#include "Krystal.Booey/Graphics/Images/BrokenImageIcon.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"

namespace krys::boo
{
  BrokenImageIcon &BrokenImageIcon::Singleton() noexcept
  {
    static BrokenImageIcon &icon = *new BrokenImageIcon();
    return icon;
  }

  ImageDrawResult BrokenImageIcon::Draw(GraphicsContext &context, const FloatRect &dstRect,
                                        const FloatRect &srcRect, ImagePaintingOptions options) noexcept
  {
    GraphicsContextStateSaver stateSaver(context);
    context.SetCompositeOperation(options.GetCompositeOperator(), options.GetBlendMode());
    context.Clip(dstRect);
    context.Translate(dstRect.Location());
    if (dstRect.Size() != srcRect.Size())
    {
      context.Scale(dstRect.Size() / srcRect.Size());
    }
    context.Translate(-srcRect.Location());

    // A sheet with a picture of a hill under the sun, in a 16 by 16 grid.
    constexpr auto sheetColor = SRGBA<uint8> {250, 250, 250};
    constexpr auto edgeColor = SRGBA<uint8> {128, 134, 142};
    constexpr auto skyColor = SRGBA<uint8> {206, 224, 245};
    constexpr auto hillColor = SRGBA<uint8> {84, 150, 98};
    constexpr auto sunColor = SRGBA<uint8> {236, 183, 58};

    context.FillRect(FloatRect(2, 1, 12, 14), sheetColor);
    context.FillRect(FloatRect(3.5f, 2.5f, 9, 11), skyColor);
    context.SetFillColor(sunColor);
    context.FillEllipse(FloatRect(9, 3.5f, 2.5f, 2.5f));

    Path hill;
    hill.MoveTo({3.5f, 13.5f});
    hill.AddLineTo({3.5f, 10});
    hill.AddLineTo({7, 6.5f});
    hill.AddLineTo({10, 9.5f});
    hill.AddLineTo({12.5f, 8});
    hill.AddLineTo({12.5f, 13.5f});
    hill.CloseSubpath();
    context.SetFillColor(hillColor);
    context.FillPath(hill);

    context.SetStrokeColor(edgeColor);
    context.SetStrokeThickness(1);
    context.StrokeRect(FloatRect(2.5f, 1.5f, 11, 13), 1);

    // The tear: a jagged gap from edge to edge, in the sheet's colour.
    Path tear;
    tear.MoveTo({1, 9.5f});
    tear.AddLineTo({4.5f, 7});
    tear.AddLineTo({7, 9});
    tear.AddLineTo({10, 6});
    tear.AddLineTo({12, 7.5f});
    tear.AddLineTo({15, 5});
    context.SetStrokeColor(sheetColor);
    context.SetStrokeThickness(1.5f);
    context.StrokePath(tear);

    return ImageDrawResult::DidDraw;
  }
}
