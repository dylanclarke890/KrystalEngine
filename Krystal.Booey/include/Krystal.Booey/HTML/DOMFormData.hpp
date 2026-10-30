#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/FileAPI/File.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLFormElement;

  /// @brief An entry list: the name and value pairs a form submits.
  /// @see https://xhr.spec.whatwg.org/#interface-formdata
  // FIXME: Encodings other than UTF-8, once there are any.
  class DOMFormData : public RefCounted<DOMFormData>
  {
  public:
    using FormDataEntryValue = Variant<RefPtr<File>, dom::DOMString>;

    struct Item
    {
      dom::DOMString Name;
      FormDataEntryValue Data;
    };

  private:
    List<Item> _items;

  public:
    DOMFormData() noexcept = default;

    /// @brief The entries of @p form, as submitted by @p submitter.
    /// @see https://xhr.spec.whatwg.org/#dom-formdata
    KRYS_NODISCARD static dom::ExceptionOr<Ref<DOMFormData>> Create(HTMLFormElement *form,
                                                                    HTMLElement *submitter) noexcept;

    KRYS_NODISCARD static Ref<DOMFormData> Create() noexcept;

    KRYS_NODISCARD const List<Item> &Items() const noexcept
    {
      return _items;
    }

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-append
    void Append(dom::DOMStringView name, dom::DOMStringView value) noexcept;
    /// @brief Appends @p blob as a file named @p filename (if given; else a file keeps its name, and other
    /// blobs are named "blob").
    /// @see https://xhr.spec.whatwg.org/#dom-formdata-append-blob
    void Append(dom::DOMStringView name, Blob &blob,
                Maybe<dom::DOMStringView> filename = std::nullopt) noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-delete
    void Remove(dom::DOMStringView name) noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-get
    KRYS_NODISCARD Maybe<FormDataEntryValue> Get(dom::DOMStringView name) const noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-getall
    KRYS_NODISCARD List<FormDataEntryValue> GetAll(dom::DOMStringView name) const noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-has
    KRYS_NODISCARD bool Has(dom::DOMStringView name) const noexcept;

    /// @see https://xhr.spec.whatwg.org/#dom-formdata-set
    void Set(dom::DOMStringView name, dom::DOMStringView value) noexcept;
    /// @see https://xhr.spec.whatwg.org/#dom-formdata-set-blob
    void Set(dom::DOMStringView name, Blob &blob, Maybe<dom::DOMStringView> filename = std::nullopt) noexcept;

    KRYS_NODISCARD Ref<DOMFormData> Clone() const noexcept;

  private:
    void Set(dom::DOMStringView name, Item &&item) noexcept;
  };
}
