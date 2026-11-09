/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/Fonts/FontSelectionAlgorithm.hpp"
#include <memory>
namespace krys::boo
{
  FontSelectionAlgorithm::FontSelectionAlgorithm(FontSelectionRequest request,
                                                 const List<Capabilities> &capabilities,
                                                 Maybe<Capabilities> bounds) noexcept
      : _request(request), _capabilities(capabilities)
  {
    krys_debug_assert(!_capabilities.empty());
    if (bounds)
    {
      _capabilitiesBounds = bounds.value();
    }
    else
    {
      for (const auto &item : _capabilities)
      {
        _capabilitiesBounds.Expand(item);
      }
    }
  }

  auto FontSelectionAlgorithm::WidthDistance(Capabilities capabilities) const noexcept -> DistanceResult
  {
    auto width = capabilities.Width;
    krys_debug_assert(width.IsValid());
    if (width.Includes(_request.Width))
    {
      return {FontSelectionValue(), _request.Width};
    }

    if (_request.Width > NormalWidthValue())
    {
      if (width.Minimum > _request.Width)
      {
        return {width.Minimum - _request.Width, width.Minimum};
      }
      krys_debug_assert(width.Maximum < _request.Width);
      auto threshold = std::max(_request.Width, _capabilitiesBounds.Width.Maximum);
      return {threshold - width.Maximum, width.Maximum};
    }

    if (width.Maximum < _request.Width)
    {
      return {_request.Width - width.Maximum, width.Maximum};
    }
    krys_debug_assert(width.Minimum > _request.Width);
    auto threshold = std::min(_request.Width, _capabilitiesBounds.Width.Minimum);
    return {width.Minimum - threshold, width.Minimum};
  }

  auto FontSelectionAlgorithm::StyleDistance(Capabilities capabilities) const noexcept -> DistanceResult
  {
    auto slope = capabilities.Slope;
    auto requestSlope = _request.Slope.value_or(NormalItalicValue());
    krys_debug_assert(slope.IsValid());
    if (slope.Includes(requestSlope))
    {
      return {FontSelectionValue(), requestSlope};
    }

    if (requestSlope >= ItalicThreshold())
    {
      if (slope.Minimum > requestSlope)
      {
        return {slope.Minimum - requestSlope, slope.Minimum};
      }
      krys_debug_assert(requestSlope > slope.Maximum);
      auto threshold = std::max(requestSlope, _capabilitiesBounds.Slope.Maximum);
      return {threshold - slope.Maximum, slope.Maximum};
    }

    if (requestSlope >= FontSelectionValue())
    {
      if (slope.Maximum >= FontSelectionValue() && slope.Maximum < requestSlope)
      {
        return {requestSlope - slope.Maximum, slope.Maximum};
      }
      if (slope.Minimum > requestSlope)
      {
        return {slope.Minimum, slope.Minimum};
      }
      krys_debug_assert(slope.Maximum < FontSelectionValue());
      auto threshold = std::max(requestSlope, _capabilitiesBounds.Slope.Maximum);
      return {threshold - slope.Maximum, slope.Maximum};
    }

    if (requestSlope > -ItalicThreshold())
    {
      if (slope.Minimum > requestSlope && slope.Minimum <= FontSelectionValue())
      {
        return {slope.Minimum - requestSlope, slope.Minimum};
      }
      if (slope.Maximum < requestSlope)
      {
        return {-slope.Maximum, slope.Maximum};
      }
      krys_debug_assert(slope.Minimum > FontSelectionValue());
      auto threshold = std::min(requestSlope, _capabilitiesBounds.Slope.Minimum);
      return {slope.Minimum - threshold, slope.Minimum};
    }

    if (slope.Maximum < requestSlope)
    {
      return {requestSlope - slope.Maximum, slope.Maximum};
    }
    krys_debug_assert(slope.Minimum > requestSlope);
    auto threshold = std::min(requestSlope, _capabilitiesBounds.Slope.Minimum);
    return {slope.Minimum - threshold, slope.Minimum};
  }

