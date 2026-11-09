/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridLayout.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridTypes.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/PlacedGridItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/UnplacedGridItem.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationUtils.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo::layout
{
  class BoxGeometry;
  class ElementBox;
  class LayoutState;

  /// @brief Lays out the in-flow children of a grid container.
  class GridFormattingContext
  {
    KRYS_NON_COPY_MOVABLE(GridFormattingContext);

  private:
    const ElementBox &_gridBox;
    LayoutState &_globalLayoutState;
    const IntegrationUtils _integrationUtils;

  public:
    GridFormattingContext(const ElementBox &gridBox, LayoutState &layoutState) noexcept;

    void Layout(GridLayoutConstraints layoutConstraints) noexcept;

    KRYS_NODISCARD PlacedGridItems ConstructPlacedGridItems(const GridAreas &gridAreas) const noexcept;

    KRYS_NODISCARD const ElementBox &Root() const noexcept
    {
      return _gridBox;
    }
    KRYS_NODISCARD const IntegrationUtils &GetIntegrationUtils() const noexcept
    {
      return _integrationUtils;
    }
    KRYS_NODISCARD const BoxGeometry &GeometryForGridItem(const ElementBox &layoutBox) const noexcept;

  private:
    KRYS_NODISCARD UnplacedGridItems ConstructUnplacedGridItems() const noexcept;
    KRYS_NODISCARD const LayoutState &GetLayoutState() const noexcept
    {
      return _globalLayoutState;
    }
    KRYS_NODISCARD BoxGeometry &GeometryForGridItem(const ElementBox &layoutBox) noexcept;
    void SetGridItemGeometries(const GridItemRects &gridItemRects) noexcept;
  };
}
