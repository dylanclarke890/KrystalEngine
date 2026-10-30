#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorder.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo::display_list
{
  Recorder::Recorder(IsDeferred isDeferred, const GraphicsContextState &state, const FloatRect &initialClip,
                     const AffineTransform &initialCTM) noexcept
      : GraphicsContext(isDeferred, state), _initialClip(initialCTM.MapRect(initialClip))
  {
    krys_debug_assert(state.Changes().IsEmpty());
    _stateStack.push_back({state, initialCTM, initialCTM.MapRect(initialClip)});
  }

  Recorder::~Recorder() noexcept
  {
    krys_debug_assert(_stateStack.size() == 1); // If this fires, it indicates mismatched save/restore.
  }

  void Recorder::AppendDisplayList(const DisplayList &displayList) noexcept
  {
    GraphicsContext::DrawDisplayList(displayList);
  }

  const GraphicsContextState &Recorder::State() const noexcept
  {
    return CurrentState().State;
  }

  void Recorder::DidUpdateState(GraphicsContextState &state) noexcept
  {
    CurrentState().State.MergeLastChanges(state, CurrentState().LastDrawingState);
    state.DidApplyChanges();
  }

  void Recorder::DidUpdateSingleState(GraphicsContextState &state,
                                      GraphicsContextState::ChangeIndex changeIndex) noexcept
  {
    krys_debug_assert(
      [&]
      {
        auto changes = state.Changes();
        changes.Remove(changeIndex.ToChange());
        return changes.IsEmpty();
      }());
    CurrentState().State.MergeSingleChange(state, changeIndex, CurrentState().LastDrawingState);
    state.DidApplyChanges();
  }

  void Recorder::UpdateStateForSave(KRYS_MAYBE_UNUSED GraphicsContextState::Purpose purpose) noexcept
  {
    krys_debug_assert(purpose == GraphicsContextState::Purpose::SaveRestore);
    AppendStateChangeItemIfNecessary();
    GraphicsContext::Save(purpose);
    _stateStack.push_back(_stateStack.back());
  }

  bool Recorder::UpdateStateForRestore(GraphicsContextState::Purpose purpose) noexcept
  {
    if (_stateStack.size() <= 1)
    {
      return false;
    }

    krys_debug_assert(purpose == GraphicsContextState::Purpose::SaveRestore);
    AppendStateChangeItemIfNecessary();
    GraphicsContext::Restore(purpose);
    _stateStack.pop_back();
    return true;
  }

  bool Recorder::UpdateStateForTranslate(float x, float y) noexcept
  {
    if (!x && !y)
    {
      return false;
    }
    CurrentState().Translate(x, y);
    return true;
  }

  bool Recorder::UpdateStateForRotate(float angleInRadians) noexcept
  {
    if (AreEssentiallyEqual(0.f, std::fmodf(angleInRadians, std::numbers::pi_v<float> * 2.f)))
    {
      return false;
    }
    CurrentState().Rotate(angleInRadians);
    return true;
  }

  bool Recorder::UpdateStateForScale(const FloatSize &size) noexcept
  {
    if (AreEssentiallyEqual(size, FloatSize {1.f, 1.f}))
    {
      return false;
    }
    CurrentState().Scale(size);
    return true;
  }

  bool Recorder::UpdateStateForConcatCTM(const AffineTransform &transform) noexcept
  {
    if (transform.IsIdentity())
    {
      return false;
    }
    CurrentState().ConcatCTM(transform);
    return true;
  }

  void Recorder::UpdateStateForSetCTM(const AffineTransform &transform) noexcept
  {
    CurrentState().SetCTM(transform);
  }

  AffineTransform Recorder::GetCTM(GraphicsContext::IncludeDeviceScale) const noexcept
  {
    return CurrentState().Ctm;
  }

  void Recorder::UpdateStateForBeginTransparencyLayer(float opacity) noexcept
  {
    GraphicsContext::BeginTransparencyLayer(opacity);
    AppendStateChangeItemIfNecessary();
    GraphicsContext::Save(GraphicsContextState::Purpose::TransparencyLayer);
    _stateStack.push_back(_stateStack.back().CloneForTransparencyLayer());
  }

  void Recorder::UpdateStateForBeginTransparencyLayer(CompositeOperator compositeOperator,
                                                      boo::BlendMode blendMode) noexcept
  {
    GraphicsContext::BeginTransparencyLayer(compositeOperator, blendMode);
    AppendStateChangeItemIfNecessary();
    GraphicsContext::Save(GraphicsContextState::Purpose::TransparencyLayer);
    _stateStack.push_back(_stateStack.back().CloneForTransparencyLayer());
  }

  bool Recorder::UpdateStateForEndTransparencyLayer() noexcept
  {
    if (StateStack().size() <= 1)
    {
      return false;
    }

    GraphicsContext::EndTransparencyLayer();
    AppendStateChangeItemIfNecessary();
    _stateStack.pop_back();
    GraphicsContext::Restore(GraphicsContextState::Purpose::TransparencyLayer);
    return true;
  }

  void Recorder::UpdateStateForResetClip() noexcept
  {
    // The clip bounds are in device coordinates, as the initial clip is kept.
    CurrentState().ClipBounds = _initialClip;
  }

  FloatRect Recorder::InitialClip() const noexcept
  {
    if (auto inverse = Ctm().Inverse())
    {
      return inverse->MapRect(_initialClip);
    }
    return _initialClip;
  }

  void Recorder::UpdateStateForClip(const FloatRect &rect) noexcept
  {
    AppendStateChangeItemIfNecessary(); // Conservative: we do not know if the clip application might use
                                        // state such as antialiasing.
    CurrentState().ClipBounds.Intersect(CurrentState().Ctm.MapRect(rect));
  }

  void Recorder::UpdateStateForClipRoundedRect(const FloatRoundedRect &rect) noexcept
  {
    AppendStateChangeItemIfNecessary(); // Conservative: we do not know if the clip application might use
                                        // state such as antialiasing.
    CurrentState().ClipBounds.Intersect(CurrentState().Ctm.MapRect(rect.Rect()));
  }

  void Recorder::UpdateStateForClipOut(const FloatRect &) noexcept
  {
    AppendStateChangeItemIfNecessary(); // Conservative: we do not know if the clip application might use
                                        // state such as antialiasing.
    // FIXME: Should we update the clip bounds?
  }

  void Recorder::UpdateStateForClipOutRoundedRect(const FloatRoundedRect &) noexcept
  {
    AppendStateChangeItemIfNecessary(); // Conservative: we do not know if the clip application might use
                                        // state such as antialiasing.
    // FIXME: Should we update the clip bounds?
  }

  void Recorder::UpdateStateForClipOut(const Path &) noexcept
  {
    AppendStateChangeItemIfNecessary(); // Conservative: we do not know if the clip application might use
                                        // state such as antialiasing.
    // FIXME: Should we update the clip bounds?
  }

  void Recorder::UpdateStateForClipPath(const Path &path) noexcept
  {
    AppendStateChangeItemIfNecessary(); // Conservative: we do not know if the clip application might use
                                        // state such as antialiasing.
    CurrentState().ClipBounds.Intersect(CurrentState().Ctm.MapRect(path.FastBoundingRect()));
  }

  void Recorder::UpdateStateForClipToImageBuffer(const FloatRect &rect) noexcept
  {
    AppendStateChangeItemIfNecessary(); // Conservative: we do not know if the clip application might use
                                        // state such as antialiasing.
    CurrentState().ClipBounds.Intersect(CurrentState().Ctm.MapRect(rect));
  }

  IntRect Recorder::ClipBounds() const noexcept
  {
    if (auto inverse = CurrentState().Ctm.Inverse())
    {
      return EnclosingIntRect(inverse->MapRect(CurrentState().ClipBounds));
    }
    // If the CTM is not invertible, return the original rect.
    // This matches CGRectApplyInverseAffineTransform behavior.
    return EnclosingIntRect(CurrentState().ClipBounds);
  }

  void Recorder::UpdateStateForApplyDeviceScaleFactor(float deviceScaleFactor) noexcept
  {
    // We modify the state directly here instead of calling GraphicsContext::scale()
    // because the recorded item will scale() when replayed.
    CurrentState().Scale({deviceScaleFactor, deviceScaleFactor});
    // FIXME: this changes the baseCTM, which will invalidate all of our cached extents.
    // Assert that it's only called early on?
  }

  const AffineTransform &Recorder::Ctm() const noexcept
  {
    return CurrentState().Ctm;
  }

  void Recorder::ContextState::Translate(float x, float y) noexcept
  {
    Ctm.Translate(x, y);
  }

  void Recorder::ContextState::Rotate(float angleInRadians) noexcept
  {
    Ctm.Rotate(rad2deg(static_cast<double>(angleInRadians)));
  }

  void Recorder::ContextState::Scale(const FloatSize &size) noexcept
  {
    Ctm.Scale(size);
  }

  void Recorder::ContextState::SetCTM(const AffineTransform &matrix) noexcept
  {
    Ctm = matrix;
  }

  void Recorder::ContextState::ConcatCTM(const AffineTransform &matrix) noexcept
  {
    Ctm.Multiply(matrix);
  }
}
