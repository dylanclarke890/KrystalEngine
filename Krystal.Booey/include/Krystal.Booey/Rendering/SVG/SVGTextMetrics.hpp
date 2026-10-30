#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <limits>

namespace krys::boo
{
  class RenderSVGInlineText;
  class TextRun;

  /// @brief The advance and height of a character of SVG text, in user space.
  class SVGTextMetrics
  {
  private:
    float _width {0};
    float _height {0};
    /// @brief In bytes.
    uint32 _length {0};

  public:
    enum class MetricsType : uint8
    {
      SkippedSpaceMetrics
    };

    SVGTextMetrics() noexcept = default;
    explicit SVGTextMetrics(MetricsType) noexcept : _length(1)
    {
    }
    SVGTextMetrics(const RenderSVGInlineText &, uint32 length, float width) noexcept;
    SVGTextMetrics(uint32 length, float scaledWidth, float scaledHeight) noexcept
        : _width(scaledWidth), _height(scaledHeight), _length(length)
    {
    }

    KRYS_NODISCARD static SVGTextMetrics MeasureCharacterRange(const RenderSVGInlineText &, uint32 position,
                                                               uint32 length) noexcept;
    KRYS_NODISCARD static TextRun
      ConstructTextRun(const RenderSVGInlineText &, uint32 position = 0,
                       uint32 length = std::numeric_limits<uint32>::max()) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return !_width && !_height && _length == 1;
    }

    KRYS_NODISCARD float Width() const noexcept
    {
      return _width;
    }
    void SetWidth(float width) noexcept
    {
      _width = width;
    }

    KRYS_NODISCARD float Height() const noexcept
    {
      return _height;
    }
    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return _length;
    }

  private:
    SVGTextMetrics(const RenderSVGInlineText &, const TextRun &) noexcept;
  };
}
