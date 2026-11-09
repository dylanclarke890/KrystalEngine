/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Nodes/TextDirectionState.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  enum class TextDirection : bool;
}

namespace krys::boo::dom
{
  class Element;

  /// @brief @p element's auto directionality: from its value if it's a form control whose value decides it,
  /// otherwise from the first of its text with a strong character. Null if nothing decides it.
  /// @see https://html.spec.whatwg.org/#auto-directionality
  KRYS_NODISCARD Maybe<TextDirection> ComputeAutoDirectionality(const Element &element) noexcept;

  /// @brief @p element's directionality if it's its auto directionality; null otherwise.
  KRYS_NODISCARD Maybe<TextDirection> ComputeTextDirectionIfDirIsAuto(const Element &element) noexcept;

  /// @brief Updates the directionality of @p element and of what it decides after its `dir` attribute
  /// changed to @p textDirectionState.
  void TextDirectionStateChanged(Element &element, TextDirectionState textDirectionState) noexcept;

  /// @brief Updates the directionality of @p element, its descendants and, if it decides theirs, its
  /// ancestors whose directionality is auto.
  /// @param initiator The element whose change led here, which is left as it is.
  void UpdateEffectiveTextDirectionState(Element &element, TextDirectionState textDirectionState,
                                         Element *initiator = nullptr) noexcept;

  /// @brief Gives @p direction to @p element's descendants (and their shadow trees) that take their
  /// directionality from it.
  void UpdateEffectiveTextDirectionOfDescendants(Element &element, Maybe<TextDirection> direction,
                                                 Element *initiator = nullptr) noexcept;

  /// @brief Updates the directionality of the nearest inclusive ancestor of @p element whose directionality
  /// is auto.
  void UpdateEffectiveTextDirectionOfAncestors(Element &element, Element *initiator) noexcept;
}
