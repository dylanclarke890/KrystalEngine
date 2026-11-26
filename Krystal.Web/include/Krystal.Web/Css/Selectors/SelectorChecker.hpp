#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Css/Selectors/Selector.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Web/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Web/Style/Relations.hpp"
#include "Krystal.Web/Style/Scope/ScopeOrdinal.hpp"

namespace krys::web
{
  class ContainerNode;
  class Document;
  class Element;
  class QualifiedName;
}

namespace krys::web::style
{
  struct SelectorMatchingState;
}

namespace krys::web::css
{
  class ScopeRule;
  class SelectorList;

  /// @brief Decides whether an element
  /// [matches](https://drafts.csswg.org/selectors/#match-a-complex-selector-against-an-element) a complex
  /// selector, checking it right to left and moving to the ancestors or siblings each combinator asks for.
  /// When resolving style, it also records how the element's style depends on others, for invalidation.
  class SelectorChecker
  {
    KRYS_NON_COPYABLE(SelectorChecker);

    /// @brief How a selector failed, which tells a combinator's search how far to keep looking.
    enum class MatchOutcome
    {
      SelectorMatches,
      /// @brief It fails for this element.
      SelectorFailsLocally,
      /// @brief It fails for this element and its siblings, so a sibling combinator can stop.
      SelectorFailsAllSiblings,
      /// @brief It fails for this element, its siblings and its ancestors, so any search can stop.
      SelectorFailsCompletely,
    };

    enum class MatchType
    {
      /// @brief Only one of the element's pseudo-elements matches, not the element itself.
      VirtualPseudoElementOnly,
      Element,
    };

    struct MatchResult
    {
      MatchOutcome Outcome;
      MatchType Type;

      KRYS_NODISCARD static MatchResult Matches(MatchType matchType) noexcept
      {
        return {MatchOutcome::SelectorMatches, matchType};
      }

      KRYS_NODISCARD static MatchResult Fails(MatchOutcome outcome) noexcept
      {
        return {outcome, MatchType::Element};
      }

      /// @brief @p result, made pseudo-element-only if @p matchType is.
      KRYS_NODISCARD static MatchResult UpdateWithMatchType(MatchResult result, MatchType matchType) noexcept
      {
        if (matchType == MatchType::VirtualPseudoElementOnly)
        {
          result.Type = MatchType::VirtualPseudoElementOnly;
        }
        return result;
      }
    };

  public:
    /// @brief Why selectors are being matched, which decides what's recorded and what can match.
    enum class Mode : uint8
    {
      /// @brief Resolving an element's style, recording its style relations.
      ResolvingStyle = 0,
      /// @brief Collecting the rules that match an element or one of its pseudo-elements, rather than
      /// resolving its style.
      CollectingRules,
      /// @brief Checking whether a change affects the element's style.
      StyleInvalidation,
      /// @brief A query such as `querySelector()` or `matches()`, which pseudo-elements never match.
      QueryingRules,
    };

    /// @brief The inputs of a match, and what it records.
    struct CheckingContext
    {
      const Mode ResolvingMode;

    private:
      Maybe<style::PseudoElementIdentifier> _requestedPseudoElement;

    public:
      List<CSSOMStringAtom> ClassList;
      /// @brief What `:scope` matches: the scoping root of a query, or of an `@scope` rule.
      RefPtr<const ContainerNode> Scope;
      /// @brief The element a relative selector in `:has()` is anchored to.
      const Element *HasScope {nullptr};
      /// @brief Whether the `:has()` anchor matches any element, as when checking whether a child change
      /// affects a `:has()`.
      bool MatchesAllHasScopes {false};
      /// @brief Which tree, relative to the element's, the rule being matched came from.
      style::ScopeOrdinal StyleScopeOrdinal {style::ScopeOrdinal::Element};
      /// @brief Caches of `:has()` results shared across matches, if any.
      style::SelectorMatchingState *SelectorMatchingState {nullptr};

      // FIXME: The outputs below would be clearer in a separate object from the inputs above.

