/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2021-2023 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  enum class ComponentTransferType : uint8
  {
    FECOMPONENTTRANSFER_TYPE_UNKNOWN = 0,
    FECOMPONENTTRANSFER_TYPE_IDENTITY = 1,
    FECOMPONENTTRANSFER_TYPE_TABLE = 2,
    FECOMPONENTTRANSFER_TYPE_DISCRETE = 3,
    FECOMPONENTTRANSFER_TYPE_LINEAR = 4,
    FECOMPONENTTRANSFER_TYPE_GAMMA = 5
  };

  /// @brief How one channel is transferred.
  struct ComponentTransferFunction
  {
    ComponentTransferType Type {ComponentTransferType::FECOMPONENTTRANSFER_TYPE_UNKNOWN};

    // For linear
    float Slope {1};
    float Intercept {0};

    // For gamma
    float Amplitude {1};
    float Exponent {1};
    float Offset {0};

    List<float> TableValues;

    KRYS_NODISCARD bool operator==(const ComponentTransferFunction &) const noexcept = default;
  };

  enum class ComponentTransferChannel : uint8
  {
    Red,
    Green,
    Blue,
    Alpha
  };

  /// @brief The functions of the red, green, blue and alpha channels.
  using ComponentTransferFunctions = Array<ComponentTransferFunction, 4>;

  /// @brief The component transfer filter effect: a function applied to each straight-alpha channel.
  class FEComponentTransfer final : public FilterEffect
  {
  public:
    using LookupTable = Array<uint8, 256>;

  private:
    ComponentTransferFunctions _functions;

  public:
    KRYS_NODISCARD static Ref<FEComponentTransfer>
      Create(const ComponentTransferFunction &redFunction, const ComponentTransferFunction &greenFunction,
             const ComponentTransferFunction &blueFunction, const ComponentTransferFunction &alphaFunction,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;
    KRYS_NODISCARD static Ref<FEComponentTransfer> Create(ComponentTransferFunctions &&functions) noexcept;

    KRYS_NODISCARD static LookupTable ComputeLookupTable(const ComponentTransferFunction &function) noexcept;

    KRYS_NODISCARD bool operator==(const FEComponentTransfer &other) const noexcept;

    KRYS_NODISCARD ComponentTransferFunction RedFunction() const noexcept
    {
      return FunctionOf(ComponentTransferChannel::Red);
    }
    KRYS_NODISCARD ComponentTransferFunction GreenFunction() const noexcept
    {
      return FunctionOf(ComponentTransferChannel::Green);
    }
    KRYS_NODISCARD ComponentTransferFunction BlueFunction() const noexcept
    {
      return FunctionOf(ComponentTransferChannel::Blue);
    }
    KRYS_NODISCARD ComponentTransferFunction AlphaFunction() const noexcept
    {
      return FunctionOf(ComponentTransferChannel::Alpha);
    }

    bool SetType(ComponentTransferChannel channel, ComponentTransferType type) noexcept;
    bool SetSlope(ComponentTransferChannel channel, float value) noexcept;
    bool SetIntercept(ComponentTransferChannel channel, float value) noexcept;
    bool SetAmplitude(ComponentTransferChannel channel, float value) noexcept;
    bool SetExponent(ComponentTransferChannel channel, float value) noexcept;
    bool SetOffset(ComponentTransferChannel channel, float value) noexcept;
    bool SetTableValues(ComponentTransferChannel channel, List<float> &&values) noexcept;

  private:
    FEComponentTransfer(const ComponentTransferFunction &redFunction,
                        const ComponentTransferFunction &greenFunction,
                        const ComponentTransferFunction &blueFunction,
                        const ComponentTransferFunction &alphaFunction,
                        const DestinationColorSpace &colorSpace) noexcept;
    explicit FEComponentTransfer(ComponentTransferFunctions &&functions) noexcept;

    KRYS_NODISCARD const ComponentTransferFunction &
      FunctionOf(ComponentTransferChannel channel) const noexcept
    {
      return _functions[static_cast<size_t>(channel)];
    }
    KRYS_NODISCARD ComponentTransferFunction &FunctionOf(ComponentTransferChannel channel) noexcept
    {
      return _functions[static_cast<size_t>(channel)];
    }

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEComponentTransfer>(*this, other);
    }

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEComponentTransfer)
