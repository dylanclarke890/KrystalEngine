#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Animations.hpp"

namespace krys::boo::css
{
  KeyframesRule::KeyframesRule(const CSSOMStringAtom &name) noexcept : Rule(RuleType::Keyframes), _name(name)
  {
  }

  KeyframesRule::KeyframesRule(const KeyframesRule &other) noexcept
      : Rule(other), _keyframes(other.Keyframes()), _name(other._name)
  {
  }

  KeyframesRule::~KeyframesRule() noexcept = default;

  Ref<KeyframesRule> KeyframesRule::Create(const CSSOMStringAtom &name) noexcept
  {
    return AdoptRef(*new KeyframesRule(name));
  }

  Ref<KeyframesRule> KeyframesRule::Copy() const noexcept
  {
    return AdoptRef(*new KeyframesRule(*this));
  }

  void KeyframesRule::ParserAppendKeyframe(RefPtr<KeyframeRule> &&keyframe) noexcept
  {
    if (!keyframe)
    {
      return;
    }
    _keyframes.push_back(Ref<KeyframeRule>::WithRef(*keyframe));
  }

  void KeyframesRule::WrapperAppendKeyframe(Ref<KeyframeRule> &&keyframe) noexcept
  {
    _keyframes.push_back(krys::move(keyframe));
  }

  void KeyframesRule::WrapperRemoveKeyframe(uint32 index) noexcept
  {
    _keyframes.erase(_keyframes.begin() + index);
  }

  Maybe<size_t> KeyframesRule::FindKeyframeIndex(const CSSOMString &key) const noexcept
  {
    auto keys = PropertyParserHelpers::ParseKeyframeKeyList(key, StrictParserContext());
    if (keys.empty())
    {
      return {};
    }

    List<KeyframeRule::Key> convertedKeys;
    convertedKeys.reserve(keys.size());
    for (const auto &[rangeName, offset] : keys)
    {
      convertedKeys.push_back({rangeName, offset});
    }

    for (auto i = _keyframes.size(); i--;)
    {
      if (_keyframes[i]->Keys() == convertedKeys)
      {
        return i;
      }
    }
    return {};
  }

  void KeyframesRule::ShrinkToFit() noexcept
  {
    _keyframes.shrink_to_fit();
  }
}
