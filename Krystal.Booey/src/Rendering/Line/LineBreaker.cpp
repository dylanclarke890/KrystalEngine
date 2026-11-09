/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003, 2004, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All right reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2013 ChangSeok Oh <shivamidow@gmail.com>
 * Copyright (C) 2013 Adobe Systems Inc. All right reserved.
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

#include "Krystal.Booey/Rendering/Line/LineBreaker.hpp"

#include "Krystal.Booey/Rendering/Line/BreakingContext.hpp"

namespace krys::boo
{
  void LineBreaker::SkipTrailingWhitespace(LegacyInlineIterator &iterator, const LineInfo &lineInfo) noexcept
  {
    while (!iterator.AtEnd() && !RequiresLineBox(iterator, lineInfo, WhitespacePosition::TrailingWhitespace))
    {
      iterator.Increment();
    }
  }

  void LineBreaker::SkipLeadingWhitespace(InlineBidiResolver &resolver, LineInfo &lineInfo) noexcept
  {
    while (!resolver.Position().AtEnd()
           && !RequiresLineBox(resolver.Position(), lineInfo, WhitespacePosition::LeadingWhitespace))
    {
      resolver.Increment();
    }

    resolver.CommitExplicitEmbedding();
  }

  LegacyInlineIterator LineBreaker::NextLineBreak(InlineBidiResolver &resolver, LineInfo &lineInfo,
                                                  RenderTextInfo &renderTextInfo) noexcept
  {
    krys_debug_assert(resolver.Position().Root() == _block.get());

    bool appliedStartWidth = resolver.Position().Offset();

    LineWidth width(*_block);

    SkipLeadingWhitespace(resolver, lineInfo);

    if (resolver.Position().AtEnd())
    {
      return resolver.Position();
    }

    BreakingContext context(*this, resolver, lineInfo, width, renderTextInfo, appliedStartWidth, *_block);

    while (context.CurrentObject())
    {
      context.InitializeForCurrentObject();
      if (context.CurrentObject()->IsRenderInline())
      {
        context.HandleEmptyInline();
      }
      else if (context.CurrentObject()->IsRenderText())
      {
        if (context.HandleText())
        {
          // We've hit a hard text line break. Our line break iterator is updated, so early return.
          return context.LineBreak();
        }
      }
      else if (context.CurrentObject()->IsLineBreakOpportunity())
      {
        context.CommitLineBreakAtCurrentWidth(*context.CurrentObject());
      }
      else
      {
        krys_debug_assert(false);
      }

      if (context.AtEnd())
      {
        return context.HandleEndOfLine();
      }

      context.CommitAndUpdateLineBreakIfNeeded();

      if (context.AtEnd())
      {
        return context.HandleEndOfLine();
      }

      context.Increment();
    }

    context.ClearLineBreakIfFitsOnLine(true);

    return context.HandleEndOfLine();
  }
}
