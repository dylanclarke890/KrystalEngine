#pragma once

#include "Krystal.Booey/Css/Queries/MediaQuery.hpp"
#include "Krystal.Booey/Css/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class CSSRule;
  class CSSStyleSheet;
  template <typename>
  class ExceptionOr;

  /// @brief The collection of media queries of a style sheet, an `@import` rule or an `@media` rule, as
  /// scripts see it. The queries belong to the owner, which the list changes; once the owner goes, the list
  /// keeps a copy of its own.
  /// @see https://drafts.csswg.org/cssom/#the-medialist-interface
  class MediaList final : public RefCounted<MediaList>
  {
  private:
    // The owner: a style sheet, or an `@import` or `@media` rule.
    WeakPtr<CSSStyleSheet> _parentStyleSheet;
    WeakPtr<CSSRule> _parentRule;
    // The queries once detached from the owner.
    Maybe<css::mq::MediaQueryList> _detachedMediaQueries;

    explicit MediaList(CSSStyleSheet *parentSheet) noexcept;

    explicit MediaList(CSSRule *parentRule) noexcept;

  public:
    ~MediaList() noexcept;

    KRYS_NODISCARD static Ref<MediaList> Create(CSSStyleSheet *parentSheet) noexcept;

    KRYS_NODISCARD static Ref<MediaList> Create(CSSRule *parentRule) noexcept;

#pragma region MediaList - https://drafts.csswg.org/cssom/#the-medialist-interface

    /// @brief The serialisation of the queries.
    /// @see https://drafts.csswg.org/cssom/#dom-medialist-mediatext
    KRYS_NODISCARD CSSOMString MediaText() const noexcept;

    /// @brief Replaces the queries with those parsed from @p text.
    /// @see https://drafts.csswg.org/cssom/#dom-medialist-mediatext
    void SetMediaText(const CSSOMString &text) noexcept;

    /// @brief The serialisation of the query at @p index, or empty past the end.
    /// @see https://drafts.csswg.org/cssom/#dom-medialist-item
    KRYS_NODISCARD CSSOMString Item(uint32 index) const noexcept;

    /// @brief The number of queries.
    /// @see https://drafts.csswg.org/cssom/#dom-medialist-length
    KRYS_NODISCARD uint32 Length() const noexcept;

    /// @brief Appends @p medium, a single query, unless the list already has it.
    /// @see https://drafts.csswg.org/cssom/#dom-medialist-appendmedium
    void AppendMedium(const CSSOMString &medium) noexcept;

    /// @brief Removes every query that is @p medium, a single query; a `NotFoundError` if none is.
    /// @see https://drafts.csswg.org/cssom/#dom-medialist-deletemedium
    ExceptionOr<void> DeleteMedium(const CSSOMString &medium) noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }

    /// @brief The `@import` or `@media` rule whose queries these are, if any.
    KRYS_NODISCARD CSSRule *ParentRule() const noexcept;

    /// @brief The style sheet whose queries these are, if any.
    KRYS_NODISCARD CSSStyleSheet *ParentStyleSheet() const noexcept;

    /// @brief The queries.
    KRYS_NODISCARD const css::mq::MediaQueryList &MediaQueries() const noexcept;

    /// @brief Keeps a copy of the queries and stops following the owner, which is going.
    void DetachFromParent() noexcept;

  private:
    /// @brief Gives the owner @p queries, as a change to its sheet.
    void SetMediaQueries(css::mq::MediaQueryList queries) noexcept;
  };
}
