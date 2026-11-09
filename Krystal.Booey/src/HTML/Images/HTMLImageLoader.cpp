/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2010, 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Images/HTMLImageLoader.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo::html
{
  HTMLImageLoader::HTMLImageLoader(dom::Element &element) noexcept : ImageLoader(element)
  {
  }

  HTMLImageLoader::~HTMLImageLoader() noexcept = default;

  void HTMLImageLoader::DispatchLoadEvent() noexcept
  {
    // FIXME: Video posters fire no events, once they load images.
    bool errorOccurred = Image()->ErrorOccurred();
    (void)dom::EventDispatcher::FireEvent(
      dom::DOMStringAtom(errorOccurred ? EventNames::Error : EventNames::Load), GetElement());
  }

  void HTMLImageLoader::NotifyFinished(CachedResource &resource) noexcept
  {
    krys_debug_assert(Image());
    Ref protect = ShareRef(GetElement());
    ImageLoader::NotifyFinished(resource);
    // FIXME: An <object> renders its fallback content when the load failed, once there are <object> loads.
  }
}
