#pragma once

#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <tuple>

namespace krys::boo::css
{
  class Selector;
}

namespace krys::boo::style
{
  class RuleData;

  // MatchElement characterizes which elements a change in an element matched by a simple selector (as a part
  // of a complex selector) may affect. Style::Invalidator uses these classifications to traverse a minimal
  // number of elements after a DOM mutation. In the examples below the '.changed' simple selector will be
  // classified with the given enum value.
  // FIXME: Has* values should be separated so we could better describe both the :has() argument and its
  // position in the selector.
  enum class MatchElement : uint8
  {
    Subject,              // .changed
    Parent,               // .changed > .subject
    Ancestor,             // .changed .subject
    DirectSibling,        // .changed + .subject
    IndirectSibling,      // .changed ~ .subject
    AnySibling,           // :nth-last-child(even of .changed)
    ParentSibling,        // .changed ~ .a > .subject
    AncestorSibling,      // .changed ~ .a .subject
    ParentAnySibling,     // :nth-last-child(even of .changed) > .subject
    AncestorAnySibling,   // :nth-last-child(even of .changed) .subject
    HasChild,             // :has(> .changed)
    HasDescendant,        // :has(.changed)
    HasSibling,           // :has(~ .changed)
    HasSiblingDescendant, // :has(~ .a .changed)
    HasAnySibling,        // :has(~ :is(.changed ~ .x))
    HasChildParent,       // :has(> .changed) > .subject
    HasChildAncestor,     // :has(> .changed) .subject
    HasDescendantParent,  // :has(.changed) > .subject
    // FIXME: This is a catch-all for the rest of cases where :has() is in a non-subject position.
    HasNonSubject, // :has(.changed) .subject
    // FIXME: This is a catch-all for cases where :has() contains a scope breaking sub-selector.
    HasScopeBreaking, // :has(:is(.changed .a))
    Host,             // :host(.changed) .subject
    HostChild         // ::slotted(.changed)
  };

  constexpr uint32 MatchElementCount = static_cast<uint32>(MatchElement::HostChild) + 1;

  enum class IsNegation : bool
  {
    No,
    Yes
  };

  // Are we inside a logical combination pseudo-class like :is() or :not(), which if we were inside a :has(),
  // could break out of its scope?
  enum class CanBreakScope : bool
  {
    No,
    Yes
  };

  // Did we find a logical combination pseudo-class like :is() or :not() with selector combinators that do
  // break out of a :has() scope?
  enum class DoesBreakScope : bool
  {
    No,
    Yes
  };

// For MSVC.
#pragma pack(push, 4)
  /// @brief A complex selector of a style rule, by its index in the rule's selector list.
  struct RuleAndSelector
  {
    RefPtr<const css::StyleRule> StyleRule;
    uint16 SelectorIndex;     // Keep in sync with RuleData's selectorIndex size.
    uint16 SelectorListIndex; // Keep in sync with RuleData's selectorListIndex size.

    explicit RuleAndSelector(const RuleData &ruleData) noexcept;

    KRYS_NODISCARD const css::Selector &Selector() const noexcept;
  };

  struct RuleFeature : public RuleAndSelector
  {
    style::MatchElement MatchElement;
    style::IsNegation IsNegation; // Whether the selector is in a (non-paired) :not() context.

    RuleFeature(const RuleData &ruleData, style::MatchElement matchElement,
                style::IsNegation isNegation) noexcept;
  };

  static_assert(sizeof(RuleFeature) <= 16, "RuleFeature is a frequently allocated object. Keep it small.");

  struct RuleFeatureWithInvalidationSelector : public RuleFeature
  {
    css::SelectorList InvalidationSelector;

    RuleFeatureWithInvalidationSelector(const RuleData &ruleData, style::MatchElement matchElement,
                                        style::IsNegation isNegation,
                                        css::SelectorList &&invalidationSelector) noexcept;
  };
#pragma pack(pop)

  using PseudoClassInvalidationKey = std::tuple<uint32, uint8, css::CSSOMStringAtom>;

  struct PseudoClassInvalidationKeyHash
  {
    KRYS_NODISCARD size_t operator()(const PseudoClassInvalidationKey &key) const noexcept
    {
      return Hash::Combine(std::get<0>(key), std::get<1>(key))
             ^ std::hash<css::CSSOMStringAtom> {}(std::get<2>(key));
    }
  };

  using RuleFeatureVector = List<RuleFeature>;

  /// @brief A selector compared and hashed ignoring pseudo-elements that aren't element-backed: selectors
  /// like
  /// `.foo` and `.foo::before` are the same for invalidation.
  struct SelectorDeduplicationKey
  {
    const css::Selector *Selector;
    size_t CachedHash;

    explicit SelectorDeduplicationKey(const css::Selector &selector) noexcept;

    KRYS_NODISCARD bool operator==(const SelectorDeduplicationKey &other) const noexcept;
  };

  struct SelectorDeduplicationKeyHash
  {
    KRYS_NODISCARD size_t operator()(const SelectorDeduplicationKey &key) const noexcept
    {
      return key.CachedHash;
    }
  };

  /// @brief The selector features of a rule set that style invalidation needs.
  struct RuleFeatureSet
  {
    struct CollectionContext
    {
      HashSet<SelectorDeduplicationKey, SelectorDeduplicationKeyHash> SelectorDeduplicationSet;
    };

