#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo
{
  class Document;
  class MediaQueryList;
  class RenderStyle;
}

namespace krys::boo::css
{
  /// @brief A document's `MediaQueryList`s: it creates them for `matchMedia()` and, each time the rendering
  /// is updated, re-evaluates them in the order they were created and fires `change` at those that changed.
  class MediaQueryMatcher final : public RefCounted<MediaQueryMatcher>
  {
    KRYS_NON_COPY_MOVABLE(MediaQueryMatcher);

  private:
    WeakPtr<Document> _document;
    List<WeakPtr<MediaQueryList>> _mediaQueryLists;
    /// @brief How many times `EvaluateAll` has run. A list compares it with the round it was last evaluated
    /// in, to tell whether its `matches` is current, and with the round it last changed in, to tell whether
    /// it needs a `change` event.
    uint32 _evaluationRound {1u};

    explicit MediaQueryMatcher(Document &document) noexcept;

  public:
    ~MediaQueryMatcher() noexcept;

    KRYS_NODISCARD static Ref<MediaQueryMatcher> Create(Document &document) noexcept;

    /// @brief Forgets the document and detaches the lists, which keep their last `matches`.
    void DocumentDestroyed() noexcept;

    /// @brief Adds a new list, which must not already have been added.
    void AddMediaQueryList(MediaQueryList &list) noexcept;

    void RemoveMediaQueryList(MediaQueryList &list) noexcept;

    /// @brief A new `MediaQueryList` for @p query, or null if the document is gone.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-matchmedia
    KRYS_NODISCARD RefPtr<MediaQueryList> MatchMedia(CSSOMStringView query) noexcept;

    KRYS_NODISCARD uint32 EvaluationRound() const noexcept
    {
      return _evaluationRound;
    }

    /// @brief Starts a new round, re-evaluating every list and firing `change` at those whose `matches`
    /// changed.
    /// @see https://drafts.csswg.org/cssom-view/#evaluate-media-queries-and-report-changes
    void EvaluateAll() noexcept;

    /// @brief Whether @p queries match the document now; false if it has no document element or frame.
    KRYS_NODISCARD bool Evaluate(const mq::MediaQueryList &queries) const noexcept;

    /// @brief The document's media type, or null if it has no view.
    KRYS_NODISCARD CSSOMStringAtom MediaType() const noexcept;

  private:
    /// @brief The document element's style from the user agent's rules alone, which media features such as
    /// `em` lengths resolve against; null without a document element or frame.
    KRYS_NODISCARD UniquePtr<RenderStyle> DocumentElementUserAgentStyle() const noexcept;
  };
}
