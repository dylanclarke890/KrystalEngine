#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimationAdditiveFunction.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <utility>

namespace krys::boo::svg
{
  /// @brief An additive animation function over a list type, interpolating item by item.
  template <typename TList>
  class SVGAnimationAdditiveListFunction : public SVGAnimationAdditiveFunction
  {
  protected:
    const Ref<TList> _from;
    const Ref<TList> _to;
    const Ref<TList> _toAtEndOfDuration;

  public:
    template <typename... TArgs>
    SVGAnimationAdditiveListFunction(AnimationMode animationMode, CalcMode calcMode, bool isAccumulated,
                                     bool isAdditive, TArgs &&...arguments) noexcept
        : SVGAnimationAdditiveFunction(animationMode, calcMode, isAccumulated, isAdditive),
          _from(TList::Create(std::forward<TArgs>(arguments)...)),
          _to(TList::Create(std::forward<TArgs>(arguments)...)),
          _toAtEndOfDuration(TList::Create(std::forward<TArgs>(arguments)...))
    {
    }

  protected:
    KRYS_NODISCARD const Ref<TList> &ToAtEndOfDuration() const noexcept
    {
      return !_toAtEndOfDuration->IsEmpty() ? _toAtEndOfDuration : _to;
    }

    bool AdjustAnimatedList(AnimationMode animationMode, float percentage, RefPtr<TList> &animated,
                            bool resizeAnimatedIfNeeded = true) noexcept
    {
      if (!_to->NumberOfItems())
      {
        return false;
      }

      if (_from->NumberOfItems() && _from->Size() != _to->Size())
      {
        if (percentage >= 0.5)
        {
          *animated = *_to;
        }
        else if (animationMode != AnimationMode::To)
        {
          *animated = *_from;
        }
        return false;
      }

      if (resizeAnimatedIfNeeded && animated->Size() < _to->Size())
      {
        animated->Resize(_to->Size());
      }
      return true;
    }
  };
}
