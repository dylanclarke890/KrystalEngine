/**
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

#include "Krystal.Booey/Rendering/Style/GridSpan.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>

namespace krys::boo
{
  GridSpan GridSpan::UntranslatedDefiniteGridSpan(int startLine, int endLine) noexcept
  {
    return GridSpan(startLine, endLine, UntranslatedDefinite);
  }

  GridSpan GridSpan::TranslatedDefiniteGridSpan(unsigned startLine, unsigned endLine) noexcept
  {
    return GridSpan(static_cast<int>(startLine), static_cast<int>(endLine), TranslatedDefinite);
  }

  GridSpan GridSpan::IndefiniteGridSpan() noexcept
  {
    return GridSpan(0, 1, Indefinite);
  }

  GridSpan GridSpan::MasonryAxisTranslatedDefiniteGridSpan() noexcept
  {
    return GridSpan(0, 1, TranslatedDefinite);
  }

  unsigned GridSpan::IntegerSpan() const noexcept
  {
    krys_debug_assert(!IsIndefinite());
    return static_cast<unsigned>(_endLine - _startLine);
  }

  int GridSpan::UntranslatedStartLine() const noexcept
  {
    krys_debug_assert(_type == UntranslatedDefinite);
    return _startLine;
  }

  int GridSpan::UntranslatedEndLine() const noexcept
  {
    krys_debug_assert(_type == UntranslatedDefinite);
    return _endLine;
  }

  unsigned GridSpan::StartLine() const noexcept
  {
    krys_debug_assert(IsTranslatedDefinite());
    krys_debug_assert(_endLine >= 0);
    return static_cast<unsigned>(_startLine);
  }

  unsigned GridSpan::EndLine() const noexcept
  {
    krys_debug_assert(IsTranslatedDefinite());
    krys_debug_assert(_endLine > 0);
    return static_cast<unsigned>(_endLine);
  }

  GridSpan::GridSpanIterator GridSpan::begin() const noexcept
  {
    krys_debug_assert(IsTranslatedDefinite());
    return static_cast<unsigned>(_startLine);
  }

  GridSpan::GridSpanIterator GridSpan::end() const noexcept
  {
    krys_debug_assert(IsTranslatedDefinite());
    return static_cast<unsigned>(_endLine);
  }

  bool GridSpan::IsTranslatedDefinite() const noexcept
  {
    return _type == TranslatedDefinite;
  }

  bool GridSpan::IsIndefinite() const noexcept
  {
    return _type == Indefinite;
  }

  void GridSpan::Translate(unsigned offset) noexcept
  {
    krys_debug_assert(_type == UntranslatedDefinite);

    _type = TranslatedDefinite;
    _startLine += static_cast<int>(offset);
    _endLine += static_cast<int>(offset);

    krys_debug_assert(_startLine >= 0);
    krys_debug_assert(_endLine > 0);
  }

  // Moves this span to be in the same coordinate space as |parent|.
  // If reverse is specified, then swaps the direction to handle RTL/LTR changes.
  void GridSpan::TranslateTo(const GridSpan &parent, bool reverse) noexcept
  {
    krys_debug_assert(_type == TranslatedDefinite);
    krys_debug_assert(parent._type == TranslatedDefinite);

    if (reverse)
    {
      int start = _startLine;
      _startLine = static_cast<int>(parent.EndLine()) - _endLine;
      _endLine = static_cast<int>(parent.EndLine()) - start;
    }
    else
    {
      _startLine += parent._startLine;
      _endLine += parent._startLine;
    }
  }

  void GridSpan::Clamp(int max) noexcept
  {
    krys_debug_assert(_type != Indefinite);

    _startLine = std::max(_startLine, 0);
    _endLine = std::max(std::min(_endLine, max), 1);
    if (_startLine >= _endLine)
    {
      _startLine = _endLine - 1;
    }
  }

  bool GridSpan::Clamp(int min, int max) noexcept
  {
    krys_debug_assert(min < max);
    krys_debug_assert(_startLine < _endLine);
    krys_debug_assert(_type != Indefinite);

    if (min >= _endLine || max <= _startLine)
    {
      return false;
    }

    _startLine = std::max(_startLine, min);
    _endLine = std::min(_endLine, max);
    krys_debug_assert(_startLine < _endLine);
    return true;
  }

  GridSpan::GridSpan(int startLine, int endLine, GridSpanType type) noexcept : _type(type)
  {
    krys_debug_assert(startLine < endLine);
    krys_debug_assert(type != TranslatedDefinite || (startLine >= 0 && endLine > 0));

    _startLine = std::max(style::GridPosition::Min(), std::min(startLine, style::GridPosition::Max() - 1));
    _endLine = std::max(style::GridPosition::Min() + 1, std::min(endLine, style::GridPosition::Max()));
  }
}
