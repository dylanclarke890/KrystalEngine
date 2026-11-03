#pragma once

#include "Krystal.Booey/DOM/Internals/PopoverData.hpp"
#include "Krystal.Booey/DOM/Nodes/TextDirectionState.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/NTTPString.hpp"
#include <algorithm>

namespace krys::boo::html
{
#pragma region States

  /// @see https://html.spec.whatwg.org/#attr-contenteditable
  enum class ContentEditableAttributeState : uint8
  {
    True,
    False,
    PlaintextOnly,
    Inherit,
  };

  /// @see https://html.spec.whatwg.org/#attr-translate
  enum class TranslateAttributeState : uint8
  {
    Yes,
    No,
    Inherit,
  };

  /// @see https://html.spec.whatwg.org/#attr-hidden
  enum class HiddenAttributeState : uint8
  {
    Hidden,
    UntilFound,
    NotHidden,
  };

  /// @see https://html.spec.whatwg.org/#attr-draggable
  enum class DraggableAttributeState : uint8
  {
    True,
    False,
    Auto,
  };

  /// @see https://html.spec.whatwg.org/#attr-spellcheck
  enum class SpellcheckAttributeState : uint8
  {
    True,
    False,
    Default,
  };

  /// @see https://html.spec.whatwg.org/#attr-writingsuggestions
  enum class WritingSuggestionsAttributeState : uint8
  {
    True,
    False,
    Default,
  };

  /// @see https://html.spec.whatwg.org/#attr-autocapitalize
  enum class AutocapitalizeAttributeState : uint8
  {
    Default,
    None,
    Sentences,
    Words,
    Characters,
  };

  /// @see https://html.spec.whatwg.org/#attr-autocorrect
  enum class AutocorrectAttributeState : uint8
  {
    On,
    Off,
  };

  /// @see https://html.spec.whatwg.org/#attr-inputmode
  enum class InputModeAttributeState : uint8
  {
    None,
    Text,
    Tel,
    URL,
    Email,
    Numeric,
    Decimal,
    Search,
  };

  /// @see https://html.spec.whatwg.org/#attr-enterkeyhint
  enum class EnterKeyHintAttributeState : uint8
  {
    Enter,
    Done,
    Go,
    Next,
    Previous,
    Search,
    Send,
  };

  /// @see https://html.spec.whatwg.org/#lazy-loading-attribute
  enum class LoadingAttributeState : uint8
  {
    Lazy,
    Eager,
  };

  /// @brief The states of a form's `method` and a submit button's `formmethod`.
  /// @see https://html.spec.whatwg.org/#attr-fs-method
  enum class MethodAttributeState : uint8
  {
    Get,
    Post,
    Dialog,
  };

  /// @brief The states of a form's `enctype` and a submit button's `formenctype`.
  /// @see https://html.spec.whatwg.org/#attr-fs-enctype
  enum class EnctypeAttributeState : uint8
  {
    URLEncoded,
    FormData,
    TextPlain,
  };

  /// @see https://html.spec.whatwg.org/#fetch-priority-attribute
  enum class FetchPriorityAttributeState : uint8
  {
    High,
    Low,
    Auto,
  };

  /// @see https://html.spec.whatwg.org/#cors-settings-attribute
  enum class CrossOriginAttributeState : uint8
  {
    NoCORS,
    Anonymous,
    UseCredentials,
  };

  /// @see https://html.spec.whatwg.org/#referrer-policy-attribute
  enum class ReferrerPolicyAttributeState : uint8
  {
    /// @brief The empty string.
    Empty,
    NoReferrer,
    NoReferrerWhenDowngrade,
    SameOrigin,
    Origin,
    StrictOrigin,
    OriginWhenCrossOrigin,
    StrictOriginWhenCrossOrigin,
    UnsafeURL,
  };

  /// @see https://html.spec.whatwg.org/#attr-dialog-closedby
  enum class ClosedByAttributeState : uint8
  {
    Any,
    CloseRequest,
    None,
    Auto,
  };

