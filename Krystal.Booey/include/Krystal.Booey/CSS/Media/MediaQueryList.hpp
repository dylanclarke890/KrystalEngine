/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/DOM/Events/EventTarget.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  namespace mq
  {
    class MediaQueryEvaluator;
  }

  /// @brief The object `matchMedia()` returns: the current result of a media query list, and `change` events
  /// whenever it changes.
  /// @see https://drafts.csswg.org/cssom-view/#the-mediaquerylist-interface
  class MediaQueryList final : public dom::EventTarget
  {
  private:
    RefPtr<MediaQueryMatcher> _matcher;
    const mq::MediaQueryList _mediaQueries;
    // Indicates if the query has been evaluated after the last style selector change.
    uint32 _evaluationRound;
    // Used to know if the query has changed in the last style selector change.
    uint32 _changeRound;
    bool _matches;
    bool _needsNotification {false};

    MediaQueryList(MediaQueryMatcher &matcher, mq::MediaQueryList &&mediaQueries, bool matches) noexcept;

  public:
    ~MediaQueryList() noexcept override;

    KRYS_NODISCARD static Ref<MediaQueryList>
      Create(MediaQueryMatcher &matcher, mq::MediaQueryList &&mediaQueries, bool matches) noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-media
    KRYS_NODISCARD CSSOMString Media() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-matches
    KRYS_NODISCARD bool Matches() noexcept;

    /// @note Legacy.
    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-addlistener
    void AddListener(RefPtr<dom::EventListenerCallback> &&listener) noexcept;

    /// @note Legacy.
    /// @see https://drafts.csswg.org/cssom-view/#dom-mediaquerylist-removelistener
    void RemoveListener(RefPtr<dom::EventListenerCallback> &&listener) noexcept;

    void Evaluate(mq::MediaQueryEvaluator &evaluator, MediaQueryMatcher::EventMode eventMode) noexcept;

    void DetachFromMatcher() noexcept;

  private:
    void SetMatches(bool newValue) noexcept;
  };
}
