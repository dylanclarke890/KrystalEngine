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

#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"

namespace krys::boo::html
{
  /// @brief Who a list button tells it was clicked: its text field's input type.
  class DataListButtonOwner
  {
  public:
    virtual ~DataListButtonOwner() noexcept = default;

    virtual void DataListButtonElementWasClicked() noexcept = 0;
  };

  /// @brief The button at the end of a text field with a datalist (`::-webkit-list-button`), which shows its
  /// suggestions. It isn't displayed while the field has no datalist.
  class DataListButtonElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DataListButtonElement);

  private:
    DataListButtonOwner *_owner {nullptr};

  public:
    DataListButtonElement(dom::Document &document, DataListButtonOwner &owner) noexcept;

    KRYS_NODISCARD static Ref<DataListButtonElement> Create(dom::Document &document,
                                                            DataListButtonOwner &owner) noexcept;

    /// @brief The owner's going away.
    void RemoveOwner() noexcept
    {
      _owner = nullptr;
    }

    void DefaultEventHandler(dom::Event &event) noexcept override;

    /// @brief Disabled with its field.
    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept override;

  private:
    KRYS_NODISCARD Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept override;
  };
}
