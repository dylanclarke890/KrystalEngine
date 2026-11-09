#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo
{
  utf8_stringview FormSubmission::Attributes::MethodString(Method method) noexcept
  {
    if (method == Method::Dialog)
    {
      return u8"dialog";
    }
    if (method == Method::Post)
    {
      return u8"post";
    }
    return u8"get";
  }

  void FormSubmission::Attributes::ParseAction(utf8_stringview action) noexcept
  {
    // FIXME: Can we parse into a URL? Then we also don't need to trim anymore.
    _action = dom::DOMString(infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(action));
  }

  utf8_stringview FormSubmission::Attributes::ParseEncodingType(utf8_stringview type) noexcept
  {
    if (text::IsASCIICaselessEqual(type, utf8_stringview(u8"multipart/form-data")))
    {
      return u8"multipart/form-data";
    }
    if (text::IsASCIICaselessEqual(type, utf8_stringview(u8"text/plain")))
    {
      return u8"text/plain";
    }
    return u8"application/x-www-form-urlencoded";
  }

  void FormSubmission::Attributes::UpdateEncodingType(utf8_stringview type) noexcept
  {
    _encodingType = dom::DOMString(ParseEncodingType(type));
  }

  FormSubmission::Method FormSubmission::Attributes::ParseMethodType(utf8_stringview type) noexcept
  {
    if (text::IsASCIICaselessEqual(type, utf8_stringview(u8"dialog")))
    {
      return FormSubmission::Method::Dialog;
    }
    if (text::IsASCIICaselessEqual(type, utf8_stringview(u8"post")))
    {
      return FormSubmission::Method::Post;
    }
    return FormSubmission::Method::Get;
  }

  void FormSubmission::Attributes::UpdateMethodType(utf8_stringview type) noexcept
  {
    _method = ParseMethodType(type);
  }

  FormSubmission::FormSubmission(Maybe<dom::DOMString> returnValue, const url::URL &action,
                                 dom::DOMStringView target, dom::DOMStringView encodingType,
                                 dom::Event *event) noexcept
      : _method(Method::Dialog), _action(action), _target(target), _encodingType(encodingType),
        _event(ShareRefPtr(event)), _returnValue(krys::move(returnValue))
  {
  }

  FormSubmission::FormSubmission(Method method, const url::URL &action, dom::DOMStringView target,
                                 dom::DOMStringView encodingType, Ref<html::DOMFormData> &&entryList,
                                 dom::Event *event) noexcept
      : _method(method), _action(action), _target(target), _encodingType(encodingType),
        _entryList(krys::move(entryList)), _event(ShareRefPtr(event))
  {
  }

  FormSubmission::~FormSubmission() noexcept = default;

  Ref<FormSubmission> FormSubmission::Create(html::HTMLFormElement &form,
                                             html::HTMLFormControlElement *overrideSubmitter,
                                             const Attributes &attributes, dom::Event *event,
                                             FormSubmissionTrigger trigger) noexcept
  {
    auto copiedAttributes = attributes;

    RefPtr<html::HTMLFormControlElement> submitter =
      ShareRefPtr(overrideSubmitter ? overrideSubmitter : form.FindSubmitter(event));
    if (submitter)
    {
      if (auto attributeValue = submitter->GetAttribute(u8"formaction"))
      {
        copiedAttributes.ParseAction(*attributeValue);
      }
      if (auto attributeValue = submitter->GetAttribute(u8"formenctype"))
      {
        copiedAttributes.UpdateEncodingType(*attributeValue);
      }
      if (auto attributeValue = submitter->GetAttribute(u8"formmethod"))
      {
        copiedAttributes.UpdateMethodType(*attributeValue);
      }
      if (auto attributeValue = submitter->GetAttribute(u8"formtarget"))
      {
        copiedAttributes.SetTarget(*attributeValue);
      }
    }

    Ref<dom::Document> document = ShareRef(form.NodeDocument());
    dom::DOMString encodingType = copiedAttributes.EncodingType();
    auto actionURL = document->CompleteURL(copiedAttributes.Action().empty() ? document->GetURL().String()
                                                                             : copiedAttributes.Action());

    Ref<FormSubmission> formSubmission = [&]
    {
      if (copiedAttributes.GetMethod() == Method::Dialog)
      {
        Maybe<dom::DOMString> returnValue = submitter ? submitter->ResultForDialogSubmit() : null;
        return CreateRef<FormSubmission>(returnValue, actionURL, form.EffectiveTarget(event, submitter.get()),
                                         encodingType, event);
      }

      krys_debug_assert(copiedAttributes.GetMethod() == Method::Post
                        || copiedAttributes.GetMethod() == Method::Get);

      // (The entries are UTF-8 whatever the form's accept-charset and the document's encoding; see
      // DIFFERENCES.md.)
      RefPtr<html::DOMFormData> result =
        form.ConstructEntryList(RefPtr(submitter), html::DOMFormData::Create(), nullptr);
      krys_assert(result);
      Ref<html::DOMFormData> entryList = result;
      return CreateRef<FormSubmission>(copiedAttributes.GetMethod(), actionURL,
                                       form.EffectiveTarget(event, submitter.get()), encodingType,
                                       krys::move(entryList), event);
    }();

    formSubmission->_form = ShareRefPtr(&form);
    formSubmission->_submitter = submitter;
    formSubmission->_trigger = trigger;
    return formSubmission;
  }

  html::HTMLFormElement *FormSubmission::Form() const noexcept
  {
    return _form.get();
  }

  html::HTMLFormControlElement *FormSubmission::Submitter() const noexcept
  {
    return _submitter.get();
  }

  html::DOMFormData *FormSubmission::EntryList() const noexcept
  {
    return _entryList.get();
  }

  void FormSubmission::PopulateFrameLoadRequest(FrameLoadRequest &frameRequest) const noexcept
  {
    krys_assert(_method == Method::Post || _method == Method::Get);

    if (!_target.empty())
    {
      frameRequest.SetFrameName(dom::DOMStringAtom(_target));
    }

    if (_method == Method::Post)
    {
      frameRequest.GetResourceRequest().SetHTTPMethod(u8"POST");
    }

    frameRequest.GetResourceRequest().SetURL(_action);
  }
}
