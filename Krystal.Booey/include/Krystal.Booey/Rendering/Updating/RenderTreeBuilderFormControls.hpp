/*
 * Copyright (C) 2017-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderButton;
  class RenderMenuList;

  /// @brief Attaches and detaches the children of form control renderers, which wrap them in an anonymous
  /// inner block.
  // FIXME: Checkmarks of checkboxes and radios (appearance: base).
  class RenderTreeBuilder::FormControls
  {
    KRYS_NON_COPY_MOVABLE(FormControls);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit FormControls(RenderTreeBuilder &builder) noexcept;

    /// @brief Updates the generated boxes of @p renderer's element, if it's a select or option rendered with
    /// base appearance: an option's checkmark and a select's picker icon.
    void UpdateAfterDescendants(RenderElement &renderer) noexcept;

    void Attach(RenderButton &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;
    void Attach(RenderMenuList &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;

    KRYS_NODISCARD RenderPtr<RenderObject> Detach(RenderMenuList &parent, RenderObject &child,
                                                  WillBeDestroyed willBeDestroyed) noexcept;
    KRYS_NODISCARD RenderPtr<RenderObject> Detach(RenderButton &parent, RenderObject &child,
                                                  WillBeDestroyed willBeDestroyed) noexcept;

  private:
    KRYS_NODISCARD RenderBlock &FindOrCreateParentForChild(RenderButton &parent) noexcept;
    KRYS_NODISCARD RenderBlock &FindOrCreateParentForChild(RenderMenuList &parent) noexcept;
  };
}
