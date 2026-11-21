/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Cameron McCormack <cam@mcc.id.au>
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
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

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Animation/SVGSMILElement.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAttributeAnimator.hpp"
#include "Krystal.Booey/SVG/SVGTests.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Maths/UnitBezier.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::svg
{
  // If we have 'currentColor' or 'inherit' as animation value, we need to grab
  // the value during the animation since the value can be animated itself.
  enum AnimatedPropertyValueType : uint8
  {
    RegularPropertyValue,
    CurrentColorValue,
    InheritValue,
  };
}

namespace krys::boo
{
  /// @brief An animation element: a SMIL timed element that interpolates between values over its interval.
  /// @see https://svgwg.org/specs/animations/#InterfaceSVGAnimationElement
  class SVGAnimationElement : public SVGSMILElement, public SVGTests
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGAnimationElement);

  public:
    enum class AttributeType : uint8
    {
      CSS,
      XML,
      Auto,
    };

  protected:
    svg::AnimatedPropertyValueType _fromPropertyValueType {svg::RegularPropertyValue};
    svg::AnimatedPropertyValueType _toPropertyValueType {svg::RegularPropertyValue};

  private:
    bool _animationValid {false};

    AttributeType _attributeType {AttributeType::Auto};
    List<DOMString> _values;
    List<float> _keyTimesFromAttribute;
    List<float> _keyTimesForPaced;
    List<float> _keyPoints;
    List<UnitBezier> _keySplines;
    Maybe<DOMString> _lastValuesAnimationFrom;
    Maybe<DOMString> _lastValuesAnimationTo;
    svg::CalcMode _calcMode {svg::CalcMode::Linear};
    svg::AnimationMode _animationMode {svg::AnimationMode::None};

  public:
#pragma region DOM API

    /// @see https://svgwg.org/specs/animations/#__svg__SVGAnimationElement__getStartTime
    KRYS_NODISCARD ExceptionOr<float> GetStartTime() const noexcept;

    /// @see https://svgwg.org/specs/animations/#__svg__SVGAnimationElement__getCurrentTime
    KRYS_NODISCARD float GetCurrentTime() const noexcept;

    /// @see https://svgwg.org/specs/animations/#__svg__SVGAnimationElement__getSimpleDuration
    KRYS_NODISCARD ExceptionOr<float> GetSimpleDuration() const noexcept;

    /// @see https://svgwg.org/specs/animations/#__svg__SVGAnimationElement__beginElement
    void BeginElement() noexcept
    {
      BeginElementAt(0);
    }

    /// @see https://svgwg.org/specs/animations/#__svg__SVGAnimationElement__beginElementAt
    void BeginElementAt(float offset) noexcept;

    /// @see https://svgwg.org/specs/animations/#__svg__SVGAnimationElement__endElement
    void EndElement() noexcept
    {
      EndElementAt(0);
    }

    /// @see https://svgwg.org/specs/animations/#__svg__SVGAnimationElement__endElementAt
    void EndElementAt(float offset) noexcept;

    /// @see https://svgwg.org/specs/animations/#InterfaceSVGAnimationElement
    KRYS_DECLARE_EVENT_HANDLER(Begin, u8"beginEvent")
    /// @see https://svgwg.org/specs/animations/#InterfaceSVGAnimationElement
    KRYS_DECLARE_EVENT_HANDLER(End, u8"endEvent")
    /// @see https://svgwg.org/specs/animations/#InterfaceSVGAnimationElement
    KRYS_DECLARE_EVENT_HANDLER(Repeat, u8"repeatEvent")

