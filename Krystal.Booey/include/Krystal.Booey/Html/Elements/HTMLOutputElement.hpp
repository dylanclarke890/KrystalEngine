/*
 * Copyright (c) 2010 Google Inc. All rights reserved.
 * Copyright (c) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Html/Elements/HTMLFormControlElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/form-elements.html#htmloutputelement
  class HTMLOutputElement : public HTMLFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLOutputElement);

  private:
    UniquePtr<DOMTokenList> _htmlFor;
    Maybe<DOMString> _defaultValueOverride;

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return false;
    }

    HTMLOutputElement(Document &document) noexcept;

    ~HTMLOutputElement() noexcept override;

#pragma region HTMLOutputElement - https://html.spec.whatwg.org/multipage/form-elements.html#htmloutputelement

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-output-htmlfor
    KRYS_NODISCARD DOMTokenList &HtmlFor() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-output-name
    KRYS_NODISCARD DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-output-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-output-type
    KRYS_NODISCARD DOMString Type() const noexcept
    {
      return FormControlType();
    }

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-output-defaultvalue
    KRYS_NODISCARD DOMString DefaultValue() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-output-defaultvalue
    void DefaultValue(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-output-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-output-value
    void Value(DOMString value) noexcept;

#pragma endregion

    KRYS_NODISCARD DOMString FormControlType() const noexcept final
    {
      return u8"output";
    }

    KRYS_NODISCARD bool IsEnumeratable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsLabelable() const noexcept final
    {
      return true;
    }

    void Reset() noexcept final;

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    KRYS_NODISCARD bool ComputeWillValidate() const noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLOutputElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLOutputElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLOutputElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
