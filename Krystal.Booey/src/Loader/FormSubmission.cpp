#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Platform/Network/FormData.hpp"
#include "Krystal.Booey/Platform/Network/FormDataBuilder.hpp"
#include "Krystal.Booey/URL/URLParser.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <chrono>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD int64 GenerateFormDataIdentifier() noexcept
    {
      // Initialize to the current time to reduce the likelihood of generating identifiers that overlap with
      // those from past/future browser sessions.
      static int64 nextIdentifier = std::chrono::duration_cast<std::chrono::microseconds>(
                                      std::chrono::system_clock::now().time_since_epoch())
                                      .count();
      return ++nextIdentifier;
    }

    KRYS_NODISCARD utf8_string ReplaceAll(utf8_stringview string, char8 target,
                                          utf8_stringview replacement) noexcept
    {
      utf8_string result;
      for (char8 character : string)
      {
        if (character == target)
        {
          result += replacement;
        }
        else
        {
          result += character;
        }
      }
      return result;
    }

    // FIXME: This function copies the body a lot and is really inefficient.
    void AppendMailtoPostFormDataToURL(url::URL &url, const FormData &data,
                                       utf8_stringview encodingType) noexcept
    {
      utf8_string body = data.FlattenToString();

      if (text::IsASCIICaselessEqual(encodingType, utf8_stringview(u8"text/plain")))
      {
        // Convention seems to be to decode, and s/&/\r\n/. Also, spaces are encoded as %20.
        body = url::URLParser::PercentDecode(ReplaceAll(ReplaceAll(body, u8'&', u8"\r\n"), u8'+', u8" "));
      }

      List<uint8> bodyData {'b', 'o', 'd', 'y', '='};
      FormDataBuilder::EncodeStringAsFormData(
        bodyData, Span<const uint8>(reinterpret_cast<const uint8 *>(body.data()), body.size()));
      body = ReplaceAll(utf8_stringview(reinterpret_cast<const char8 *>(bodyData.data()), bodyData.size()),
                        u8'+', u8"%20");

      auto query = url.Query();
      if (query.empty())
      {
        url.SetQuery(body);
      }
      else
      {
        url.SetQuery(utf8_string(query) + u8'&' + body);
      }
    }
  }

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
    _isMultiPartForm = (_encodingType == u8"multipart/form-data");
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

  FormSubmission::FormSubmission(Method method, dom::DOMStringView returnValue, const url::URL &action,
                                 dom::DOMStringView target, dom::DOMStringView contentType,
                                 dom::Event *event) noexcept
      : _method(method), _action(action), _target(target), _contentType(contentType),
        _event(ShareRefPtr(event)), _returnValue(returnValue)
  {
  }

  FormSubmission::FormSubmission(Method method, const url::URL &action, dom::DOMStringView target,
                                 dom::DOMStringView contentType, Ref<FormData> &&data,
                                 utf8_stringview boundary, dom::Event *event) noexcept
      : _method(method), _action(action), _target(target), _contentType(contentType),
        _formData(krys::move(data)), _boundary(boundary), _event(ShareRefPtr(event))
  {
  }

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
        dom::DOMString returnValue = submitter ? submitter->ResultForDialogSubmit() : dom::DOMString();
        return CreateRef<FormSubmission>(copiedAttributes.GetMethod(), returnValue, actionURL,
                                         form.EffectiveTarget(event, submitter.get()), encodingType, event);
      }

      krys_debug_assert(copiedAttributes.GetMethod() == Method::Post
                        || copiedAttributes.GetMethod() == Method::Get);

      bool isMailtoForm = actionURL.ProtocolIs(u8"mailto");
      bool isMultiPartForm = false;

      // FIXME: Upgrading insecure requests (the content security policy), once there's one.

      if (copiedAttributes.GetMethod() == Method::Post)
      {
        isMultiPartForm = copiedAttributes.IsMultiPartForm();
        if (isMultiPartForm && isMailtoForm)
        {
          encodingType = u8"application/x-www-form-urlencoded";
          isMultiPartForm = false;
        }
      }

      // FIXME: The form's accept-charset and the document's encoding, once there are encodings other than
      // UTF-8.
      auto domFormData = html::DOMFormData::Create();

      RefPtr<html::DOMFormData> result =
        form.ConstructEntryList(RefPtr(submitter), krys::move(domFormData), nullptr);
      krys_assert(result);
      Ref<html::DOMFormData> entryList = result;

      RefPtr<FormData> formData;
      utf8_string boundary;

      if (isMultiPartForm)
      {
        formData = FormData::CreateMultiPart(*entryList);
        auto boundaryBytes = formData->Boundary();
        boundary = utf8_string(reinterpret_cast<const char8 *>(boundaryBytes.data()), boundaryBytes.size());
      }
      else
      {
        formData = FormData::Create(*entryList, attributes.GetMethod() == Method::Get
                                                  ? FormData::EncodingType::FormURLEncoded
                                                  : FormData::ParseEncodingType(encodingType));
        if (copiedAttributes.GetMethod() == Method::Post && isMailtoForm)
        {
          // Convert the form data into a string that we put into the URL.
          AppendMailtoPostFormDataToURL(actionURL, *formData, encodingType);
          formData = FormData::Create();
        }
      }

      formData->SetIdentifier(GenerateFormDataIdentifier());
      Ref<FormData> data = formData;
      return CreateRef<FormSubmission>(copiedAttributes.GetMethod(), actionURL,
                                       form.EffectiveTarget(event, submitter.get()), encodingType,
                                       krys::move(data), boundary, event);
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

  url::URL FormSubmission::RequestURL() const noexcept
  {
    if (_method == Method::Post)
    {
      return _action;
    }

    url::URL requestURL(_action);
    if (_method == Method::Get && !requestURL.ProtocolIsJavaScript())
    {
      requestURL.SetQuery(_formData->FlattenToString());
    }
    return requestURL;
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
      // FIXME: The body and its content type, once requests carry them.
      frameRequest.GetResourceRequest().SetHTTPMethod(u8"POST");
    }

    frameRequest.GetResourceRequest().SetURL(RequestURL());
  }
}