      /// @brief How the element's style depends on other elements', recorded when resolving style.
      style::Relations StyleRelations;
      /// @brief The pseudo-elements a match against the element found, which style resolution then resolves.
      EnumSet<PseudoElementType> PublicPseudoElements;
      /// @brief Whether a `:has()` anchor was reached.
      bool MatchedInsideScope {false};
      /// @brief Whether `:has()` must fail, as it does inside another `:has()`'s arguments.
      bool DisallowHasPseudoClass {false};

      explicit CheckingContext(Mode resolvingMode) noexcept : ResolvingMode(resolvingMode)
      {
      }

      /// @brief Matches against the element's @p pseudoElementIdentifier rather than the element.
      void SetRequestedPseudoElement(style::PseudoElementIdentifier pseudoElementIdentifier) noexcept
      {
        _requestedPseudoElement = krys::move(pseudoElementIdentifier);
      }

      KRYS_NODISCARD const Maybe<style::PseudoElementIdentifier> &RequestedPseudoElement() const noexcept
      {
        return _requestedPseudoElement;
      }
    };

    struct LocalContext;

  private:
    bool _strictParsing;
    bool _documentIsHTML;

  public:
    explicit SelectorChecker(const Document &document) noexcept;

    /// @brief Whether @p element matches the complex selector whose rightmost simple selector is @p selector.
    KRYS_NODISCARD bool Match(const Selector &selector, const Element &element,
                              CheckingContext &checkingContext) const noexcept;

    /// @brief Whether @p selector is `:link`, `:any-link`, `:visited` or `:focus`.
    KRYS_NODISCARD static bool IsCommonPseudoClassSelector(const Selector *selector) noexcept;

    /// @brief Whether attribute selector @p selector matches @p element's attribute @p attributeName with
    /// value @p attributeValue.
    KRYS_NODISCARD static bool AttributeSelectorMatches(const Element &element,
                                                        const QualifiedName &attributeName,
                                                        const CSSOMStringAtom &attributeValue,
                                                        const Selector &selector) noexcept;

  private:
    /// @brief Whether @p element is a shadow host that matches `:host` or `:host(<compound>)` @p selector.
    KRYS_NODISCARD bool MatchHostPseudoClass(const Selector &selector, const Element &element,
                                             CheckingContext &checkingContext) const noexcept;

    /// @brief Checks @p context's simple selector against its element, then the rest of the complex
    /// selector, following its combinators.
    /// @param collectedPseudoElements Gains the pseudo-elements the rightmost compound matched.
    KRYS_NODISCARD MatchResult
      MatchRecursively(CheckingContext &checkingContext, LocalContext &context,
                       EnumSet<PseudoElementType> &collectedPseudoElements) const noexcept;

    /// @brief Whether @p context's simple selector matches its element.
    /// @param matchType Set to pseudo-element-only if the selector only matched a pseudo-element.
    KRYS_NODISCARD bool CheckOne(CheckingContext &checkingContext, LocalContext &context,
                                 MatchType &matchType) const noexcept;

    /// @brief Whether @p context's pseudo-class matches its element.
    KRYS_NODISCARD bool CheckPseudoClass(CheckingContext &checkingContext, LocalContext &context,
                                         MatchType &matchType) const noexcept;

    /// @brief Whether @p context's pseudo-element, or its arguments, match its element.
    KRYS_NODISCARD bool CheckPseudoElement(CheckingContext &checkingContext,
                                           LocalContext &context) const noexcept;

    /// @brief Whether @p element matches any of @p selectorList, a functional pseudo-class's arguments.
    KRYS_NODISCARD bool MatchSelectorList(CheckingContext &checkingContext, const LocalContext &context,
                                          const Element &element,
                                          const SelectorList &selectorList) const noexcept;

    /// @brief Whether @p element matches `:has()` @p hasSelector.
    KRYS_NODISCARD bool MatchHasPseudoClass(CheckingContext &checkingContext, const Element &element,
                                            const Selector &hasSelector) const noexcept;
  };

  inline bool SelectorChecker::IsCommonPseudoClassSelector(const Selector *selector) noexcept
  {
    if (selector->Match() != SelectorMatch::PseudoClass)
    {
      return false;
    }
    switch (selector->PseudoClass())
    {
      case SelectorPseudoClass::Link:
      case SelectorPseudoClass::AnyLink:
      case SelectorPseudoClass::Visited:
      case SelectorPseudoClass::Focus:   return true;
      default:                           return false;
    }
  }
}
