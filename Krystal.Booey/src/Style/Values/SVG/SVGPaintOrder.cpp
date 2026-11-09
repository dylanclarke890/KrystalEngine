/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/SVG/SVGPaintOrder.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  std::span<const PaintType, 3> SVGPaintOrder::PaintTypes() const noexcept
  {
    static constexpr std::array fillStrokeMarkers {PaintType::Fill, PaintType::Stroke, PaintType::Markers};
    static constexpr std::array fillMarkersStroke {PaintType::Fill, PaintType::Markers, PaintType::Stroke};
    static constexpr std::array strokeFillMarkers {PaintType::Stroke, PaintType::Fill, PaintType::Markers};
    static constexpr std::array strokeMarkersFill {PaintType::Stroke, PaintType::Markers, PaintType::Fill};
    static constexpr std::array markersFillStroke {PaintType::Markers, PaintType::Fill, PaintType::Stroke};
    static constexpr std::array markersStrokeFill {PaintType::Markers, PaintType::Stroke, PaintType::Fill};

    switch (_type)
    {
      case Type::Normal:
      case Type::FillStrokeMarkers: return fillStrokeMarkers;
      case Type::FillMarkersStroke: return fillMarkersStroke;
      case Type::StrokeFillMarkers: return strokeFillMarkers;
      case Type::StrokeMarkersFill: return strokeMarkersFill;
      case Type::MarkersFillStroke: return markersFillStroke;
      case Type::MarkersStrokeFill: return markersStrokeFill;
    };
    krys_unreachable();
  }

  // MARK: - Conversion

  SVGPaintOrder CSSValueConversion<SVGPaintOrder>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal:  return css::keywords::Normal {};
        case css::ValueId::Fill:    return css::keywords::Fill {};
        case css::ValueId::Stroke:  return css::keywords::Stroke {};
        case css::ValueId::Markers: return css::keywords::Markers {};
        default:                    state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue, 1>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    switch (const auto &first = list->item(0); first.ValueId())
    {
      case css::ValueId::Fill:
        if (list->size() > 1)
        {
          switch (const auto &second = list->item(1); second.ValueId())
          {
            case css::ValueId::Markers: return {css::keywords::Fill {}, css::keywords::Markers {}};
            default:                    state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
          }
        }
        return css::keywords::Fill {};
      case css::ValueId::Stroke:
        if (list->size() > 1)
        {
          switch (const auto &second = list->item(1); second.ValueId())
          {
            case css::ValueId::Markers: return {css::keywords::Stroke {}, css::keywords::Markers {}};
            default:                    state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
          }
        }
        return css::keywords::Stroke {};
      case css::ValueId::Markers:
        if (list->size() > 1)
        {
          switch (const auto &second = list->item(1); second.ValueId())
          {
            case css::ValueId::Stroke: return {css::keywords::Markers {}, css::keywords::Stroke {}};
            default:                   state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
          }
        }
        return css::keywords::Markers {};
      default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
    }
  }
}
