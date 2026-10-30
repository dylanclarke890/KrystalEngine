#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::svg
{
  class SVGElement;

  enum class CTMScope : bool
  {
    NearestViewportScope, // Used for getCTM()
    ScreenScope           // Used for getScreenCTM()
  };

  /// @brief The elements with a place in a coordinate system: their viewport elements, bounding box and
  /// transforms to their viewport and to the screen.
  class SVGLocatable
  {
  public:
    virtual ~SVGLocatable() noexcept = default;

    // 'SVGLocatable' functions
    KRYS_NODISCARD virtual SVGElement *NearestViewportElement() const noexcept = 0;
    KRYS_NODISCARD virtual SVGElement *FarthestViewportElement() const noexcept = 0;

    enum StyleUpdateStrategy : uint8
    {
      AllowStyleUpdate,
      DisallowStyleUpdate
    };

    KRYS_NODISCARD virtual FloatRect GetBBox(StyleUpdateStrategy) noexcept = 0;
    KRYS_NODISCARD virtual AffineTransform GetCTM(StyleUpdateStrategy) noexcept = 0;
    KRYS_NODISCARD virtual AffineTransform GetScreenCTM(StyleUpdateStrategy) noexcept = 0;

    KRYS_NODISCARD static SVGElement *NearestViewportElement(const SVGElement *element) noexcept;
    KRYS_NODISCARD static SVGElement *FarthestViewportElement(const SVGElement *element) noexcept;

  protected:
    KRYS_NODISCARD virtual AffineTransform LocalCoordinateSpaceTransform(CTMScope) const noexcept
    {
      return AffineTransform();
    }

    KRYS_NODISCARD static FloatRect GetBBox(SVGElement *element, StyleUpdateStrategy) noexcept;
    KRYS_NODISCARD static AffineTransform ComputeCTM(SVGElement *element, CTMScope,
                                                     StyleUpdateStrategy) noexcept;
  };
}
