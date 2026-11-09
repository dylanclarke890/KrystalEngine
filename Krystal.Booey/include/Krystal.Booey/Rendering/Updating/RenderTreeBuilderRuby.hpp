/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  /// @brief Attaches children to style-based ruby (`display: ruby`, `ruby-base` and `ruby-text`), wrapping
  /// them in the anonymous ruby containers and ruby bases the CSS ruby box model needs.
  class RenderTreeBuilder::Ruby
  {
    KRYS_NON_COPY_MOVABLE(Ruby);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit Ruby(RenderTreeBuilder &builder) noexcept;

    KRYS_NODISCARD RenderElement &
      FindOrCreateParentForStyleBasedRubyChild(RenderElement &parent, const RenderObject &child,
                                               RenderObject *&beforeChild) noexcept;
    void AttachForStyleBasedRuby(RenderElement &parent, RenderPtr<RenderObject> child,
                                 RenderObject *beforeChild) noexcept;
  };

  /// @brief The style of an anonymous ruby container or ruby base: bidi-isolated, and ruby bases don't wrap.
  KRYS_NODISCARD RenderStyle CreateAnonymousStyleForRuby(const RenderStyle &parentStyle,
                                                         DisplayType display) noexcept;
}
