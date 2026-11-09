/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::style
{
  EnumSet<Change> DetermineChanges(const RenderStyle &s1, const RenderStyle &s2) noexcept
  {
    EnumSet<Change> result;

    if (!s1.NonInheritedEqual(s2))
    {
      result.Add(Change::NonInherited);
    }
    if (!s1.NonFastPathInheritedEqual(s2))
    {
      result.Add(Change::Inherited);
    }
    if (!s1.FastPathInheritedEqual(s2))
    {
      result.Add(Change::FastPathInherited);
    }

    if (!s1.DescendantAffectingNonInheritedPropertiesEqual(s2))
    {
      result.Add(Change::Inherited);
    }

    // We just detach if a renderer acquires or loses a column-span, since spanning elements
    // typically won't contain much content.
    auto columnSpanNeedsNewRenderer = [&]
    {
      if (!s1.ColumnSpanEqual(s2))
      {
        return true;
      }
      if (s1.GetColumnSpan() != ColumnSpan::All)
      {
        return false;
      }
      // Spanning in ignored for floating and out-of-flow boxes.
      return s1.IsFloating() != s2.IsFloating() || s1.HasOutOfFlowPosition() != s2.HasOutOfFlowPosition();
    };

    auto needsRendererUpdate = [&]
    {
      if (s1.Display() != s2.Display())
      {
        return true;
      }
      if (s1.HasPseudoStyle(PseudoElementType::FirstLetter)
          != s2.HasPseudoStyle(PseudoElementType::FirstLetter))
      {
        return true;
      }
      if (columnSpanNeedsNewRenderer())
      {
        return true;
      }
      // When text-combine is on, we use RenderCombineText, otherwise RenderText.
      // https://bugs.webkit.org/show_bug.cgi?id=55069
      if (s1.HasTextCombine() != s2.HasTextCombine())
      {
        return true;
      }
      if (s1.GetContent() != s2.GetContent())
      {
        return true;
      }
      return false;
    };

    if (needsRendererUpdate())
    {
      result.Add(Change::Renderer);
    }

    // Query container changes affect descendant style.
    if (!s1.ContainerTypeAndNamesEqual(s2))
    {
      result.Add(Change::Container);
    }

    return result;
  }
}