  /// @see https://html.spec.whatwg.org/#attr-popovertargetaction
  enum class PopoverTargetActionAttributeState : uint8
  {
    Toggle,
    Show,
    Hide,
  };

  /// @brief The states of a form's `autocomplete`.
  /// @see https://html.spec.whatwg.org/#attr-form-autocomplete
  enum class AutocompleteAttributeState : uint8
  {
    On,
    Off,
  };

  /// @see https://html.spec.whatwg.org/#attr-img-decoding
  enum class DecodingAttributeState : uint8
  {
    Sync,
    Async,
    Auto,
  };

  /// @brief The states of a table cell's `scope`.
  /// @see https://html.spec.whatwg.org/#attr-th-scope
  enum class ScopeAttributeState : uint8
  {
    Row,
    Col,
    RowGroup,
    ColGroup,
    Auto,
  };

  /// @see https://html.spec.whatwg.org/#attr-template-shadowrootmode
  enum class ShadowRootModeAttributeState : uint8
  {
    Open,
    Closed,
    None,
  };

  /// @see https://html.spec.whatwg.org/#attr-template-shadowrootslotassignment
  enum class ShadowRootSlotAssignmentAttributeState : uint8
  {
    Named,
    Manual,
  };

  /// @brief The states of a track's `kind`.
  /// @see https://html.spec.whatwg.org/#attr-track-kind
  enum class KindAttributeState : uint8
  {
    Subtitles,
    Captions,
    Descriptions,
    Chapters,
    Metadata,
  };

  /// @brief The states of a link's `as`: one for each potential destination.
  /// @see https://html.spec.whatwg.org/#attr-link-as
  enum class AsAttributeState : uint8
  {
    Fetch,
    Audio,
    AudioWorklet,
    Document,
    Embed,
    Font,
    Frame,
    IFrame,
    Image,
    JSON,
    Manifest,
    Object,
    PaintWorklet,
    Report,
    Script,
    ServiceWorker,
    SharedWorker,
    Style,
    Track,
    Video,
    WebIdentity,
    Worker,
    XSLT,
  };

#pragma endregion

  /// @brief A keyword of an enumerated attribute and the state it maps to.
  template <IsEnum TState>
  struct KeywordStateMapping
  {
    dom::DOMStringAtom Keyword = dom::DOMStringAtom::Null();
    TState State;
  };

  /// @brief Traits struct that must be specialized for each enumerated attribute on each element it applies
  /// to (or just for `HTMLElement` in the case of generic enumerated attributes).
  /// @code
  /// // Required: the attribute's states, and its keywords with the canonical keyword of each state first.
  /// using State = dom::TextDirectionState;
  /// static inline const KeywordStateMapping<State> StateMappings[] = {
  ///   {u8"ltr", State::LTR}, {u8"rtl", State::RTL}, {u8"auto", State::Auto}};
  ///
  /// // As needed:
  /// constexpr static State MissingValueDefault = State::RTL;
  /// constexpr static State InvalidValueDefault = State::LTR;
  /// constexpr static State EmptyValueDefault = State::Auto;
  /// @endcode
  template <NTTPString AttributeName, DerivedFrom<HTMLElement> TElement>
  struct EnumeratedAttributeTraits;

  template <typename Traits>
  concept HasStateMappings = requires {
    typename Traits::State;
    Traits::StateMappings;
  };

  template <typename Traits>
  concept HasMissingValueDefault = requires { Traits::MissingValueDefault; };

  template <typename Traits>
  concept HasInvalidValueDefault = requires { Traits::InvalidValueDefault; };

  template <typename Traits>
  concept HasEmptyValueDefault = requires { Traits::EmptyValueDefault; };

  /// @brief Whether an attribute can be in no state: when it's missing without a missing value default, or
  /// invalid without an invalid value default.
  template <typename Traits>
  concept CanReturnNullResolvedState = !HasMissingValueDefault<Traits> || !HasInvalidValueDefault<Traits>;

  /// @see https://html.spec.whatwg.org/#enumerated-attribute
  template <NTTPString AttributeName, DerivedFrom<HTMLElement> TElement>
  class EnumeratedAttribute
  {
    using Traits = EnumeratedAttributeTraits<AttributeName, TElement>;

