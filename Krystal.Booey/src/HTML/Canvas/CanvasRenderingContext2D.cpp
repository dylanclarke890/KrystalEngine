#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::html
{
  UniquePtr<CanvasRenderingContext2D>
    CanvasRenderingContext2D::Create(CanvasBase &canvas, CanvasRenderingContext2DSettings &&settings,
                                     bool usesCSSCompatibilityParseMode) noexcept
  {
    return CreateUnique<CanvasRenderingContext2D>(canvas, krys::move(settings),
                                                  usesCSSCompatibilityParseMode);
  }

  CanvasRenderingContext2D::CanvasRenderingContext2D(CanvasBase &canvas,
                                                     CanvasRenderingContext2DSettings &&settings,
                                                     bool usesCSSCompatibilityParseMode) noexcept
      : CanvasRenderingContext2DBase(canvas, Type::CanvasElement2D, krys::move(settings),
                                     usesCSSCompatibilityParseMode)
  {
  }

  CanvasRenderingContext2D::~CanvasRenderingContext2D() noexcept = default;

  HTMLCanvasElement &CanvasRenderingContext2D::Canvas() const noexcept
  {
    return Downcast<HTMLCanvasElement>(GetCanvasBase());
  }

  TextDirection CanvasRenderingContext2D::ToTextDirection(Direction direction,
                                                          const RenderStyle **computedStyle) const noexcept
  {
    const auto *style =
      computedStyle || direction == Direction::Inherit ? Canvas().ExistingComputedStyle() : nullptr;
    if (computedStyle)
    {
      *computedStyle = style;
    }
    switch (direction)
    {
      case Direction::Inherit:
        return style ? style->GetWritingMode().ComputedTextDirection() : TextDirection::LTR;
      case Direction::Rtl: return TextDirection::RTL;
      case Direction::Ltr: return TextDirection::LTR;
    }
    krys_debug_assert(false);
    return TextDirection::LTR;
  }

  CanvasDirection CanvasRenderingContext2D::GetDirection() const noexcept
  {
    if (GetState().Direction == Direction::Inherit)
    {
      Canvas().NodeDocument().UpdateStyleIfNeeded();
    }
    return ToTextDirection(GetState().Direction) == TextDirection::RTL ? CanvasDirection::Rtl
                                                                       : CanvasDirection::Ltr;
  }
}
