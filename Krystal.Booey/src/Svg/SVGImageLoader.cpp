/*
 * Copyright (C) 2005, 2005 Alexander Kellett <lypanov@kde.org>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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

#include "Krystal.Booey/Svg/SVGImageLoader.hpp"
#include "Krystal.Booey/Dom/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/Dom/Events/EventNames.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Svg/Elements/SVGImageElement.hpp"

namespace krys::boo
{
  SVGImageLoader::SVGImageLoader(SVGImageElement &element) noexcept : ImageLoader(element)
  {
  }

  SVGImageLoader::~SVGImageLoader() noexcept = default;

  void SVGImageLoader::DispatchLoadEvent() noexcept
  {
    if (Image()->ErrorOccurred())
    {
      (void)dom::EventDispatcher::FireEvent(event_names::Error, GetElement());
    }
    else
    {
      Ref<SVGImageElement> element = ShareRef(Downcast<SVGImageElement>(Downcast<SVGElement>(GetElement())));
      element->SendLoadEventIfPossible();
    }
  }
}
