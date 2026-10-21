#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief How a filter primitive reads past its input's edges.
  enum class EdgeModeType : uint8
  {
    Unknown,
    Duplicate,
    Wrap,
    None
  };

  /// @brief The Gaussian blur filter effect.
  class FEGaussianBlur final : public FilterEffect
  {
  private:
    float _stdX;
    float _stdY;
    EdgeModeType _edgeMode;

  public:
    KRYS_NODISCARD static Ref<FEGaussianBlur> Create(float x, float y, EdgeModeType edgeMode) noexcept;

    KRYS_NODISCARD bool operator==(const FEGaussianBlur &other) const noexcept;

    KRYS_NODISCARD float StdDeviationX() const noexcept
    {
      return _stdX;
    }
    bool SetStdDeviationX(float stdX) noexcept;

    KRYS_NODISCARD float StdDeviationY() const noexcept
    {
      return _stdY;
    }
    bool SetStdDeviationY(float stdY) noexcept;

    KRYS_NODISCARD EdgeModeType EdgeMode() const noexcept
    {
      return _edgeMode;
    }
    bool SetEdgeMode(EdgeModeType edgeMode) noexcept;

    KRYS_NODISCARD static IntSize CalculateKernelSize(const Filter &filter, FloatSize stdDeviation) noexcept;
    /// @brief The size of the box blur kernels that approximate a Gaussian blur of `stdDeviation`.
    KRYS_NODISCARD static IntSize CalculateUnscaledKernelSize(FloatSize stdDeviation) noexcept;
    /// @brief How far a blur of `stdDeviation` spreads past each edge.
    KRYS_NODISCARD static IntSize CalculateOutsetSize(FloatSize stdDeviation) noexcept;

    KRYS_NODISCARD static IntOutsets CalculateOutsets(const FloatSize &stdDeviation) noexcept;

  private:
    FEGaussianBlur(float x, float y, EdgeModeType edgeMode) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEGaussianBlur>(*this, other);
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    KRYS_NODISCARD EnumSet<FilterRenderingMode>
      SupportedFilterRenderingModes(EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept override;

    KRYS_NODISCARD Maybe<GraphicsStyle> CreateGraphicsStyle(GraphicsContext &context,
                                                            const Filter &filter) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts, FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEGaussianBlur)
