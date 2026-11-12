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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::ax
{
  class AccessibilityObject;
  class AXObjectCache;

  /// @brief What a dump of the accessibility tree shows of each object.
  enum class AXStreamOption : uint8
  {
    ObjectID,
    Role,
    ParentID,
    /// @brief The element's `id`.
    IdentifierAttribute,
    /// @brief The accessible name, and the description if there's one.
    Name,
    /// @brief The object's value as text, for text and text controls.
    Value,
    /// @brief The renderer's or node's description.
    RendererOrNode,
    DisplayContents,
  };

  /// @brief Dumps of the accessibility tree, for tests and debugging.
  /// @note WebKit's `AXLogger` also traces the cache's work to its log channel; Booey has only the dumps.
  class AXLogger
  {
  public:
    /// @brief @p root and its descendants in the tree (unignored objects only), one per line, indented by
    /// depth.
    KRYS_NODISCARD static utf8_string TreeAsText(AccessibilityObject &root,
                                                 EnumSet<AXStreamOption> options = {
                                                   AXStreamOption::Role, AXStreamOption::Name}) noexcept;
    /// @brief The tree from @p cache's root.
    KRYS_NODISCARD static utf8_string TreeAsText(AXObjectCache &cache,
                                                 EnumSet<AXStreamOption> options = {
                                                   AXStreamOption::Role, AXStreamOption::Name}) noexcept;
    /// @brief One line describing @p object.
    KRYS_NODISCARD static utf8_string ObjectAsText(AccessibilityObject &object,
                                                   EnumSet<AXStreamOption> options) noexcept;
  };
}
