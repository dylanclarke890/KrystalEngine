#pragma once

#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class Event;
}

namespace krys::boo::html
{
  class DOMFormData;
  class HTMLFormControlElement;
  class HTMLFormElement;
}

namespace krys::boo
{
  class FrameLoadRequest;

  enum class FormSubmissionTrigger : uint8
  {
    SubmittedByJavaScript,
    NotSubmittedByJavaScript,
  };

  /// @brief A form's submission: its method, action, target, encoding type and entry list, which the host
  /// handles. Booey doesn't encode the entries: the host turns them into a request if it wants one.
  /// @see https://html.spec.whatwg.org/#form-submission-algorithm
  // FIXME: The referrer policy.
  class FormSubmission : public RefCounted<FormSubmission>, public CanMakeWeakPtr<FormSubmission>
  {
  public:
    enum class Method : uint8
    {
      Get,
      Post,
      Dialog
    };

    /// @brief The submission attributes of a form, which a submitter's own attributes override.
    class Attributes
    {
    private:
      Method _method {Method::Get};
      dom::DOMString _action;
      dom::DOMString _target;
      dom::DOMString _encodingType {u8"application/x-www-form-urlencoded"};
      dom::DOMString _acceptCharset;

    public:
      KRYS_NODISCARD Method GetMethod() const noexcept
      {
        return _method;
      }

      KRYS_NODISCARD static Method ParseMethodType(utf8_stringview type) noexcept;
      void UpdateMethodType(utf8_stringview type) noexcept;
      KRYS_NODISCARD static utf8_stringview MethodString(Method method) noexcept;

      KRYS_NODISCARD const dom::DOMString &Action() const noexcept
      {
        return _action;
      }

      void ParseAction(utf8_stringview action) noexcept;

      KRYS_NODISCARD const dom::DOMString &Target() const noexcept
      {
        return _target;
      }

      void SetTarget(utf8_stringview target) noexcept
      {
        _target = dom::DOMString(target);
      }

      KRYS_NODISCARD const dom::DOMString &EncodingType() const noexcept
      {
        return _encodingType;
      }

      KRYS_NODISCARD static utf8_stringview ParseEncodingType(utf8_stringview type) noexcept;
      void UpdateEncodingType(utf8_stringview type) noexcept;

      KRYS_NODISCARD const dom::DOMString &AcceptCharset() const noexcept
      {
        return _acceptCharset;
      }

      void SetAcceptCharset(utf8_stringview value) noexcept
      {
        _acceptCharset = dom::DOMString(value);
      }
    };

  private:
    Method _method;
    bool _wasCancelled {false};
    url::URL _action;
    dom::DOMString _target;
    dom::DOMString _encodingType;
    RefPtr<html::DOMFormData> _entryList;
    RefPtr<dom::Event> _event;
    Maybe<dom::DOMString> _returnValue; // for form[method=dialog]
    RefPtr<html::HTMLFormElement> _form;
    RefPtr<html::HTMLFormControlElement> _submitter;
    FormSubmissionTrigger _trigger {FormSubmissionTrigger::NotSubmittedByJavaScript};

  public:
    /// @brief A dialog's submission, with the submitter's result.
    FormSubmission(Maybe<dom::DOMString> returnValue, const url::URL &action, dom::DOMStringView target,
                   dom::DOMStringView encodingType, dom::Event *event) noexcept;

    /// @brief A `GET` or `POST` submission of @p entryList.
    FormSubmission(Method method, const url::URL &action, dom::DOMStringView target,
                   dom::DOMStringView encodingType, Ref<html::DOMFormData> &&entryList,
                   dom::Event *event) noexcept;
    ~FormSubmission() noexcept;

    KRYS_NODISCARD static Ref<FormSubmission> Create(html::HTMLFormElement &form,
                                                     html::HTMLFormControlElement *overrideSubmitter,
                                                     const Attributes &attributes, dom::Event *event,
                                                     FormSubmissionTrigger trigger) noexcept;

    KRYS_NODISCARD Method GetMethod() const noexcept
    {
      return _method;
    }

    KRYS_NODISCARD const url::URL &Action() const noexcept
    {
      return _action;
    }

    KRYS_NODISCARD const dom::DOMString &Target() const noexcept
    {
      return _target;
    }

    /// @brief The form's encoding type (`enctype`, or the submitter's `formenctype`), for the host to encode
    /// the entries with: `application/x-www-form-urlencoded`, `multipart/form-data` or `text/plain`.
    KRYS_NODISCARD const dom::DOMString &EncodingType() const noexcept
    {
      return _encodingType;
    }

    /// @brief The entries submitted, in tree order; null for a dialog.
    KRYS_NODISCARD html::DOMFormData *EntryList() const noexcept;

    KRYS_NODISCARD dom::Event *Event() const noexcept
    {
      return _event.get();
    }

    KRYS_NODISCARD const Maybe<dom::DOMString> &ReturnValue() const noexcept
    {
      return _returnValue;
    }

    /// @brief The submitted form.
    KRYS_NODISCARD html::HTMLFormElement *Form() const noexcept;

    /// @brief The submit button that submitted the form, if any.
    KRYS_NODISCARD html::HTMLFormControlElement *Submitter() const noexcept;

    KRYS_NODISCARD FormSubmissionTrigger Trigger() const noexcept
    {
      return _trigger;
    }

    /// @brief Gives @p frameRequest the submission's target, method and action.
    void PopulateFrameLoadRequest(FrameLoadRequest &frameRequest) const noexcept;

    void ClearTarget() noexcept
    {
      _target = {};
    }

    void Cancel() noexcept
    {
      _wasCancelled = true;
    }

    KRYS_NODISCARD bool WasCancelled() const noexcept
    {
      return _wasCancelled;
    }
  };
}
