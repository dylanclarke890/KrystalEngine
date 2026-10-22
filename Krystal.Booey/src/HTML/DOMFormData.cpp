#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include <algorithm>

namespace krys::boo::html
{
  dom::ExceptionOr<Ref<DOMFormData>> DOMFormData::Create(HTMLFormElement *form,
                                                         HTMLElement *submitter) noexcept
  {
    auto formData = Create();
    if (!form)
    {
      return formData;
    }

    RefPtr<HTMLFormControlElement> control;
    if (submitter)
    {
      control = ShareRefPtr(DynamicDowncast<HTMLFormControlElement>(*submitter));
      if (!control || !control->IsSubmitButton())
      {
        return dom::Exception {dom::ExceptionCode::TypeError,
                               u8"The specified element is not a submit button."};
      }
      if (control->Form() != form)
      {
        return dom::Exception {dom::ExceptionCode::NotFoundError,
                               u8"The specified element is not owned by this form element."};
      }
    }

    auto result = form->ConstructEntryList(krys::move(control), krys::move(formData), nullptr);
    if (!result)
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"Already constructing Form entry list."};
    }

    Ref<DOMFormData> entryList = krys::move(result);
    return entryList;
  }

  Ref<DOMFormData> DOMFormData::Create() noexcept
  {
    return CreateRef<DOMFormData>();
  }

  Ref<DOMFormData> DOMFormData::Clone() const noexcept
  {
    auto newFormData = Create();
    newFormData->_items = _items;
    return newFormData;
  }

  // https://html.spec.whatwg.org/#create-an-entry
  void DOMFormData::Append(dom::DOMStringView name, dom::DOMStringView value) noexcept
  {
    _items.push_back({.Name = dom::DOMString(name), .Data = dom::DOMString(value)});
  }

  void DOMFormData::Remove(dom::DOMStringView name) noexcept
  {
    std::erase_if(_items, [&](const Item &item) { return item.Name == name; });
  }

  Maybe<DOMFormData::FormDataEntryValue> DOMFormData::Get(dom::DOMStringView name) const noexcept
  {
    for (auto &item : _items)
    {
      if (item.Name == name)
      {
        return item.Data;
      }
    }
    return std::nullopt;
  }

  List<DOMFormData::FormDataEntryValue> DOMFormData::GetAll(dom::DOMStringView name) const noexcept
  {
    List<FormDataEntryValue> result;
    for (auto &item : _items)
    {
      if (item.Name == name)
      {
        result.push_back(item.Data);
      }
    }
    return result;
  }

  bool DOMFormData::Has(dom::DOMStringView name) const noexcept
  {
    return std::ranges::any_of(_items, [&](const Item &item) { return item.Name == name; });
  }

  void DOMFormData::Set(dom::DOMStringView name, dom::DOMStringView value) noexcept
  {
    Set(name, Item {.Name = dom::DOMString(name), .Data = dom::DOMString(value)});
  }

  void DOMFormData::Set(dom::DOMStringView name, Item &&item) noexcept
  {
    // Replace the first item with a matching name, and remove the rest.
    auto first = std::ranges::find_if(_items, [&](const Item &existing) { return existing.Name == name; });
    if (first != _items.end())
    {
      auto firstIndex = static_cast<size_t>(first - _items.begin());
      *first = krys::move(item);
      for (size_t index = _items.size() - 1; index > firstIndex; --index)
      {
        if (_items[index].Name == name)
        {
          _items.erase(_items.begin() + static_cast<ptrdiff_t>(index));
        }
      }
      return;
    }

    _items.push_back(krys::move(item));
  }
}
