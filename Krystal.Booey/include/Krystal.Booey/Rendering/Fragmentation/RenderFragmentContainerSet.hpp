/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Fragmentation/RenderFragmentContainer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class RenderFragmentedFlow;

  /// @brief A run of fragments that all have the same width and height (such as a run of same-sized
  /// columns), so one renderer stands for them all.
  // FIXME: For now we derive from RenderFragmentContainer, but this may change at some point.
  class RenderFragmentContainerSet : public RenderFragmentContainer
  {
    KRYS_NON_COPY_MOVABLE(RenderFragmentContainerSet);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderFragmentContainerSet);

  public:
    /// @brief Extends the set's portion of the flow to the end of the flow's content and overflow, since a
    /// set can always make more columns for it.
    void ExpandToEncompassFragmentedFlowContentsIfNeeded() noexcept;

  protected:
    RenderFragmentContainerSet(Type type, dom::Document &document, RenderStyle &&style,
                               RenderFragmentedFlow &fragmentedFlow) noexcept;
    ~RenderFragmentContainerSet() noexcept override;

  private:
    void InstallFragmentedFlow() noexcept final;

    KRYS_NODISCARD bool IsRenderFragmentContainerSet() const noexcept final
    {
      return true;
    }
  };
}
