#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  template <typename>
  class ExceptionOr;
}

namespace krys::boo::css
{
  class CSSRule;
  class CSSStyleSheet;

  /// @brief The media query list of a style sheet, an `@import` rule or an `@media` rule, as exposed to
  /// scripts.
  /// @see https://drafts.csswg.org/cssom/#the-medialist-interface
  class MediaList final : public RefCounted<MediaList>
  {
  private:
    WeakPtr<CSSStyleSheet> _parentStyleSheet;
    WeakPtr<CSSRule> _parentRule;
    Maybe<mq::MediaQueryList> _detachedMediaQueries;

    explicit MediaList(CSSStyleSheet *parentSheet) noexcept;
    explicit MediaList(CSSRule *parentRule) noexcept;

  public:
    ~MediaList() noexcept;

    KRYS_NODISCARD static Ref<MediaList> Create(CSSStyleSheet *parentSheet) noexcept;
    KRYS_NODISCARD static Ref<MediaList> Create(CSSRule *parentRule) noexcept;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }

    KRYS_NODISCARD uint32 Length() const noexcept;
    KRYS_NODISCARD CSSOMString Item(uint32 index) const noexcept;
    dom::ExceptionOr<void> DeleteMedium(const CSSOMString &oldMedium) noexcept;
    void AppendMedium(const CSSOMString &newMedium) noexcept;

    KRYS_NODISCARD CSSOMString MediaText() const noexcept;
    void SetMediaText(const CSSOMString &value) noexcept;

    KRYS_NODISCARD CSSRule *ParentRule() const noexcept;
    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept;

    /// @brief Keeps a copy of the media queries, so the list still works once its owner has gone.
    void DetachFromParent() noexcept;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept;

  private:
    void SetMediaQueries(mq::MediaQueryList &&queries) noexcept;
  };
}
