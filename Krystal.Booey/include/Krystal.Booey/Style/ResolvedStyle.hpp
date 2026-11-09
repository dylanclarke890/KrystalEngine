/**
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Booey/Style/Rules/MatchResult.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::style
{
  /// @brief An element's style as the builder computed it, before the adjuster.
  struct UnadjustedStyle
  {
    UniquePtr<RenderStyle> Style;
    UniquePtr<style::Relations> Relations {};
    RefPtr<const style::MatchResult> MatchResult {};
  };

  /// @brief An element's resolved style, with the relations and matched declarations it came from.
  struct ResolvedStyle
  {
    UniquePtr<RenderStyle> Style;
    UniquePtr<style::Relations> Relations {};
    RefPtr<const style::MatchResult> MatchResult {};
  };
}