    static_assert(HasStateMappings<Traits>, "Must define 'EnumeratedAttributeTraits' with 'State' and "
                                            "'StateMappings' for this AttributeName/TElement.");

  public:
    using State = Traits::State;

    /// @brief The canonical keyword of the state of an attribute whose value is @p contentAttributeValue
    /// (`ReflectEnumerated`'s step 4). Without one, it's null for a `DOMString?` and the empty string for a
    /// `DOMString`.
    /// @see https://html.spec.whatwg.org/#limited-to-only-known-values
    template <OneOf<dom::DOMString, Maybe<dom::DOMString>> TResult>
    KRYS_NODISCARD static TResult
      ResolveCanonicalKeyword(Maybe<dom::DOMStringView> contentAttributeValue) noexcept
    {
      Maybe<State> state = ResolveState(contentAttributeValue);
      auto keyword = state ? ResolveCanonicalKeywordForState(*state) : dom::DOMStringAtom::Null();
      if (keyword == dom::DOMStringAtom::Null())
      {
        return TResult {};
      }
      return TResult(keyword.View());
    }

    /// @brief The state of an attribute whose value is @p contentAttributeValue (null when it's not
    /// specified); no state is null.
    /// @see https://html.spec.whatwg.org/#empty-value-default
    KRYS_NODISCARD static conditional_t<CanReturnNullResolvedState<Traits>, Maybe<State>, State>
      ResolveState(Maybe<dom::DOMStringView> contentAttributeValue) noexcept
    {
      // 1. If the attribute is not specified:
      if (contentAttributeValue == null)
      {
        // 1a. If the attribute has a missing value default state defined, then return that missing value
        //     default state.
        if constexpr (HasMissingValueDefault<Traits>)
        {
          return Traits::MissingValueDefault;
        }
        // 1b. Otherwise, return no state.
        else
        {
          return null;
        }
      }

      auto value = *contentAttributeValue;

      // 2. If the attribute's value is an ASCII case-insensitive match for one of the keywords defined for
      //    the attribute, then return the state represented by that keyword.
      auto it = std::ranges::find_if(
        Traits::StateMappings, [&](auto &mapping)
        { return infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, mapping.Keyword.View()); });
      if (it != std::ranges::end(Traits::StateMappings))
      {
        return it->State;
      }

      // 3. If the attribute has an empty value default state defined and the attribute's value is the empty
      //    string, then return that empty value default state.
      if constexpr (HasEmptyValueDefault<Traits>)
      {
        if (value.empty())
        {
          return Traits::EmptyValueDefault;
        }
      }

      // 4. If the attribute has an invalid value default state defined, then return that invalid value
      //    default state.
      if constexpr (HasInvalidValueDefault<Traits>)
      {
        return Traits::InvalidValueDefault;
      }
      // 5. Return no state.
      else
      {
        return null;
      }
    }

