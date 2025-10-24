#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/NullableFloat.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI.Styles/Enums/Dimension.hpp"
#include "Krystal.UI.Styles/Enums/Direction.hpp"
#include "Krystal.UI.Styles/Enums/Edge.hpp"
#include "Krystal.UI.Styles/Enums/PhysicalEdge.hpp"
#include "Krystal.UI.Layout/Node/CachedMeasurement.hpp"
#include "Krystal.UI.Layout/Api/Forward.hpp"

namespace Krys::UI::Layout
{
  struct LayoutResults
  {
  private:
    Styles::Direction _direction : BitCount<Styles::Direction>() = Styles::Direction::Inherit;
    bool _hadOverflow : 1 = false;

  public:
    Styles::Direction LastOwnerDirection : BitCount<Styles::Direction>() = Styles::Direction::Inherit;
    uint8 NextCachedMeasurementsIndex = 0;
    uint16 GenerationCount = 0;
    uint16 ConfigVersion = 0;
    uint16 ComputedFlexBasisGeneration = 0;

  private:
    Array<float, 2> _dimensions = {{Undefined, Undefined}};
    Array<float, 2> _measuredDimensions = {{Undefined, Undefined}};
    Array<float, 2> _rawDimensions = {{Undefined, Undefined}};
    Array<float, 4> _position = {};
    Array<float, 4> _margin = {};
    Array<float, 4> _border = {};
    Array<float, 4> _padding = {};

  public:
    // This value was chosen based on empirical data:
    // 98% of analyzed layouts require less than 8 entries.
    static constexpr int32 MaxCachedMeasurements = 8;

    NullableFloat ComputedFlexBasis = {};
    // Instead of recomputing the entire layout every single time, we cache some
    // information to break early when nothing changed

    Array<CachedMeasurement, MaxCachedMeasurements> CachedMeasurements = {};
    CachedMeasurement CachedLayout {};

    Styles::Direction GetDirection() const
    {
      return _direction;
    }

    void SetDirection(Styles::Direction direction)
    {
      _direction = direction;
    }

    bool GetHadOverflow() const
    {
      return _hadOverflow;
    }

    void SetHadOverflow(bool hadOverflow)
    {
      _hadOverflow = hadOverflow;
    }

    float GetDimension(Styles::Dimension axis) const
    {
      return _dimensions[ToUnderlying(axis)];
    }

    void SetDimension(Styles::Dimension axis, float dimension)
    {
      _dimensions[ToUnderlying(axis)] = dimension;
    }

    float GetMeasuredDimension(Styles::Dimension axis) const
    {
      return _measuredDimensions[ToUnderlying(axis)];
    }

    void SetMeasuredDimension(Styles::Dimension axis, float dimension)
    {
      _measuredDimensions[ToUnderlying(axis)] = dimension;
    }

    float GetRawDimension(Styles::Dimension axis) const
    {
      return _rawDimensions[ToUnderlying(axis)];
    }

    void SetRawDimension(Styles::Dimension axis, float dimension)
    {
      _rawDimensions[ToUnderlying(axis)] = dimension;
    }

    float GetPosition(Styles::PhysicalEdge physicalEdge) const
    {
      return _position[ToUnderlying(physicalEdge)];
    }

    void SetPosition(Styles::PhysicalEdge physicalEdge, float dimension)
    {
      _position[ToUnderlying(physicalEdge)] = dimension;
    }

    float GetMargin(Styles::PhysicalEdge physicalEdge) const
    {
      return _margin[ToUnderlying(physicalEdge)];
    }

    void SetMargin(Styles::PhysicalEdge physicalEdge, float dimension)
    {
      _margin[ToUnderlying(physicalEdge)] = dimension;
    }

    float GetBorder(Styles::PhysicalEdge physicalEdge) const
    {
      return _border[ToUnderlying(physicalEdge)];
    }

    void SetBorder(Styles::PhysicalEdge physicalEdge, float dimension)
    {
      _border[ToUnderlying(physicalEdge)] = dimension;
    }

    float GetPadding(Styles::PhysicalEdge physicalEdge) const
    {
      return _padding[ToUnderlying(physicalEdge)];
    }

    void SetPadding(Styles::PhysicalEdge physicalEdge, float dimension)
    {
      _padding[ToUnderlying(physicalEdge)] = dimension;
    }

    bool operator==(LayoutResults layout) const;

    bool operator!=(LayoutResults layout) const
    {
      return !(*this == layout);
    }
  };
}