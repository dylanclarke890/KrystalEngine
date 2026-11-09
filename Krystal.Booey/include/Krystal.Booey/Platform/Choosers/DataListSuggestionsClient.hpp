/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Booey/Platform/Choosers/DataListSuggestionPicker.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief Who a datalist suggestion picker shows the suggestions of: a text field. It detaches its picker
  /// before it goes away.
  class DataListSuggestionsClient
  {
  public:
    virtual ~DataListSuggestionsClient() noexcept = default;

    /// @brief Where the field is, in the root view's coordinates, for the list to go below it.
    KRYS_NODISCARD virtual IntRect ElementRectInRootViewCoordinates() const noexcept = 0;
    /// @brief The suggestions for the field's current text: those starting with it, then those containing
    /// it.
    KRYS_NODISCARD virtual List<DataListSuggestion> Suggestions() noexcept = 0;
    /// @brief Whether the field lays out right to left, so the list is mirrored.
    KRYS_NODISCARD virtual bool IsElementRTL() const noexcept = 0;
    virtual void DidSelectDataListOption(utf8_stringview value) noexcept = 0;
    virtual void DidCloseSuggestions() noexcept = 0;
  };
}