    /// @brief The canonical keyword for @p state: the first keyword mapping to it, or null when none does.
    /// @note The canonical keyword is the only keyword for the state, the only conforming one, the
    /// conforming one that isn't empty (where an empty keyword is an empty value default here), or the one
    /// the specification gives explicitly, so `StateMappings` lists it first.
    /// @see https://html.spec.whatwg.org/#canonical-keyword
    KRYS_NODISCARD static dom::DOMStringAtom ResolveCanonicalKeywordForState(State state) noexcept
    {
      auto it =
        std::ranges::find_if(Traits::StateMappings, [&](auto &mapping) { return mapping.State == state; });
      return it != std::ranges::end(Traits::StateMappings) ? it->Keyword : dom::DOMStringAtom::Null();
    }
  };

  template <>
  struct EnumeratedAttributeTraits<"dir", HTMLElement>
  {
    using State = dom::TextDirectionState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"ltr", State::LTR},
      {u8"rtl", State::RTL},
      {u8"auto", State::Auto},
    };

    // Without a missing or invalid value default, those values are in no state (`State::Undefined` outside
    // this).
  };

  /// @see https://html.spec.whatwg.org/#attr-contenteditable
  template <>
  struct EnumeratedAttributeTraits<"contenteditable", HTMLElement>
  {
    using State = ContentEditableAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"true", State::True},
      {u8"false", State::False},
      {u8"plaintext-only", State::PlaintextOnly},
    };

    constexpr static State MissingValueDefault = State::Inherit;
    constexpr static State InvalidValueDefault = State::Inherit;
    constexpr static State EmptyValueDefault = State::True;
  };

  template <>
  struct EnumeratedAttributeTraits<"translate", HTMLElement>
  {
    using State = TranslateAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"yes", State::Yes},
      {u8"no", State::No},
    };

    constexpr static State MissingValueDefault = State::Inherit;
    constexpr static State InvalidValueDefault = State::Inherit;
    constexpr static State EmptyValueDefault = State::Yes;
  };

  template <>
  struct EnumeratedAttributeTraits<"hidden", HTMLElement>
  {
    using State = HiddenAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"hidden", State::Hidden},
      {u8"until-found", State::UntilFound},
    };

    constexpr static State MissingValueDefault = State::NotHidden;
    constexpr static State InvalidValueDefault = State::Hidden;
    constexpr static State EmptyValueDefault = State::Hidden;
  };

  template <>
  struct EnumeratedAttributeTraits<"draggable", HTMLElement>
  {
    using State = DraggableAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"true", State::True},
      {u8"false", State::False},
    };

    constexpr static State MissingValueDefault = State::Auto;
    constexpr static State InvalidValueDefault = State::Auto;
  };

  template <>
  struct EnumeratedAttributeTraits<"spellcheck", HTMLElement>
  {
    using State = SpellcheckAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"true", State::True},
      {u8"false", State::False},
    };

    constexpr static State MissingValueDefault = State::Default;
    constexpr static State InvalidValueDefault = State::Default;
    constexpr static State EmptyValueDefault = State::True;
  };

  template <>
  struct EnumeratedAttributeTraits<"writingsuggestions", HTMLElement>
  {
    using State = WritingSuggestionsAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"true", State::True},
      {u8"false", State::False},
    };

    constexpr static State MissingValueDefault = State::Default;
    constexpr static State InvalidValueDefault = State::True;
    constexpr static State EmptyValueDefault = State::True;
  };

  template <>
  struct EnumeratedAttributeTraits<"autocapitalize", HTMLElement>
  {
    using State = AutocapitalizeAttributeState;
    // "none" and "sentences" are the canonical keywords of their states. The default state has none.
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"none", State::None},    {u8"off", State::None},    {u8"sentences", State::Sentences},
      {u8"on", State::Sentences}, {u8"words", State::Words}, {u8"characters", State::Characters},
    };

    constexpr static State MissingValueDefault = State::Default;
    constexpr static State InvalidValueDefault = State::Sentences;
  };

  template <>
  struct EnumeratedAttributeTraits<"autocorrect", HTMLElement>
  {
    using State = AutocorrectAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"on", State::On},
      {u8"off", State::Off},
    };

    constexpr static State MissingValueDefault = State::On;
    constexpr static State InvalidValueDefault = State::On;
    constexpr static State EmptyValueDefault = State::On;
  };

  /// @see https://html.spec.whatwg.org/#attr-inputmode
  template <>
  struct EnumeratedAttributeTraits<"inputmode", HTMLElement>
  {
    using State = InputModeAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"none", State::None},       {u8"text", State::Text},     {u8"tel", State::Tel},
      {u8"url", State::URL},         {u8"email", State::Email},   {u8"numeric", State::Numeric},
      {u8"decimal", State::Decimal}, {u8"search", State::Search},
    };
  };

  /// @see https://html.spec.whatwg.org/#attr-enterkeyhint
  template <>
  struct EnumeratedAttributeTraits<"enterkeyhint", HTMLElement>
  {
    using State = EnterKeyHintAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"enter", State::Enter}, {u8"done", State::Done},         {u8"go", State::Go},
      {u8"next", State::Next},   {u8"previous", State::Previous}, {u8"search", State::Search},
      {u8"send", State::Send},
    };
  };

  /// @see https://html.spec.whatwg.org/#lazy-loading-attribute
  template <>
  struct EnumeratedAttributeTraits<"loading", HTMLElement>
  {
    using State = LoadingAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"lazy", State::Lazy},
      {u8"eager", State::Eager},
    };

    constexpr static State MissingValueDefault = State::Eager;
    constexpr static State InvalidValueDefault = State::Eager;
  };

  /// @brief A submit button's `formmethod`, which has no missing value default.
  /// @see https://html.spec.whatwg.org/#attr-fs-formmethod
  template <>
  struct EnumeratedAttributeTraits<"formmethod", HTMLElement>
  {
    using State = MethodAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"get", State::Get},
      {u8"post", State::Post},
      {u8"dialog", State::Dialog},
    };

    constexpr static State InvalidValueDefault = State::Get;
  };

  /// @brief A submit button's `formenctype`, which has no missing value default.
  /// @see https://html.spec.whatwg.org/#attr-fs-formenctype
  template <>
  struct EnumeratedAttributeTraits<"formenctype", HTMLElement>
  {
    using State = EnctypeAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"application/x-www-form-urlencoded", State::URLEncoded},
      {u8"multipart/form-data", State::FormData},
      {u8"text/plain", State::TextPlain},
    };

    constexpr static State InvalidValueDefault = State::URLEncoded;
  };

  /// @see https://html.spec.whatwg.org/#fetch-priority-attribute
  template <>
  struct EnumeratedAttributeTraits<"fetchpriority", HTMLElement>
  {
    using State = FetchPriorityAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"low", State::Low},
      {u8"high", State::High},
      {u8"auto", State::Auto},
    };

    constexpr static State MissingValueDefault = State::Auto;
    constexpr static State InvalidValueDefault = State::Auto;
  };

  /// @see https://html.spec.whatwg.org/#attr-popover
  template <>
  struct EnumeratedAttributeTraits<"popover", HTMLElement>
  {
    using State = dom::PopoverState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"auto", State::Auto},
      {u8"manual", State::Manual},
      {u8"hint", State::Hint},
    };

    // The No Popover state, which has no keyword.
    constexpr static State MissingValueDefault = State::None;
    constexpr static State InvalidValueDefault = State::Manual;
    constexpr static State EmptyValueDefault = State::Auto;
  };

  /// @see https://html.spec.whatwg.org/#cors-settings-attribute
  /// @see https://html.spec.whatwg.org/#attr-crossorigin-none
  template <>
  struct EnumeratedAttributeTraits<"crossorigin", HTMLElement>
  {
    using State = CrossOriginAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"anonymous", State::Anonymous},
      {u8"use-credentials", State::UseCredentials},
    };

    constexpr static State MissingValueDefault = State::NoCORS;
    constexpr static State InvalidValueDefault = State::Anonymous;
    constexpr static State EmptyValueDefault = State::Anonymous;
  };

  /// @brief A referrer policy attribute: its keywords are the referrer policies, and the empty string is its
  /// missing and invalid value default.
  /// @see https://html.spec.whatwg.org/#referrer-policy-attribute
  template <>
  struct EnumeratedAttributeTraits<"referrerpolicy", HTMLElement>
  {
    using State = ReferrerPolicyAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"", State::Empty},
      {u8"no-referrer", State::NoReferrer},
      {u8"no-referrer-when-downgrade", State::NoReferrerWhenDowngrade},
      {u8"same-origin", State::SameOrigin},
      {u8"origin", State::Origin},
      {u8"strict-origin", State::StrictOrigin},
      {u8"origin-when-cross-origin", State::OriginWhenCrossOrigin},
      {u8"strict-origin-when-cross-origin", State::StrictOriginWhenCrossOrigin},
      {u8"unsafe-url", State::UnsafeURL},
    };

    constexpr static State MissingValueDefault = State::Empty;
    constexpr static State InvalidValueDefault = State::Empty;
  };
}
