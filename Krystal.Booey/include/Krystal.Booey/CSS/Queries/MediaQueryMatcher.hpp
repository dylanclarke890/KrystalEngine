#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css
{
  class MediaQueryList;

  /// @brief Evaluates a document's `MediaQueryList`s whenever it's needed and dispatches `change` events on
  /// the ones whose query has changed. `MediaQueryList`s are evaluated in the order in which they were added.
  class MediaQueryMatcher final : public RefCounted<MediaQueryMatcher>
  {
    KRYS_NON_COPY_MOVABLE(MediaQueryMatcher);

  public:
    enum class EventMode : uint8
    {
      Schedule,
      DispatchNow
    };

  private:
    WeakPtr<dom::Document> _document;
    List<WeakPtr<MediaQueryList>> _mediaQueryLists;

    // This value is incremented at style selector changes.
    // It is used to avoid evaluating queries more then once and to make sure
    // that a media query result change is notified exactly once.
    uint32 _evaluationRound {1};

    explicit MediaQueryMatcher(dom::Document &document) noexcept;

  public:
    ~MediaQueryMatcher() noexcept;

    KRYS_NODISCARD static Ref<MediaQueryMatcher> Create(dom::Document &document) noexcept;

    void DocumentDestroyed() noexcept;
    void AddMediaQueryList(MediaQueryList &list) noexcept;
    void RemoveMediaQueryList(MediaQueryList &list) noexcept;

    /// @brief A new `MediaQueryList` for @p query, or null if the document is gone.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-matchmedia
    KRYS_NODISCARD RefPtr<MediaQueryList> MatchMedia(CSSOMStringView query) noexcept;

    KRYS_NODISCARD uint32 EvaluationRound() const noexcept
    {
      return _evaluationRound;
    }

    void EvaluateAll(EventMode eventMode) noexcept;

    KRYS_NODISCARD bool Evaluate(const mq::MediaQueryList &queries) const noexcept;

    /// @brief The document's media type, or null if it has no view.
    KRYS_NODISCARD CSSOMStringAtom MediaType() const noexcept;

  private:
    KRYS_NODISCARD UniquePtr<RenderStyle> DocumentElementUserAgentStyle() const noexcept;
  };
}
