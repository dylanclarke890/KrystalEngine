#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <memory>

namespace krys::boo::css
{
  class CSSRule;
  class CSSStyleSheet;

  class MediaList final : public RefCounted<MediaList>
  {
  private:
    MediaList(CSSStyleSheet *parentSheet);
    MediaList(CSSRule *parentRule);

    WeakPtr<CSSStyleSheet> _parentStyleSheet;
    WeakPtr<CSSRule> _parentRule;
    Maybe<mq::MediaQueryList> _detachedMediaQueries;

  public:
    ~MediaList() noexcept;

    KRYS_NODISCARD static Ref<MediaList> Create(CSSStyleSheet *parentSheet) noexcept
    {
      return AdoptRef(*new MediaList(parentSheet));
    }

    KRYS_NODISCARD static Ref<MediaList> Create(CSSRule *parentRule) noexcept
    {
      return AdoptRef(*new MediaList(parentRule));
    }

    KRYS_NODISCARD bool IsSupportedPropertyIndex(unsigned index) const noexcept
    {
      return index < Length();
    }

    KRYS_NODISCARD size_t Length() const noexcept;

    KRYS_NODISCARD CSSOMString Item(unsigned index) const noexcept;

    dom::ExceptionOr<void> DeleteMedium(const CSSOMString &oldMedium) noexcept;

    void AppendMedium(const CSSOMString &newMedium) noexcept;

    KRYS_NODISCARD CSSOMString MediaText() const noexcept;

    void MediaText(const CSSOMString &) noexcept;

    KRYS_NODISCARD CSSRule *ParentRule() const noexcept;

    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept;

    void DetachFromParent() noexcept;

    KRYS_NODISCARD const mq::MediaQueryList &MediaQueries() const noexcept;

  private:
    void SetMediaQueries(mq::MediaQueryList &&queries) noexcept;
  };
}