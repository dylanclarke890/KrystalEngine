/*
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 * Copyright (C) 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
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
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class SVGResourcesCache;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::svg
{
  class SVGElement;
  class SVGSvgElement;

  /// @brief What a document keeps for its SVG content: its resources, the elements whose references wait to
  /// be rebuilt, and its `<svg>` elements' time containers.
  class SVGDocumentExtensions final
  {
    KRYS_NON_COPY_MOVABLE(SVGDocumentExtensions);

  private:
    WeakPtr<dom::Document> _document;
    UniquePtr<SVGResourcesCache> _resourcesCache;
    List<Ref<SVGElement>> _rebuildElements;
    // For SVG 1.2 support this will need to be made more general.
    List<WeakPtr<SVGSvgElement>> _timeContainers;
    bool _areAnimationsPaused;

  public:
    explicit SVGDocumentExtensions(dom::Document &) noexcept;
    ~SVGDocumentExtensions() noexcept;

    KRYS_NODISCARD SVGResourcesCache &ResourcesCache() noexcept
    {
      return *_resourcesCache;
    }

    /// @brief Remembers @p element, whose reference target was removed, to look its target up again after the
    /// tree mutation.
    void AddElementToRebuild(SVGElement &element) noexcept;
    void RemoveElementToRebuild(SVGElement &element) noexcept;
    void RebuildElements() noexcept;

    void AddTimeContainer(SVGSvgElement &) noexcept;
    void RemoveTimeContainer(SVGSvgElement &) noexcept;
    KRYS_NODISCARD List<Ref<SVGSvgElement>> TimeContainers() const noexcept;

    void StartAnimations() noexcept;
    void PauseAnimations() noexcept;
    void UnpauseAnimations() noexcept;
    /// @brief Samples the animations of the time containers that asked for this rendering update.
    void ServiceAnimations() noexcept;
    void DispatchLoadEventToOutermostSVGElements() noexcept;

    KRYS_NODISCARD bool AreAnimationsPaused() const noexcept
    {
      return _areAnimationsPaused;
    }

    void ClearTargetDependencies(SVGElement &referencedElement) noexcept;
    void RebuildAllElementReferencesForTarget(SVGElement &referencedElement) noexcept;
  };
}
