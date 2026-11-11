/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2011-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/InputTypes/HiddenInputType.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  bool HiddenInputType::RendererIsNeeded() noexcept
  {
    return false;
  }

  bool HiddenInputType::StoresValueSeparateFromAttribute() noexcept
  {
    return false;
  }

  void HiddenInputType::SetValue(dom::DOMStringView sanitizedValue, bool, TextFieldEventBehavior,
                                 TextControlSetValueSelection) noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetAttribute(u8"value", dom::DOMString(sanitizedValue));
  }

  bool HiddenInputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto name = element.Name();

    if (text::IsASCIICaselessEqual(dom::DOMStringView(name), u8"_charset_"))
    {
      // Entry lists are UTF-8 only (see DIFFERENCES.md).
      formData.Append(name, u8"UTF-8");
      return true;
    }
    InputType::AppendFormData(formData);
    if (auto dirname = element.GetAttribute(u8"dirname"))
    {
      formData.Append(*dirname, element.DirectionForFormData());
    }
    return true;
  }

  bool HiddenInputType::ShouldRespectHeightAndWidthAttributes() noexcept
  {
    return true;
  }

  bool HiddenInputType::DirAutoUsesValue() const noexcept
  {
    return true;
  }
}
