/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005-2008, 2017 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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
 *
 */

#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  class IdentityTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<IdentityTransformOperation> Create() noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return Create();
    }

  private:
    KRYS_NODISCARD bool Equals(const TransformOperation &o) const noexcept override
    {
      return IsSameType(o);
    }

    void Apply(TransformationMatrix &) const noexcept override
    {
    }

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *, const BlendingContext &,
                                                 bool = false) const noexcept override
    {
      return ShareRef(const_cast<IdentityTransformOperation &>(*this));
    }

    IdentityTransformOperation() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::IdentityTransformOperation,
                                                krys::boo::TransformOperation::Type::Identity ==)
