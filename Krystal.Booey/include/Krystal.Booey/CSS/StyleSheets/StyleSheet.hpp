/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2006, 2008, 2012-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::dom
{
  class Node;
}

namespace krys::boo::css
{
  class CSSImportRule;
  class MediaList;

  /// @brief The base of the style sheets exposed to scripts.
  /// @see https://drafts.csswg.org/cssom/#the-stylesheet-interface
  class StyleSheet : public RefCounted<StyleSheet>
  {
  public:
    virtual ~StyleSheet() noexcept = default;

    KRYS_NODISCARD virtual bool Disabled() const noexcept = 0;
    virtual void SetDisabled(bool disabled) noexcept = 0;
    KRYS_NODISCARD virtual dom::Node *OwnerNode() const noexcept = 0;

    KRYS_NODISCARD virtual StyleSheet *ParentStyleSheet() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual CSSOMString Href() const noexcept = 0;
    KRYS_NODISCARD virtual CSSOMString Title() const noexcept = 0;

    KRYS_NODISCARD virtual MediaList *Media() const noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual CSSOMString Type() const noexcept = 0;

    KRYS_NODISCARD virtual CSSImportRule *OwnerRule() const noexcept
    {
      return nullptr;
    }

    virtual void ClearOwnerNode() noexcept = 0;
    KRYS_NODISCARD virtual url::URL BaseURL() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsLoading() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsCSSStyleSheet() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsXSLStyleSheet() const noexcept
    {
      return false;
    }
  };
}