    template <typename T>
    using AtomMap = HashMap<css::CSSOMStringAtom, UniquePtr<T>>;
    template <typename T>
    using PseudoClassMap = HashMap<PseudoClassInvalidationKey, UniquePtr<T>, PseudoClassInvalidationKeyHash>;

    HashSet<css::CSSOMStringAtom> IdsInRules;
    HashSet<css::CSSOMStringAtom> IdsMatchingAncestorsInRules;
    HashSet<css::CSSOMStringAtom> AttributeLowercaseLocalNamesInRules;
    HashSet<css::CSSOMStringAtom> AttributeLocalNamesInRules;
    HashSet<css::CSSOMStringAtom> ContentAttributeNamesInRules;
    AtomMap<RuleFeatureVector> IdRules;
    AtomMap<RuleFeatureVector> ClassRules;
    AtomMap<List<RuleFeatureWithInvalidationSelector>> AttributeRules;
    PseudoClassMap<RuleFeatureVector> PseudoClassRules;
    PseudoClassMap<List<RuleFeatureWithInvalidationSelector>> HasPseudoClassRules;
    List<RuleAndSelector> ScopeBreakingHasPseudoClassRules;
    HashSet<css::CSSOMStringAtom> ClassesAffectingHost;
    HashSet<css::CSSOMStringAtom> AttributesAffectingHost;
    HashSet<css::SelectorPseudoClass> PseudoClassesAffectingHost;
    HashSet<css::SelectorPseudoClass> PseudoClasses;
    Array<bool, MatchElementCount> UsedMatchElements {};
    bool UsesFirstLineRules {false};
    bool UsesFirstLetterRules {false};
    bool HasStartingStyleRules {false};

  private:
    struct SelectorFeatures
    {
      using InvalidationFeature = std::tuple<const css::Selector *, style::MatchElement, style::IsNegation>;
      using HasInvalidationFeature =
        std::tuple<const css::Selector *, style::MatchElement, style::IsNegation, style::DoesBreakScope>;

      List<InvalidationFeature> Ids;
      List<InvalidationFeature> Classes;
      List<InvalidationFeature> Attributes;
      List<InvalidationFeature> PseudoClasses;
      List<HasInvalidationFeature> HasPseudoClasses;
    };

  public:
    void Add(const RuleFeatureSet &other) noexcept;
    void Clear() noexcept;
    void ShrinkToFit() noexcept;

    void CollectFeatures(CollectionContext &collectionContext, const RuleData &ruleData,
                         const List<Ref<const css::ScopeRule>> &scopeRules = {}) noexcept;
    void RegisterContentAttribute(const css::CSSOMStringAtom &attributeName) noexcept;

    KRYS_NODISCARD bool UsesHasPseudoClass() const noexcept;

    KRYS_NODISCARD bool UsesMatchElement(style::MatchElement matchElement) const noexcept
    {
      return UsedMatchElements[ToUnderlying(matchElement)];
    }

    void SetUsesMatchElement(style::MatchElement matchElement) noexcept
    {
      UsedMatchElements[ToUnderlying(matchElement)] = true;
    }

  private:
    style::DoesBreakScope RecursivelyCollectFeaturesFromSelector(
      SelectorFeatures &selectorFeatures, const css::Selector &firstSelector,
      style::MatchElement matchElement = style::MatchElement::Subject,
      style::IsNegation isNegation = style::IsNegation::No,
      style::CanBreakScope allComponentsCanBreakScope = style::CanBreakScope::No) noexcept;
    void CollectPseudoElementFeatures(const RuleData &ruleData) noexcept;
  };

  KRYS_NODISCARD bool IsHasPseudoClassMatchElement(MatchElement matchElement) noexcept;

  /// @brief Which elements, relative to the `:has()` subject, the relative selector @p hasSelector can match.
  KRYS_NODISCARD MatchElement ComputeHasPseudoClassMatchElement(const css::Selector &hasSelector) noexcept;

  enum class InvalidationKeyType : uint8
  {
    Universal = 1,
    Class,
    Id,
    Attribute,
    Tag
  };

  KRYS_NODISCARD PseudoClassInvalidationKey
    MakePseudoClassInvalidationKey(css::SelectorPseudoClass pseudoClass, InvalidationKeyType keyType,
                                   const css::CSSOMStringAtom &keyString) noexcept;
  KRYS_NODISCARD PseudoClassInvalidationKey MakePseudoClassInvalidationKey(
    css::SelectorPseudoClass pseudoClass, InvalidationKeyType keyType) noexcept;

  KRYS_NODISCARD bool UnlikelyToHaveSelectorForAttribute(const css::CSSOMStringAtom &name) noexcept;

  KRYS_NODISCARD inline bool IsUniversalInvalidation(const PseudoClassInvalidationKey &key) noexcept
  {
    return static_cast<InvalidationKeyType>(std::get<1>(key)) == InvalidationKeyType::Universal;
  }

  inline bool RuleFeatureSet::UsesHasPseudoClass() const noexcept
  {
    return UsesMatchElement(style::MatchElement::HasChild)
           || UsesMatchElement(style::MatchElement::HasDescendant)
           || UsesMatchElement(style::MatchElement::HasSibling)
           || UsesMatchElement(style::MatchElement::HasSiblingDescendant)
           || UsesMatchElement(style::MatchElement::HasAnySibling)
           || UsesMatchElement(style::MatchElement::HasNonSubject)
           || UsesMatchElement(style::MatchElement::HasScopeBreaking);
  }
}
