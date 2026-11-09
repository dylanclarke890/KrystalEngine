/*
 * Copyright (C) 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"

namespace krys::boo::html
{
  class HTMLProgressElement;

  /// @brief An element of a `<progress>`'s shadow tree, rendered only when the progress element has no native
  /// appearance.
  class ProgressShadowElement : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProgressShadowElement);

  public:
    KRYS_NODISCARD HTMLProgressElement *ProgressElement() const noexcept;

  protected:
    explicit ProgressShadowElement(dom::Document &) noexcept;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept override;
  };

  /// @brief The `::-webkit-progress-inner-element` part, rendered as a `RenderProgress`.
  class ProgressInnerElement final : public ProgressShadowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProgressInnerElement);

  public:
    explicit ProgressInnerElement(dom::Document &) noexcept;

    KRYS_NODISCARD static Ref<ProgressInnerElement> Create(dom::Document &) noexcept;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept override;
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept override;
  };

  /// @brief The `::-webkit-progress-bar` part.
  class ProgressBarElement final : public ProgressShadowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProgressBarElement);

  public:
    explicit ProgressBarElement(dom::Document &) noexcept;

    KRYS_NODISCARD static Ref<ProgressBarElement> Create(dom::Document &) noexcept;
  };

  /// @brief The `::-webkit-progress-value` part, as wide as the progress element's position.
  class ProgressValueElement final : public ProgressShadowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProgressValueElement);

  public:
    explicit ProgressValueElement(dom::Document &) noexcept;

    KRYS_NODISCARD static Ref<ProgressValueElement> Create(dom::Document &) noexcept;
    void SetInlineSizePercentage(double) noexcept;
  };
}
