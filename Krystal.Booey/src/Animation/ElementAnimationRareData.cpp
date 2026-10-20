#include "Krystal.Booey/Animation/ElementAnimationRareData.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo
{
  ElementAnimationRareData::ElementAnimationRareData() noexcept = default;

  ElementAnimationRareData::~ElementAnimationRareData() noexcept
  {
    krys_debug_assert(!_keyframeEffectStack || !_keyframeEffectStack->HasEffects());
  }

  KeyframeEffectStack &ElementAnimationRareData::EnsureKeyframeEffectStack() noexcept
  {
    if (!_keyframeEffectStack)
    {
      _keyframeEffectStack = CreateUnique<boo::KeyframeEffectStack>();
    }
    return *_keyframeEffectStack;
  }

  void ElementAnimationRareData::SetAnimationsCreatedByMarkup(CSSAnimationCollection &&animations) noexcept
  {
    if (_keyframeEffectStack)
    {
      for (const auto &animation : _animationsCreatedByMarkup)
      {
        if (RefPtr keyframeEffect = ShareRefPtr(animation->GetKeyframeEffect()))
        {
          _keyframeEffectStack->RemoveEffect(*keyframeEffect);
        }
      }
    }

    _animationsCreatedByMarkup = krys::move(animations);
  }

  void ElementAnimationRareData::SetLastStyleChangeEventStyle(UniquePtr<const RenderStyle> &&style) noexcept
  {
    // FIXME: Telling the keyframe effect stack the style changed, for accelerated animations, once there are
    // accelerated animations.
    _lastStyleChangeEventStyle = krys::move(style);
  }
}
