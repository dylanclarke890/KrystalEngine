#pragma once

#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  /// @brief `@keyframes`.
  class KeyframesRule final : public Rule
  {
  private:
    mutable List<Ref<KeyframeRule>> _keyframes;
    CSSOMStringAtom _name;

    explicit KeyframesRule(const CSSOMStringAtom &name) noexcept;
    KeyframesRule(const KeyframesRule &other) noexcept;

  public:
    ~KeyframesRule() noexcept;

    KRYS_NODISCARD static Ref<KeyframesRule> Create(const CSSOMStringAtom &name) noexcept;

    KRYS_NODISCARD Ref<KeyframesRule> Copy() const noexcept;

    KRYS_NODISCARD const List<Ref<KeyframeRule>> &Keyframes() const noexcept
    {
      return _keyframes;
    }

    void ParserAppendKeyframe(RefPtr<KeyframeRule> &&keyframe) noexcept;
    void WrapperAppendKeyframe(Ref<KeyframeRule> &&keyframe) noexcept;
    void WrapperRemoveKeyframe(uint32 index) noexcept;

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    void SetName(const CSSOMStringAtom &name) noexcept
    {
      _name = name;
    }

    /// @return The index of the last keyframe whose keys are those parsed from @p key, or nothing if there
    /// is none or @p key isn't a valid keyframe selector list.
    KRYS_NODISCARD Maybe<size_t> FindKeyframeIndex(const CSSOMString &key) const noexcept;

    void ShrinkToFit() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::KeyframesRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsKeyframesRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
