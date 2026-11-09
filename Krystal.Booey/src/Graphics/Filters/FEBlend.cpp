/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FEBlend.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<FEBlend> FEBlend::Create(BlendMode mode, const DestinationColorSpace &colorSpace) noexcept
  {
    return AdoptRef(*new FEBlend(mode, colorSpace));
  }

  FEBlend::FEBlend(BlendMode mode, const DestinationColorSpace &colorSpace) noexcept
      : FilterEffect(FilterEffect::Type::FEBlend, colorSpace), _mode(mode)
  {
  }

  bool FEBlend::operator==(const FEBlend &other) const noexcept
  {
    return FilterEffect::operator==(other) && _mode == other._mode;
  }

  bool FEBlend::SetBlendMode(BlendMode mode) noexcept
  {
    if (_mode == mode)
    {
      return false;
    }
    _mode = mode;
    return true;
  }

  TextStream &FEBlend::ExternalRepresentation(TextStream &ts,
                                              FilterRepresentation representation) const noexcept
  {
    ts << Indent << u8"[feBlend";
    FilterEffect::ExternalRepresentation(ts, representation);
    ts << u8" mode=\"";
    if (_mode == BlendMode::Normal)
    {
      ts << u8"normal";
    }
    else
    {
      ts << CompositeOperatorName(CompositeOperator::SourceOver, _mode);
    }
    ts << u8"\"]\n";
    return ts;
  }
}