#pragma endregion

    KRYS_NODISCARD static bool IsTargetAttributeCSSProperty(SVGElement *, const QualifiedName &) noexcept;

    KRYS_NODISCARD bool IsAdditive() const noexcept override;

    KRYS_NODISCARD bool IsAccumulated() const noexcept;

    KRYS_NODISCARD svg::AnimationMode GetAnimationMode() const noexcept
    {
      return _animationMode;
    }

    KRYS_NODISCARD svg::CalcMode GetCalcMode() const noexcept
    {
      return _calcMode;
    }

    KRYS_NODISCARD svg::AnimatedPropertyValueType FromPropertyValueType() const noexcept
    {
      return _fromPropertyValueType;
    }

    KRYS_NODISCARD svg::AnimatedPropertyValueType ToPropertyValueType() const noexcept
    {
      return _toPropertyValueType;
    }

    void AnimateAdditiveNumber(float percentage, uint32 repeatCount, float fromNumber, float toNumber,
                               float toAtEndOfDurationNumber, float &animatedNumber) const noexcept
    {
      float number;
      if (GetCalcMode() == svg::CalcMode::Discrete)
      {
        number = percentage < 0.5 ? fromNumber : toNumber;
      }
      else
      {
        number = (toNumber - fromNumber) * percentage + fromNumber;
      }

      if (IsAccumulated() && repeatCount)
      {
        number += toAtEndOfDurationNumber * static_cast<float>(repeatCount);
      }

      if (IsAdditive() && GetAnimationMode() != svg::AnimationMode::To)
      {
        animatedNumber += number;
      }
      else
      {
        animatedNumber = number;
      }
    }

    KRYS_NODISCARD AttributeType GetAttributeType() const noexcept
    {
      return _attributeType;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGAnimationElement, SVGElement, SVGTests>;

  protected:
    SVGAnimationElement(Document &, SVGElementInterface) noexcept;

    virtual void ResetAnimation() noexcept;

    KRYS_NODISCARD static bool IsSupportedAttribute(const QualifiedName &) noexcept;

    void AttributeChanged(const QualifiedName &, Maybe<DOMStringView> oldValue,
                          Maybe<DOMStringView> value) noexcept override;

    void SVGAttributeChanged(const QualifiedName &) noexcept override;

    KRYS_NODISCARD DOMString ToValue() const noexcept;

    KRYS_NODISCARD DOMString ByValue() const noexcept;

    KRYS_NODISCARD DOMString FromValue() const noexcept;

    // from SVGSMILElement
    void StartedActiveInterval() noexcept override;

    void UpdateAnimation(float percent, uint32 repeat) noexcept override;

    void SetAttributeName(const QualifiedName &) noexcept override
    {
    }

    virtual void UpdateAnimationMode() noexcept;

    void SetAnimationMode(svg::AnimationMode animationMode) noexcept
    {
      _animationMode = animationMode;
    }

    void SetCalcMode(svg::CalcMode calcMode) noexcept
    {
      _calcMode = calcMode;
    }

  private:
    void AnimationAttributeChanged() noexcept override;

    void SetAttributeType(DOMStringView) noexcept;

    KRYS_NODISCARD bool IsSVGAnimationElement() const noexcept final
    {
      return true;
    }

    virtual bool SetFromAndToValues(DOMStringView fromString, DOMStringView toString) noexcept = 0;

    virtual bool SetFromAndByValues(DOMStringView fromString, DOMStringView byString) noexcept = 0;

    virtual bool SetToAtEndOfDurationValue(DOMStringView toAtEndOfDurationString) noexcept = 0;

    virtual void CalculateAnimatedValue(float percent, uint32 repeatCount) noexcept = 0;

    KRYS_NODISCARD virtual Maybe<float> CalculateDistance(DOMStringView fromString,
                                                          DOMStringView toString) noexcept = 0;

    KRYS_NODISCARD const List<float> &KeyTimes() const noexcept;

    void CurrentValuesForValuesAnimation(float percent, float &effectivePercent, DOMString &from,
                                         DOMString &to) noexcept;

    void CalculateKeyTimesForCalcModePaced() noexcept;

    KRYS_NODISCARD float CalculatePercentFromKeyPoints(float percent) const noexcept;

    void CurrentValuesFromKeyPoints(float percent, float &effectivePercent, DOMString &from,
                                    DOMString &to) const noexcept;

    KRYS_NODISCARD float CalculatePercentForSpline(float percent, uint32 splineIndex) const noexcept;

    KRYS_NODISCARD float CalculatePercentForFromTo(float percent) const noexcept;

    KRYS_NODISCARD uint32 CalculateKeyTimesIndex(float percent) const noexcept;

    void SetCalcMode(DOMStringView) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SVGAnimationElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::SVGElement &element) noexcept
  {
    return element.IsSVGAnimationElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
