/*
 * Copyright (C) 2019-2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Web/Css/Highlights/Highlight.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web
{
  class Document;
  class ShadowRoot;
}

namespace krys::web::dom
{
  class StaticRange;
}

namespace krys::web
{
  /// @brief A highlight hit by `highlightsFromPoint()`, with those of its ranges that were hit.
  /// @see https://drafts.csswg.org/css-highlight-api-1/#dictdef-highlighthitresult
  struct HighlightHitResult
  {
    Ref<web::Highlight> Highlight;
    List<Ref<dom::AbstractRange>> Ranges;
  };

  /// @see https://drafts.csswg.org/css-highlight-api-1/#dictdef-highlightsfrompointoptions
  struct HighlightsFromPointOptions
  {
    /// @brief The shadow roots whose highlights can be returned.
    List<Ref<ShadowRoot>> ShadowRoots;
  };

  /// @brief A document's custom highlights by name, in the order they were registered (which is the order
  /// they're painted in, below their priority). The document's is `CSS.highlights`.
  /// @see https://drafts.csswg.org/css-highlight-api-1/#highlight-registry
  class HighlightRegistry : public RefCounted<HighlightRegistry>
  {
  private:
    WeakPtr<Document> _document;
    HashMap<DOMStringAtom, Ref<Highlight>> _map;
    List<DOMStringAtom> _highlightNames;

    explicit HighlightRegistry(Document &document) noexcept;

  public:
    /// @brief The registry of @p document's custom highlights.
    KRYS_NODISCARD static Ref<HighlightRegistry> Create(Document &document) noexcept;

#pragma region HighlightRegistry - https://drafts.csswg.org/css-highlight-api-1/#highlightregistry

    /// @brief Registers @p value as the highlight named @p key, in place of any highlight of that name (which
    /// keeps its place in the order).
    /// @see https://drafts.csswg.org/css-highlight-api-1/#custom-highlight-name
    /// @see https://webidl.spec.whatwg.org/#js-map-set
    void Set(const DOMStringAtom &key, Ref<Highlight> value) noexcept;

    /// @brief The highlight named @p key, if any.
    /// @see https://webidl.spec.whatwg.org/#js-map-get
    KRYS_NODISCARD Highlight *Get(const DOMStringAtom &key) const noexcept;

    /// @see https://webidl.spec.whatwg.org/#js-map-has
    KRYS_NODISCARD bool Has(const DOMStringAtom &key) const noexcept
    {
      return _map.contains(key);
    }

    /// @brief Unregisters the highlight named @p key.
    /// @return Whether there was one.
    /// @see https://webidl.spec.whatwg.org/#js-map-delete
    bool Delete(const DOMStringAtom &key) noexcept;

    /// @brief Unregisters every highlight.
    /// @see https://webidl.spec.whatwg.org/#js-map-clear
    void Clear() noexcept;

    /// @brief The number of registered names.
    /// @see https://webidl.spec.whatwg.org/#js-maplike
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _map.size();
    }

    /// @brief The highlights with a range at (@p x, @p y), in viewport coordinates, highest priority first.
    /// @see https://drafts.csswg.org/css-highlight-api-1/#dom-highlightregistry-highlightsfrompoint
    KRYS_NODISCARD List<HighlightHitResult>
      HighlightsFromPoint(float x, float y, const HighlightsFromPointOptions &options = {}) const noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _map.empty();
    }

    /// @brief Adds @p value to the registry's annotation highlight (the text fragments' highlight).
    void AddAnnotationHighlightWithRange(Ref<dom::StaticRange> value) noexcept;

    KRYS_NODISCARD const HashMap<DOMStringAtom, Ref<Highlight>> &Map() const noexcept
    {
      return _map;
    }

    /// @brief The registered names, in the order they were registered.
    KRYS_NODISCARD const List<DOMStringAtom> &HighlightNames() const noexcept
    {
      return _highlightNames;
    }
  };
}
