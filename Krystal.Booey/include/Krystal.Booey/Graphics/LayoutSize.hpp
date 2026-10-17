#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include <algorithm>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  enum class AspectRatioFit
  {
    Shrink,
    Grow
  };

  /// @brief A width and height in layout units.
  class LayoutSize
  {
  private:
    LayoutUnit _width;
    LayoutUnit _height;

  public:
    constexpr LayoutSize() noexcept = default;

    constexpr LayoutSize(const IntSize &size) noexcept : _width(size.Width()), _height(size.Height())
    {
    }

    template <typename T, typename U>
    constexpr LayoutSize(T width, U height) noexcept : _width(width), _height(height)
    {
    }

    constexpr explicit LayoutSize(const FloatSize &size) noexcept
        : _width(size.Width()), _height(size.Height())
    {
    }

    KRYS_NODISCARD constexpr LayoutUnit Width() const noexcept
    {
      return _width;
    }

    KRYS_NODISCARD constexpr LayoutUnit Height() const noexcept
    {
      return _height;
    }

    KRYS_NODISCARD constexpr LayoutUnit MinDimension() const noexcept
    {
      return std::min(_width, _height);
    }

    KRYS_NODISCARD constexpr LayoutUnit MaxDimension() const noexcept
    {
      return std::max(_width, _height);
    }

    template <typename T>
    constexpr void SetWidth(T width) noexcept
    {
      _width = width;
    }

    template <typename T>
    constexpr void SetHeight(T height) noexcept
    {
      _height = height;
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return _width.RawValue() <= 0 || _height.RawValue() <= 0;
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return !_width && !_height;
    }

    KRYS_NODISCARD constexpr float AspectRatio() const noexcept
    {
      return static_cast<float>(_width) / static_cast<float>(_height);
    }

    template <typename T, typename U>
    constexpr void Expand(T width, U height) noexcept
    {
      _width += width;
      _height += height;
    }

    constexpr void Shrink(LayoutUnit width, LayoutUnit height) noexcept
    {
      _width -= width;
      _height -= height;
    }

    constexpr void Scale(float scale) noexcept
    {
      _width *= scale;
      _height *= scale;
    }

    constexpr void Scale(float widthScale, float heightScale) noexcept
    {
      _width *= widthScale;
      _height *= heightScale;
    }

    KRYS_NODISCARD constexpr LayoutSize ConstrainedBetween(const LayoutSize &min,
                                                           const LayoutSize &max) const noexcept
    {
      return {std::max(min.Width(), std::min(max.Width(), _width)),
              std::max(min.Height(), std::min(max.Height(), _height))};
    }

    KRYS_NODISCARD constexpr LayoutSize ExpandedTo(const LayoutSize &other) const noexcept
    {
      return LayoutSize(_width > other._width ? _width : other._width,
                        _height > other._height ? _height : other._height);
    }

    KRYS_NODISCARD constexpr LayoutSize ShrunkTo(const LayoutSize &other) const noexcept
    {
      return LayoutSize(_width < other._width ? _width : other._width,
                        _height < other._height ? _height : other._height);
    }

    constexpr void ClampNegativeToZero() noexcept
    {
      *this = ExpandedTo(LayoutSize());
    }

    constexpr void ClampToMinimumSize(const LayoutSize &minimumSize) noexcept
    {
      if (_width < minimumSize.Width())
      {
        _width = minimumSize.Width();
      }

      if (_height < minimumSize.Height())
      {
        _height = minimumSize.Height();
      }
    }

    KRYS_NODISCARD constexpr LayoutSize TransposedSize() const noexcept
    {
      return LayoutSize(_height, _width);
    }

    constexpr operator FloatSize() const noexcept
    {
      return FloatSize(_width, _height);
    }

    KRYS_NODISCARD constexpr LayoutSize FitToAspectRatio(const LayoutSize &aspectRatio,
                                                         AspectRatioFit fit) const noexcept
    {
      float heightScale = Height().ToFloat() / aspectRatio.Height().ToFloat();
      float widthScale = Width().ToFloat() / aspectRatio.Width().ToFloat();

      if ((widthScale > heightScale) != (fit == AspectRatioFit::Grow))
      {
        return LayoutSize(Height() * aspectRatio.Width() / aspectRatio.Height(), Height());
      }

      return LayoutSize(Width(), Width() * aspectRatio.Height() / aspectRatio.Width());
    }

    KRYS_NODISCARD constexpr bool MightBeSaturated() const noexcept
    {
      return _width.MightBeSaturated() || _height.MightBeSaturated();
    }

    KRYS_NODISCARD friend constexpr bool operator==(const LayoutSize &,
                                                    const LayoutSize &) noexcept = default;
  };

  constexpr LayoutSize &operator+=(LayoutSize &a, const LayoutSize &b) noexcept
  {
    a.SetWidth(a.Width() + b.Width());
    a.SetHeight(a.Height() + b.Height());
    return a;
  }

  constexpr LayoutSize &operator-=(LayoutSize &a, const LayoutSize &b) noexcept
  {
    a.SetWidth(a.Width() - b.Width());
    a.SetHeight(a.Height() - b.Height());
    return a;
  }

  KRYS_NODISCARD constexpr LayoutSize operator+(const LayoutSize &a, const LayoutSize &b) noexcept
  {
    return LayoutSize(a.Width() + b.Width(), a.Height() + b.Height());
  }

  KRYS_NODISCARD constexpr LayoutSize operator-(const LayoutSize &a, const LayoutSize &b) noexcept
  {
    return LayoutSize(a.Width() - b.Width(), a.Height() - b.Height());
  }

  KRYS_NODISCARD constexpr LayoutSize operator-(const LayoutSize &size) noexcept
  {
    return LayoutSize(-size.Width(), -size.Height());
  }

  KRYS_NODISCARD constexpr IntSize FlooredIntSize(const LayoutSize &s) noexcept
  {
    return IntSize(s.Width().Floor(), s.Height().Floor());
  }

  KRYS_NODISCARD constexpr IntSize CeiledIntSize(const LayoutSize &s) noexcept
  {
    return IntSize(s.Width().Ceil(), s.Height().Ceil());
  }

  KRYS_NODISCARD constexpr IntSize RoundedIntSize(const LayoutSize &s) noexcept
  {
    return IntSize(s.Width().Round(), s.Height().Round());
  }

  KRYS_NODISCARD inline FloatSize FloorSizeToDevicePixels(const LayoutSize &size,
                                                          float pixelSnappingFactor) noexcept
  {
    return FloatSize(FloorToDevicePixel(size.Width(), pixelSnappingFactor),
                     FloorToDevicePixel(size.Height(), pixelSnappingFactor));
  }

  KRYS_NODISCARD inline FloatSize RoundSizeToDevicePixels(const LayoutSize &size,
                                                          float pixelSnappingFactor) noexcept
  {
    return FloatSize(RoundToDevicePixel(size.Width(), pixelSnappingFactor),
                     RoundToDevicePixel(size.Height(), pixelSnappingFactor));
  }

  struct LayoutSizeLimits
  {
    LayoutSize Min;
    LayoutSize Max;

    KRYS_NODISCARD constexpr LayoutSize Clamp(const LayoutSize &size) const noexcept
    {
      return size.ConstrainedBetween(Min, Max);
    }

    KRYS_NODISCARD constexpr bool Fits(const LayoutSize &size) const noexcept
    {
      return size == Clamp(size);
    }

    KRYS_NODISCARD constexpr LayoutSize Distance(const LayoutSize &size) const noexcept
    {
      return size - Clamp(size);
    }
  };

  TextStream &operator<<(TextStream &ts, const LayoutSize &value) noexcept;
}
