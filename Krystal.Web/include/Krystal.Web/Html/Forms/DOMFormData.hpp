/*
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Web/FileApi/File.hpp"

namespace krys::web
{
  class HTMLElement;
  class HTMLFormElement;
}

namespace krys::web::html
{
  /// @brief An entry list: the name and value pairs a form submits.
  /// @see https://xhr.spec.whatwg.org/#interface-formdata
  // Entry lists are UTF-8 only (see DIFFERENCES.md).
  class DOMFormData : public RefCounted<DOMFormData>
  {
  public:
    using FormDataEntryValue = Variant<RefPtr<File>, DOMString>;

    struct Item
    {
      DOMString Name;
      FormDataEntryValue Data;
    };

  private:
    List<Item> _items;

  public:
    DOMFormData() noexcept = default;

    /// @brief The entries of @p form, as submitted by @p submitter.
    /// @see https://xhr.spec.whatwg.org/#dom-formdata
    KRYS_NODISCARD static ExceptionOr<Ref<DOMFormData>> Create(HTMLFormElement *form,
                                                               HTMLElement *submitter) noexcept;

    KRYS_NODISCARD static Ref<DOMFormData> Create() noexcept;

    KRYS_NODISCARD const List<Item> &Items() const noexcept
    {
      return _items;
    }

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-append
    void Append(DOMStringView name, DOMStringView value) noexcept;

    /// @brief Appends @p blob as a file named @p filename (if given; else a file keeps its name, and other
    /// blobs are named "blob").
    /// @see https://xhr.spec.whatwg.org/#dom-formdata-append-blob
    void Append(DOMStringView name, Blob &blob, Maybe<DOMStringView> filename = std::nullopt) noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-delete
    void Remove(DOMStringView name) noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-get
    KRYS_NODISCARD Maybe<FormDataEntryValue> Get(DOMStringView name) const noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-getall
    KRYS_NODISCARD List<FormDataEntryValue> GetAll(DOMStringView name) const noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-has
    KRYS_NODISCARD bool Has(DOMStringView name) const noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-set
    void Set(DOMStringView name, DOMStringView value) noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-set-blob
    void Set(DOMStringView name, Blob &blob, Maybe<DOMStringView> filename = std::nullopt) noexcept;

    KRYS_NODISCARD Ref<DOMFormData> Clone() const noexcept;

  private:
    void Set(DOMStringView name, Item &&item) noexcept;
  };
}
