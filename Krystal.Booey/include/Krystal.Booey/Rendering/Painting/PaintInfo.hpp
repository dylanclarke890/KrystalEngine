/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2004 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/Painting/PaintPhase.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"

namespace krys::boo
{
  class RenderLayer;
  class RenderInline;
  class RenderLayerModelObject;
  class RenderObject;

  /*
   * Paint the object and its children, clipped by (x|y|w|h).
   * (tx|ty) is the calculated position of the parent
   */
  // FIXME: Event regions (`regionContext`) aren't ported.
  struct PaintInfo
  {
    LayoutRect Rect;
    PaintPhase Phase;
    EnumSet<boo::PaintBehavior> PaintBehavior;
    RenderObject *SubtreePaintRoot;               // used to draw just one element and its visual children
    const RenderLayerModelObject *PaintContainer; // the layer object that originates the current painting
    /// @brief The inlines whose outlines the inline content being painted paints after its boxes.
    ListHashSet<const RenderInline *> *OutlineObjects {nullptr};
    bool RequireSecurityOriginAccessForWidgets {false};

  private:
    const RenderLayer *_enclosingSelfPaintingLayer {nullptr};
    GraphicsContext *_context;

  public:
    PaintInfo(GraphicsContext &newContext, const LayoutRect &newRect, PaintPhase newPhase,
              EnumSet<boo::PaintBehavior> newPaintBehavior, RenderObject *newSubtreePaintRoot = nullptr,
              const RenderLayerModelObject *newPaintContainer = nullptr,
              const RenderLayer *enclosingSelfPaintingLayer = nullptr,
              bool newRequireSecurityOriginAccessForWidgets = false) noexcept
        : Rect(newRect), Phase(newPhase), PaintBehavior(newPaintBehavior),
          SubtreePaintRoot(newSubtreePaintRoot), PaintContainer(newPaintContainer),
          RequireSecurityOriginAccessForWidgets(newRequireSecurityOriginAccessForWidgets),
          _enclosingSelfPaintingLayer(enclosingSelfPaintingLayer), _context(&newContext)
    {
    }

    KRYS_NODISCARD GraphicsContext &Context() const noexcept
    {
      krys_debug_assert(_context);
      return *_context;
    }

    void SetContext(GraphicsContext &context) noexcept
    {
      _context = &context;
    }

    void UpdateSubtreePaintRootForChildren(const RenderObject *renderer) noexcept
    {
      if (!SubtreePaintRoot)
      {
        return;
      }

      // If we're the painting root, kids draw normally, and see root of nullptr.
      if (SubtreePaintRoot == renderer)
      {
        SubtreePaintRoot = nullptr;
        return;
      }
    }

    KRYS_NODISCARD bool ShouldPaintWithinRoot(const RenderObject &renderer) const noexcept
    {
      return !SubtreePaintRoot || SubtreePaintRoot == &renderer;
    }

    KRYS_NODISCARD bool ForceTextColor() const noexcept
    {
      return ForceBlackText() || ForceWhiteText();
    }
    KRYS_NODISCARD bool ForceBlackText() const noexcept
    {
      return PaintBehavior.Contains(boo::PaintBehavior::ForceBlackText);
    }
    KRYS_NODISCARD bool ForceWhiteText() const noexcept
    {
      return PaintBehavior.Contains(boo::PaintBehavior::ForceWhiteText);
    }
    KRYS_NODISCARD Color ForcedTextColor() const noexcept
    {
      return ForceBlackText() ? Color::black : Color::white;
    }

    KRYS_NODISCARD bool SkipRootBackground() const noexcept
    {
      return PaintBehavior.Contains(boo::PaintBehavior::SkipRootBackground);
    }
    KRYS_NODISCARD bool PaintRootBackgroundOnly() const noexcept
    {
      return PaintBehavior.Contains(boo::PaintBehavior::RootBackgroundOnly);
    }

    KRYS_NODISCARD const RenderLayer *EnclosingSelfPaintingLayer() const noexcept
    {
      return _enclosingSelfPaintingLayer;
    }

    void ApplyTransform(const AffineTransform &localToAncestorTransform) noexcept
    {
      if (localToAncestorTransform.IsIdentity())
      {
        return;
      }

      Context().ConcatCTM(localToAncestorTransform);

      if (Rect.IsInfinite())
      {
        return;
      }

      FloatRect transformedRect(
        localToAncestorTransform.Inverse().value_or(AffineTransform {}).MapRect(FloatRect(Rect)));
      Rect.SetLocation(LayoutPoint(transformedRect.Location()));
      Rect.SetSize(LayoutSize(transformedRect.Size()));
    }
  };
}
