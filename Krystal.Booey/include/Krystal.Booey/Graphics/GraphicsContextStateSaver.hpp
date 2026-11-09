/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Graphics/GraphicsContext.hpp"

namespace krys::boo
{
  /// @brief Saves a graphics context's state for the length of its scope.
  class GraphicsContextStateSaver
  {
    KRYS_NON_COPY_MOVABLE(GraphicsContextStateSaver);

  private:
    GraphicsContext &_context;
    bool _saveAndRestore;

  public:
    GraphicsContextStateSaver(GraphicsContext &context, bool saveAndRestore = true) noexcept
        : _context(context), _saveAndRestore(saveAndRestore)
    {
      if (_saveAndRestore)
      {
        _context.Save();
      }
    }

    ~GraphicsContextStateSaver() noexcept
    {
      if (_saveAndRestore)
      {
        _context.Restore();
      }
    }

    void Save() noexcept
    {
      krys_debug_assert(!_saveAndRestore);
      _context.Save();
      _saveAndRestore = true;
    }

    void Restore() noexcept
    {
      krys_debug_assert(_saveAndRestore);
      _context.Restore();
      _saveAndRestore = false;
    }

    KRYS_NODISCARD GraphicsContext *Context() const noexcept
    {
      return &_context;
    }
  };

  /// @brief Draws into a transparency layer for the length of its scope.
  class TransparencyLayerScope
  {
    KRYS_NON_COPY_MOVABLE(TransparencyLayerScope);

  private:
    GraphicsContext &_context;
    float _alpha;
    bool _beganLayer;

  public:
    TransparencyLayerScope(GraphicsContext &context, float alpha, bool beginLayer = true) noexcept
        : _context(context), _alpha(alpha), _beganLayer(beginLayer)
    {
      if (beginLayer)
      {
        _context.BeginTransparencyLayer(_alpha);
      }
    }

    void BeginLayer(float alpha) noexcept
    {
      _alpha = alpha;
      _context.BeginTransparencyLayer(_alpha);
      _beganLayer = true;
    }

    ~TransparencyLayerScope() noexcept
    {
      if (_beganLayer)
      {
        _context.EndTransparencyLayer();
      }
    }
  };

  /// @brief Checks, in debug builds, that a scope leaves a graphics context's state stack as it found it.
  class GraphicsContextStateStackChecker
  {
    KRYS_NON_COPY_MOVABLE(GraphicsContextStateStackChecker);

  private:
    GraphicsContext &_context;
    uint32 _stackSize;

  public:
    GraphicsContextStateStackChecker(GraphicsContext &context) noexcept
        : _context(context), _stackSize(context.StackSize())
    {
    }

    ~GraphicsContextStateStackChecker() noexcept
    {
      krys_debug_assert_msg(_context.StackSize() == _stackSize, "GraphicsContext stack changed");
    }
  };

  /// @brief Uses an image interpolation quality for the length of its scope.
  class InterpolationQualityMaintainer
  {
    KRYS_NON_COPY_MOVABLE(InterpolationQualityMaintainer);

  private:
    GraphicsContext &_graphicsContext;
    InterpolationQuality _currentInterpolationQuality;
    bool _interpolationQualityChanged;

  public:
    explicit InterpolationQualityMaintainer(GraphicsContext &graphicsContext,
                                            InterpolationQuality interpolationQualityToUse) noexcept
        : _graphicsContext(graphicsContext),
          _currentInterpolationQuality(graphicsContext.ImageInterpolationQuality()),
          _interpolationQualityChanged(interpolationQualityToUse != InterpolationQuality::Default
                                       && _currentInterpolationQuality != interpolationQualityToUse)
    {
      if (_interpolationQualityChanged)
      {
        _graphicsContext.SetImageInterpolationQuality(interpolationQualityToUse);
      }
    }

    explicit InterpolationQualityMaintainer(GraphicsContext &graphicsContext,
                                            Maybe<InterpolationQuality> interpolationQuality) noexcept
        : InterpolationQualityMaintainer(graphicsContext, interpolationQuality
                                                            ? interpolationQuality.value()
                                                            : graphicsContext.ImageInterpolationQuality())
    {
    }

    ~InterpolationQualityMaintainer() noexcept
    {
      if (_interpolationQualityChanged)
      {
        _graphicsContext.SetImageInterpolationQuality(_currentInterpolationQuality);
      }
    }
  };
}
