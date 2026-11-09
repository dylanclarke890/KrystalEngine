/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::css
{
  class ToLengthConversionData;

  enum class PropertyId : uint16;

  struct ComputedStyleDependencies
  {
    List<PropertyId> Properties;
    List<PropertyId> RootProperties;
    bool ContainerDimensions {false};
    bool ViewportDimensions {false};
    bool Anchors {false};

    KRYS_NODISCARD bool IsComputationallyIndependent() const noexcept
    {
      return Properties.empty() && RootProperties.empty() && !ContainerDimensions && !Anchors;
    }

    /// @brief Checks to see if the provided conversion data is sufficient to resolve the provided
    /// dependencies.
    KRYS_NODISCARD bool
      CanResolveDependenciesWithConversionData(const ToLengthConversionData &conversionData) const noexcept;
  };
}
