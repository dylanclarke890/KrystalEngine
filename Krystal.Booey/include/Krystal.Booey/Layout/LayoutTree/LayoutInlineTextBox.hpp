/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::layout
{
  /// @brief A layout box for a run of text, always anonymous and inline.
  class InlineTextBox : public Box
  {
    KRYS_NON_COPY_MOVABLE(InlineTextBox);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(InlineTextBox);

  public:
    enum class ContentCharacteristic : uint8
    {
      CanUseSimplifiedContentMeasuring,
      CanUseSimpleFontCodepath,
      ShouldUseSimpleGlyphOverflowCodePath,
      HasPositionDependentContentWidth,
      HasStrongDirectionalityContent
    };

  private:
    utf8_string _content;
    bool _isCombined {false};
    EnumSet<ContentCharacteristic> _contentCharacteristicSet;

  public:
    InlineTextBox(utf8_string content, bool isCombined,
                  EnumSet<ContentCharacteristic> contentCharacteristicSet, RenderStyle &&style,
                  UniquePtr<RenderStyle> &&firstLineStyle = nullptr) noexcept;
    ~InlineTextBox() noexcept override = default;

    KRYS_NODISCARD const utf8_string &Content() const noexcept
    {
      return _content;
    }
    KRYS_NODISCARD bool IsCombined() const noexcept
    {
      return _isCombined;
    }

    // FIXME: This should not be a box's property.
    KRYS_NODISCARD bool CanUseSimplifiedContentMeasuring() const noexcept
    {
      return _contentCharacteristicSet.Contains(ContentCharacteristic::CanUseSimplifiedContentMeasuring);
    }
    KRYS_NODISCARD bool CanUseSimpleFontCodePath() const noexcept
    {
      return _contentCharacteristicSet.Contains(ContentCharacteristic::CanUseSimpleFontCodepath);
    }
    KRYS_NODISCARD bool ShouldUseSimpleGlyphOverflowCodePath() const noexcept
    {
      return _contentCharacteristicSet.Contains(ContentCharacteristic::ShouldUseSimpleGlyphOverflowCodePath);
    }
    KRYS_NODISCARD bool HasPositionDependentContentWidth() const noexcept
    {
      return _contentCharacteristicSet.Contains(ContentCharacteristic::HasPositionDependentContentWidth);
    }
    KRYS_NODISCARD bool HasStrongDirectionalityContent() const noexcept
    {
      return _contentCharacteristicSet.Contains(ContentCharacteristic::HasStrongDirectionalityContent);
    }

    void SetContent(utf8_string newContent, EnumSet<ContentCharacteristic> contentCharacteristicSet) noexcept
    {
      _content = krys::move(newContent);
      _contentCharacteristicSet = contentCharacteristicSet;
    }
    void SetContentCharacteristic(EnumSet<ContentCharacteristic> contentCharacteristicSet) noexcept
    {
      _contentCharacteristicSet = contentCharacteristicSet;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::layout::InlineTextBox)
  KRYS_NODISCARD static bool IsType(const krys::boo::layout::Box &box) noexcept
  {
    return box.IsInlineTextBox();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
