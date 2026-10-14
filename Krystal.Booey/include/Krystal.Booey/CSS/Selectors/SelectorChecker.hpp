#pragma once

#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class QualifiedName;
}

namespace krys::boo::style
{
  struct SelectorMatchingState;
}

namespace krys::boo::css
{
  class SelectorList;
  class ScopeRule;

  /// @brief Matches selectors against elements.
  class SelectorChecker
  {
    KRYS_NON_COPYABLE(SelectorChecker);

    enum class MatchOutcome
    {
      SelectorMatches,
      SelectorFailsLocally,
      SelectorFailsAllSiblings,
      SelectorFailsCompletely
    };

    enum class MatchType
    {
      VirtualPseudoElementOnly,
      Element
    };

    struct MatchResult
    {
      MatchOutcome Outcome;
      MatchType Type;

      KRYS_NODISCARD static MatchResult Matches(MatchType matchType) noexcept
      {
        return {MatchOutcome::SelectorMatches, matchType};
      }

      KRYS_NODISCARD static MatchResult UpdateWithMatchType(MatchResult result, MatchType matchType) noexcept
      {
        if (matchType == MatchType::VirtualPseudoElementOnly)
        {
          result.Type = MatchType::VirtualPseudoElementOnly;
        }
        return result;
      }

      KRYS_NODISCARD static MatchResult Fails(MatchOutcome outcome) noexcept
      {
        return {outcome, MatchType::Element};
      }
    };

  public:
    enum class Mode : uint8
    {
      ResolvingStyle = 0,
      CollectingRules,
      StyleInvalidation,
      // This is used for querySelector() API
      QueryingRules
    };

    struct CheckingContext
    {
      const Mode ResolvingMode;

    private:
      Maybe<style::PseudoElementIdentifier> _requestedPseudoElement;

    public:
      List<CSSOMStringAtom> ClassList;
      RefPtr<const dom::ContainerNode> Scope;
      const dom::Element *HasScope {nullptr};
      bool MatchesAllHasScopes {false};
      bool IsEvaluatingScopingRoot {false};
      style::ScopeOrdinal StyleScopeOrdinal {style::ScopeOrdinal::Element};
      style::SelectorMatchingState *SelectorMatchingState {nullptr};

      // FIXME: It would be nicer to have a separate object for return values.
      style::Relations StyleRelations;
      EnumSet<PseudoElementType> PublicPseudoElements;
      bool MatchedInsideScope {false};
      bool DisallowHasPseudoClass {false};
      bool ScopingRootMatchesVisited {false};

      explicit CheckingContext(Mode resolvingMode) noexcept : ResolvingMode(resolvingMode)
      {
      }

      void SetRequestedPseudoElement(style::PseudoElementIdentifier pseudoElementIdentifier) noexcept
      {
        _requestedPseudoElement = krys::move(pseudoElementIdentifier);
      }

      KRYS_NODISCARD const Maybe<style::PseudoElementIdentifier> &RequestedPseudoElement() const noexcept
      {
        return _requestedPseudoElement;
      }
    };

    enum LinkMatchMask : uint32
    {
      MatchDefault = 0,
      MatchLink = 1,
      MatchVisited = 2,
      MatchAll = MatchLink | MatchVisited
    };

    struct LocalContext;

  private:
    bool _strictParsing;
    bool _documentIsHTML;

  public:
    explicit SelectorChecker(const dom::Document &document) noexcept;

    KRYS_NODISCARD bool Match(const Selector &selector, const dom::Element &element,
                              CheckingContext &checkingContext) const noexcept;

    KRYS_NODISCARD bool MatchHostPseudoClass(const Selector &selector, const dom::Element &element,
                                             CheckingContext &checkingContext) const noexcept;

    KRYS_NODISCARD static bool IsCommonPseudoClassSelector(const Selector *selector) noexcept;
    KRYS_NODISCARD static bool AttributeSelectorMatches(const dom::Element &element,
                                                        const dom::QualifiedName &attributeName,
                                                        const CSSOMStringAtom &attributeValue,
                                                        const Selector &selector) noexcept;

    KRYS_NODISCARD static uint32 DetermineLinkMatchType(const Selector *selector,
                                                        const ScopeRule *scopeRule = nullptr) noexcept;

  private:
    KRYS_NODISCARD MatchResult
      MatchRecursively(CheckingContext &checkingContext, LocalContext &context,
                       EnumSet<PseudoElementType> &collectedPseudoElements) const noexcept;
    KRYS_NODISCARD bool CheckOne(CheckingContext &checkingContext, LocalContext &context,
                                 MatchType &matchType) const noexcept;
    KRYS_NODISCARD bool MatchSelectorList(CheckingContext &checkingContext, const LocalContext &context,
                                          const dom::Element &element,
                                          const SelectorList &selectorList) const noexcept;
    KRYS_NODISCARD bool MatchHasPseudoClass(CheckingContext &checkingContext, const dom::Element &element,
                                            const Selector &hasSelector) const noexcept;

    KRYS_NODISCARD bool CheckViewTransitionPseudoClass(const CheckingContext &checkingContext,
                                                       const dom::Element &element,
                                                       const Selector &selector) const noexcept;
  };

  inline bool SelectorChecker::IsCommonPseudoClassSelector(const Selector *selector) noexcept
  {
    if (selector->Match() != SelectorMatch::PseudoClass)
    {
      return false;
    }
    auto pseudoType = selector->PseudoClass();
    return pseudoType == SelectorPseudoClass::Link || pseudoType == SelectorPseudoClass::AnyLink
           || pseudoType == SelectorPseudoClass::Visited || pseudoType == SelectorPseudoClass::Focus;
  }
}
