/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2021-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FEMerge.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEMerge> FEMerge::Create(uint32 numberOfEffectInputs, const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEMerge(numberOfEffectInputs, colorSpace));
  }

  FEMerge::FEMerge(uint32 numberOfEffectInputs, const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEMerge, colorSpace), _numberOfEffectInputs(numberOfEffectInputs)
  {
  }

  bool FEMerge::operator==(const FEMerge &other) const noexcept
  {
    return FilterEffect::operator==(other) && _numberOfEffectInputs == other._numberOfEffectInputs;
  }

  TextStream &FEMerge::ExternalRepresentation(TextStream &ts,
                                              FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feMerge";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8" mergeNodes=\"" << _numberOfEffectInputs << u8'"';
    ts << u8"]\n";
    return ts;
  }
}