  auto FontSelectionAlgorithm::WeightDistance(Capabilities capabilities) const noexcept -> DistanceResult
  {
    auto weight = capabilities.Weight;
    krys_debug_assert(weight.IsValid());
    if (weight.Includes(_request.Weight))
    {
      return {FontSelectionValue(), _request.Weight};
    }

    if (_request.Weight >= LowerWeightSearchThreshold() && _request.Weight <= UpperWeightSearchThreshold())
    {
      if (weight.Minimum > _request.Weight && weight.Minimum <= UpperWeightSearchThreshold())
      {
        return {weight.Minimum - _request.Weight, weight.Minimum};
      }
      if (weight.Maximum < _request.Weight)
      {
        return {UpperWeightSearchThreshold() - weight.Maximum, weight.Maximum};
      }
      krys_debug_assert(weight.Minimum > UpperWeightSearchThreshold());
      auto threshold = std::min(_request.Weight, _capabilitiesBounds.Weight.Minimum);
      return {weight.Minimum - threshold, weight.Minimum};
    }
    if (_request.Weight < LowerWeightSearchThreshold())
    {
      if (weight.Maximum < _request.Weight)
      {
        return {_request.Weight - weight.Maximum, weight.Maximum};
      }
      krys_debug_assert(weight.Minimum > _request.Weight);
      auto threshold = std::min(_request.Weight, _capabilitiesBounds.Weight.Minimum);
      return {weight.Minimum - threshold, weight.Minimum};
    }
    krys_debug_assert(_request.Weight >= UpperWeightSearchThreshold());
    if (weight.Minimum > _request.Weight)
    {
      return {weight.Minimum - _request.Weight, weight.Minimum};
    }
    krys_debug_assert(weight.Maximum < _request.Weight);
    auto threshold = std::max(_request.Weight, _capabilitiesBounds.Weight.Maximum);
    return {threshold - weight.Maximum, weight.Maximum};
  }

  FontSelectionValue FontSelectionAlgorithm::BestValue(Span<const bool> eliminated,
                                                       DistanceFunction computeDistance) const noexcept
  {
    Maybe<DistanceResult> smallestDistance;
    for (size_t i = 0, size = _capabilities.size(); i < size; ++i)
    {
      if (eliminated[i])
      {
        continue;
      }
      auto distanceResult = (this->*computeDistance)(_capabilities[i]);
      if (!smallestDistance || distanceResult.Distance < smallestDistance.value().Distance)
      {
        smallestDistance = distanceResult;
      }
    }
    return smallestDistance.value().Value;
  }

  void FontSelectionAlgorithm::FilterCapability(Span<bool> eliminated, DistanceFunction computeDistance,
                                                CapabilitiesRange inclusionRange) noexcept
  {
    auto value = BestValue(eliminated, computeDistance);
    for (size_t i = 0, size = _capabilities.size(); i < size; ++i)
    {
      eliminated[i] = eliminated[i] || !(_capabilities[i].*inclusionRange).Includes(value);
    }
  }

  size_t FontSelectionAlgorithm::IndexOfBestCapabilities() noexcept
  {
    // `List<bool>` packs its bits, so the flags live in a plain array.
    auto eliminated = std::make_unique<bool[]>(_capabilities.size());
    Span<bool> flags {eliminated.get(), _capabilities.size()};
    FilterCapability(flags, &FontSelectionAlgorithm::WidthDistance, &Capabilities::Width);
    FilterCapability(flags, &FontSelectionAlgorithm::StyleDistance, &Capabilities::Slope);
    FilterCapability(flags, &FontSelectionAlgorithm::WeightDistance, &Capabilities::Weight);
    return static_cast<size_t>(std::find(flags.begin(), flags.end(), false) - flags.begin());
  }
}
