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
