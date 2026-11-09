/*
 * Copyright (C) 2016-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"

namespace krys::boo::display_list
{
  /// @brief A graphics context that records what's drawn, tracking the transform and clip bounds as it goes.
  /// FIXME: Decomposing glyphs into their layers as they're recorded (`DrawGlyphsMode`) is for Core Text and
  /// Skia text blobs; recorded glyphs are drawn as they are.
  class Recorder : public GraphicsContext
  {
    KRYS_NON_COPYABLE(Recorder);

  protected:
    struct ContextState
    {
      GraphicsContextState State;
      AffineTransform Ctm;
      FloatRect ClipBounds;
      Maybe<GraphicsContextState> LastDrawingState {std::nullopt};

      KRYS_NODISCARD ContextState CloneForTransparencyLayer() const noexcept
      {
        auto stateClone = State.Clone(GraphicsContextState::Purpose::TransparencyLayer);
        Maybe<GraphicsContextState> lastDrawingStateClone;
        if (LastDrawingState)
        {
          lastDrawingStateClone = LastDrawingState->Clone(GraphicsContextState::Purpose::TransparencyLayer);
        }
        return ContextState {krys::move(stateClone), Ctm, ClipBounds, krys::move(lastDrawingStateClone)};
      }

      void Translate(float x, float y) noexcept;
      void Rotate(float angleInRadians) noexcept;
      void Scale(const FloatSize &) noexcept;
      void ConcatCTM(const AffineTransform &) noexcept;
      void SetCTM(const AffineTransform &) noexcept;
    };

  private:
    SmallList<ContextState, 4> _stateStack;
    const FloatRect _initialClip;

  public:
    Recorder(const GraphicsContextState &state, const FloatRect &initialClip,
             const AffineTransform &transform) noexcept
        : Recorder(IsDeferred::Yes, state, initialClip, transform)
    {
    }
    virtual ~Recorder() noexcept;

    void AppendDisplayList(const DisplayList &) noexcept;

  protected:
    Recorder(IsDeferred, const GraphicsContextState &, const FloatRect &initialClip,
             const AffineTransform &) noexcept;

    KRYS_NODISCARD const SmallList<ContextState, 4> &StateStack() const noexcept
    {
      return _stateStack;
    }
    KRYS_NODISCARD const ContextState &CurrentState() const noexcept;
    KRYS_NODISCARD ContextState &CurrentState() noexcept;

    void UpdateStateForSave(GraphicsContextState::Purpose) noexcept;
    KRYS_NODISCARD bool UpdateStateForRestore(GraphicsContextState::Purpose) noexcept;
    KRYS_NODISCARD bool UpdateStateForTranslate(float x, float y) noexcept;
    KRYS_NODISCARD bool UpdateStateForRotate(float angleInRadians) noexcept;
    KRYS_NODISCARD bool UpdateStateForScale(const FloatSize &) noexcept;
    KRYS_NODISCARD bool UpdateStateForConcatCTM(const AffineTransform &) noexcept;
    void UpdateStateForSetCTM(const AffineTransform &) noexcept;
    void UpdateStateForBeginTransparencyLayer(float opacity) noexcept;
    void UpdateStateForBeginTransparencyLayer(CompositeOperator, boo::BlendMode) noexcept;
    KRYS_NODISCARD bool UpdateStateForEndTransparencyLayer() noexcept;
    void UpdateStateForResetClip() noexcept;
    void UpdateStateForClip(const FloatRect &) noexcept;
    void UpdateStateForClipRoundedRect(const FloatRoundedRect &) noexcept;
    void UpdateStateForClipPath(const Path &) noexcept;
    void UpdateStateForClipToImageBuffer(const FloatRect &) noexcept;
    void UpdateStateForClipOut(const FloatRect &) noexcept;
    void UpdateStateForClipOut(const Path &) noexcept;
    void UpdateStateForClipOutRoundedRect(const FloatRoundedRect &) noexcept;
    void UpdateStateForApplyDeviceScaleFactor(float) noexcept;

    KRYS_NODISCARD FloatRect InitialClip() const noexcept;

  private:
    void FillRoundedRectImpl(const FloatRoundedRect &, const Color &) noexcept final
    {
      krys_debug_assert(false);
    }

    KRYS_NODISCARD const GraphicsContextState &State() const noexcept final;
    void DidUpdateState(GraphicsContextState &) noexcept final;
    void DidUpdateSingleState(GraphicsContextState &, GraphicsContextState::ChangeIndex) noexcept final;

    KRYS_NODISCARD
    AffineTransform GetCTM(IncludeDeviceScale = PossiblyIncludeDeviceScale) const noexcept final;
    KRYS_NODISCARD IntRect ClipBounds() const noexcept final;

    virtual void AppendStateChangeItemIfNecessary() noexcept = 0;

    KRYS_NODISCARD const AffineTransform &Ctm() const noexcept;
  };

  inline const Recorder::ContextState &Recorder::CurrentState() const noexcept
  {
    krys_debug_assert(_stateStack.size());
    return _stateStack.back();
  }

  inline Recorder::ContextState &Recorder::CurrentState() noexcept
  {
    krys_debug_assert(_stateStack.size());
    return _stateStack.back();
  }
}
