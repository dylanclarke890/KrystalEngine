/*
 * Copyright (C) 2022, 2023 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/DOM/Types/Exception.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;

  template <typename>
  class ExceptionOr;
}

namespace krys::boo::css
{
  /// @brief The style sheets adopted by a document or shadow root (`adoptedStyleSheets`), with the checks and
  /// notifications of the IDL observable array.
  /// @see https://drafts.csswg.org/cssom/#dom-documentorshadowroot-adoptedstylesheets
  class CSSStyleSheetObservableArray
  {
    KRYS_NON_COPY_MOVABLE(CSSStyleSheetObservableArray);

  private:
    WeakPtr<dom::ContainerNode> _treeScope;
    List<Ref<CSSStyleSheet>> _sheets;

  public:
    /// @param treeScope The document or shadow root that adopts the sheets.
    explicit CSSStyleSheetObservableArray(dom::ContainerNode &treeScope) noexcept;

    /// @brief Replaces every sheet, if each of @p sheets can be adopted.
    dom::ExceptionOr<void> SetSheets(List<Ref<CSSStyleSheet>> &&sheets) noexcept;

    KRYS_NODISCARD const List<Ref<CSSStyleSheet>> &Sheets() const noexcept
    {
      return _sheets;
    }

    /// @brief Sets the sheet at @p index, or appends it if @p index is the length.
    /// @attention @p index must be at most the length.
    dom::ExceptionOr<void> SetValueAt(uint32 index, CSSStyleSheet &sheet) noexcept;

    /// @attention The array must not be empty.
    void RemoveLast() noexcept;

    /// @return The sheet at @p index, or null if @p index is out of range.
    KRYS_NODISCARD CSSStyleSheet *ValueAt(uint32 index) const noexcept;

    KRYS_NODISCARD uint32 Length() const noexcept
    {
      return static_cast<uint32>(_sheets.size());
    }

    /// @brief Removes the sheets from @p length onwards.
    /// @attention @p length must be at most the length.
    void ShrinkTo(uint32 length) noexcept;

  private:
    KRYS_NODISCARD Maybe<dom::Exception>
      ShouldThrowWhenAddingSheet(const CSSStyleSheet &sheet) const noexcept;

    void DidAddSheet(CSSStyleSheet &sheet) noexcept;
    void WillRemoveSheet(CSSStyleSheet &sheet) noexcept;
  };
}
