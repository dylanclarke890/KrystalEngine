#pragma once

#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorFilter.hpp"
#include "Krystal.Booey/Style/PropertyAllowlist.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/CompactRefPtrTuple.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::style
{
  enum class MatchBasedOnRuleHash : uint32
  {
    None,
    Universal,
    ClassA,
    ClassB,
    ClassC
  };

  enum class UsedRuleType : uint8
  {
    StartingStyle,
  };

  /// @brief One complex selector of a style rule, as added to a rule set, with what matching needs to know
  /// about it up front.
  class RuleData
  {
  public:
    static constexpr uint32 MaximumSelectorComponentCount = 8'192;

  private:
    // Keep in sync with RuleFeature's selectorIndex and selectorListIndex size.
    CompactRefPtrTuple<const css::StyleRule, uint16> _styleRuleWithSelectorIndex;
    uint32 _selectorListIndex : 16;
    uint32 _matchBasedOnRuleHash : 3;
    uint32 _canMatchPseudoElement : 1;
    uint32 _linkMatchType : 2; // SelectorChecker::LinkMatchMask
    uint32 _propertyAllowlist : 3;
    uint32 _usedRuleTypes : 1;
    uint32 _isEnabled : 1;
    // If we have more rules than 2^bitcount here we'll get confused about rule order.
    uint32 _position : 21;
    css::SelectorFilter::Hashes _descendantSelectorIdentifierHashes;

  public:
    RuleData(const css::StyleRule &styleRule, uint32 selectorIndex, uint32 selectorListIndex, uint32 position,
             EnumSet<UsedRuleType> usedRuleTypes) noexcept;

    KRYS_NODISCARD uint32 Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD const css::StyleRule &StyleRule() const noexcept
    {
      return *_styleRuleWithSelectorIndex.Ptr();
    }

    KRYS_NODISCARD const css::Selector *Selector() const noexcept
    {
      return StyleRule().GetSelectorList().SelectorComponentAt(SelectorIndex());
    }

    KRYS_NODISCARD uint32 SelectorIndex() const noexcept
    {
      return _styleRuleWithSelectorIndex.Data();
    }

    KRYS_NODISCARD uint32 SelectorListIndex() const noexcept
    {
      return _selectorListIndex;
    }

    KRYS_NODISCARD bool CanMatchPseudoElement() const noexcept
    {
      return _canMatchPseudoElement;
    }

    KRYS_NODISCARD MatchBasedOnRuleHash GetMatchBasedOnRuleHash() const noexcept
    {
      return static_cast<MatchBasedOnRuleHash>(_matchBasedOnRuleHash);
    }

    KRYS_NODISCARD uint32 LinkMatchType() const noexcept
    {
      return _linkMatchType;
    }

    void SetLinkMatchType(uint32 value) noexcept
    {
      _linkMatchType = value;
    }

    KRYS_NODISCARD style::PropertyAllowlist GetPropertyAllowlist() const noexcept
    {
      return static_cast<style::PropertyAllowlist>(_propertyAllowlist);
    }

    KRYS_NODISCARD EnumSet<UsedRuleType> UsedRuleTypes() const noexcept
    {
      return EnumSet<UsedRuleType>::FromRaw(_usedRuleTypes);
    }

    KRYS_NODISCARD bool IsEnabled() const noexcept
    {
      return _isEnabled;
    }

    void SetEnabled(bool value) noexcept
    {
      _isEnabled = value;
    }

    KRYS_NODISCARD const css::SelectorFilter::Hashes &DescendantSelectorIdentifierHashes() const noexcept
    {
      return _descendantSelectorIdentifierHashes;
    }

    void DisableSelectorFiltering() noexcept
    {
      _descendantSelectorIdentifierHashes[0] = 0;
    }
  };
}
