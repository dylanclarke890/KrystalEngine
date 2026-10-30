#include "Krystal.Booey/HTML/Shadow/DateTimeEditElement.hpp"
#include "Krystal.Booey/Core/DateTimeFormat.hpp"
#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/FocusEvent.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/DateTimeFieldsState.hpp"
#include "Krystal.Booey/HTML/Shadow/DateTimeFieldElements.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo::html
{
  namespace
  {
    constexpr size_t NotFound = std::numeric_limits<size_t>::max();

    class DateTimeEditBuilder final : private DateTimeFormat::TokenHandler
    {
      KRYS_NON_COPYABLE(DateTimeEditBuilder);

    private:
      Ref<DateTimeEditElement> _editElement;
      const DateTimeEditElement::LayoutParameters &_parameters;

    public:
      DateTimeEditBuilder(DateTimeEditElement &element,
                          const DateTimeEditElement::LayoutParameters &layoutParameters) noexcept
          : _editElement(ShareRef(element)), _parameters(layoutParameters)
      {
      }

      bool Build(dom::DOMStringView formatString) noexcept
      {
        _editElement->ResetFields();
        return DateTimeFormat::Parse(formatString, *this);
      }

    private:
      // DateTimeFormat::TokenHandler functions:
      void VisitField(DateTimeFormat::FieldType fieldType, int count) noexcept final
      {
        using FieldType = DateTimeFormat::FieldType;
        auto &document = _editElement->NodeDocument();
        auto &owner = static_cast<DateTimeFieldElementFieldOwner &>(*_editElement);

        switch (fieldType)
        {
          case FieldType::DayOfMonth:
            _editElement->AddField(DateTimeDayFieldElement::Create(document, owner));
            return;
          case FieldType::FractionalSecond:
            _editElement->AddField(DateTimeMillisecondFieldElement::Create(document, owner));
            return;
          case FieldType::Hour11:
            _editElement->AddField(DateTimeHourFieldElement::Create(document, owner, 0, 11));
            return;
          case FieldType::Hour12:
            _editElement->AddField(DateTimeHourFieldElement::Create(document, owner, 1, 12));
            return;
          case FieldType::Hour23:
            _editElement->AddField(DateTimeHourFieldElement::Create(document, owner, 0, 23));
            return;
          case FieldType::Hour24:
            _editElement->AddField(DateTimeHourFieldElement::Create(document, owner, 1, 24));
            return;
          case FieldType::Minute:
            _editElement->AddField(DateTimeMinuteFieldElement::Create(document, owner));
            return;

          case FieldType::Month:
          case FieldType::MonthStandAlone:
          {
            constexpr int CountForAbbreviatedMonth = 3;
            constexpr int CountForFullMonth = 4;
            constexpr int CountForNarrowMonth = 5;

            auto &locale = _parameters.FormatLocale;
            switch (count)
            {
              case CountForNarrowMonth:
              case CountForAbbreviatedMonth:
                _editElement->AddField(DateTimeSymbolicMonthFieldElement::Create(
                  document, owner,
                  fieldType == FieldType::Month ? locale.ShortMonthLabels()
                                                : locale.ShortStandAloneMonthLabels()));
                return;
              case CountForFullMonth:
                _editElement->AddField(DateTimeSymbolicMonthFieldElement::Create(
                  document, owner,
                  fieldType == FieldType::Month ? locale.MonthLabels() : locale.StandAloneMonthLabels()));
                return;
              default: _editElement->AddField(DateTimeMonthFieldElement::Create(document, owner)); return;
            }
          }

          case FieldType::Period:
            _editElement->AddField(DateTimeMeridiemFieldElement::Create(
              document, owner, _parameters.FormatLocale.TimeAMPMLabels()));
            return;

          case FieldType::Second:
            _editElement->AddField(DateTimeSecondFieldElement::Create(document, owner));

            if (_parameters.ShouldHaveMillisecondField)
            {
              VisitLiteral(_parameters.FormatLocale.LocalizedDecimalSeparator());
              VisitField(FieldType::FractionalSecond, 3);
            }
            return;

          case FieldType::WeekOfYear:
            _editElement->AddField(DateTimeWeekFieldElement::Create(document, owner));
            return;
          case FieldType::Year:
            _editElement->AddField(DateTimeYearFieldElement::Create(document, owner));
            return;

          default: return;
        }
      }

      void VisitLiteral(utf8_stringview text) noexcept final
      {
        krys_debug_assert(!text.empty());

        auto &document = _editElement->NodeDocument();
        auto element = CreateRef<HTMLDivElement>(document);
        element->SetUserAgentPart(css::UserAgentParts::WebkitDatetimeEditText());

        // If the literal begins/ends with a space, the gap between two fields will appear exaggerated due to
        // the presence of a 1px padding around each field. This can make spaces appear up to 2px larger
        // between fields. This padding is necessary to prevent selected fields from appearing squished. To
        // fix, pull fields closer together by applying a negative margin.
        if (text.starts_with(' '))
        {
          (void)element->SetInlineStyleProperty(css::PropertyId::MarginInlineStart, -1, css::UnitType::px);
        }
        if (text.ends_with(' '))
        {
          (void)element->SetInlineStyleProperty(css::PropertyId::MarginInlineEnd, -1, css::UnitType::px);
        }

        (void)element->AppendChild(*CreateRef<dom::Text>(document, dom::DOMString(text)));
        (void)_editElement->FieldsWrapperElement().AppendChild(*element);
      }
    };
  }

  DateTimeEditElement::DateTimeEditElement(dom::Document &document,
                                           DateTimeEditElementEditControlOwner &editControlOwner) noexcept
      : HTMLDivElement(document), _editControlOwner(&editControlOwner),
        _placeholderDate(
          DateComponents::FromMillisecondsSinceEpochForDate(LocalTimeNow()).value_or(DateComponents()))
  {
  }

  DateTimeEditElement::~DateTimeEditElement() noexcept
  {
    for (auto &field : _fields)
    {
      field->RemoveFieldOwner();
    }
  }

  Ref<DateTimeEditElement>
    DateTimeEditElement::Create(dom::Document &document,
                                DateTimeEditElementEditControlOwner &editControlOwner) noexcept
  {
    auto element = CreateRef<DateTimeEditElement>(document, editControlOwner);
    element->SetUserAgentPart(css::UserAgentParts::WebkitDatetimeEdit());
    return element;
  }

  dom::Element &DateTimeEditElement::FieldsWrapperElement() const noexcept
  {
    krys_debug_assert(FirstChild());
    return Downcast<dom::Element>(*FirstChild());
  }

  void DateTimeEditElement::AddField(Ref<DateTimeFieldElement> field) noexcept
  {
    if (_fields.size() == MaximumNumberOfFields)
    {
      return;
    }
    _fields.push_back(field);
    (void)FieldsWrapperElement().AppendChild(*field);
  }

  size_t DateTimeEditElement::FieldIndexOf(const DateTimeFieldElement &fieldToFind) const noexcept
  {
    auto it = std::ranges::find_if(_fields, [&](auto &field) { return field.get() == &fieldToFind; });
    return it == _fields.end() ? NotFound : static_cast<size_t>(it - _fields.begin());
  }

  void DateTimeEditElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event))
    {
      if (keyboardEvent->KeyIdentifier() == u8"U+0020" && _editControlOwner)
      {
        // Forward space keypresses to the owner to activate the date picker.
        _editControlOwner->DidReceiveSpaceKeyFromControl();
        // We want to mark the event as handled to avoid scrolling the page.
        event.SetDefaultHandled();
        return;
      }
    }

    HTMLDivElement::DefaultEventHandler(event);
  }

  DateTimeFieldElement *DateTimeEditElement::FocusedFieldElement() const noexcept
  {
    auto *focusedElement = NodeDocument().FocusedElement();
    auto it = std::ranges::find_if(_fields, [&](auto &field) { return field.get() == focusedElement; });
    return it == _fields.end() ? nullptr : it->get();
  }

  void DateTimeEditElement::Layout(const LayoutParameters &layoutParameters) noexcept
  {
    if (!FirstChild())
    {
      auto element = CreateRef<HTMLDivElement>(NodeDocument());
      (void)AppendChild(*element);
      element->SetUserAgentPart(css::UserAgentParts::WebkitDatetimeEditFieldsWrapper());
    }

    Ref<dom::Element> fieldsWrapper = ShareRef(FieldsWrapperElement());
    RefPtr<DateTimeFieldElement> focusedField = ShareRefPtr(FocusedFieldElement());

    DateTimeEditBuilder builder(*this, layoutParameters);
    RefPtr<dom::Node> lastChildToBeRemoved = ShareRefPtr(fieldsWrapper->LastChild());
    if (!builder.Build(layoutParameters.DateTimeFormat) || _fields.empty())
    {
      lastChildToBeRemoved = ShareRefPtr(fieldsWrapper->LastChild());
      builder.Build(layoutParameters.FallbackDateTimeFormat);
    }

    if (focusedField)
    {
      auto focusedFieldId = focusedField->UserAgentPart();

      auto foundFieldToFocus = false;
      for (auto &field : _fields)
      {
        if (field->UserAgentPart() == focusedFieldId)
        {
          foundFieldToFocus = true;
          field->Focus();
          break;
        }
      }

      if (!foundFieldToFocus)
      {
        FocusOnNextFocusableField(0);
      }
    }

    if (lastChildToBeRemoved)
    {
      while (RefPtr<dom::Node> childNode = ShareRefPtr(fieldsWrapper->FirstChild()))
      {
        (void)fieldsWrapper->RemoveChild(*childNode);
        if (childNode == lastChildToBeRemoved)
        {
          break;
        }
      }
    }
  }

  void DateTimeEditElement::DidBlurFromField(dom::Event &event) noexcept
  {
    if (!_editControlOwner)
    {
      return;
    }

    auto *focusEvent = DynamicDowncast<dom::FocusEvent>(event);
    if (auto newFocusedElement = focusEvent ? focusEvent->RelatedTarget() : nullptr)
    {
      bool didFocusSiblingField = std::ranges::any_of(
        _fields,
        [&](auto &field) { return static_cast<dom::EventTarget *>(field.get()) == newFocusedElement.get(); });

      if (didFocusSiblingField)
      {
        return;
      }
    }

    _editControlOwner->DidBlurFromControl();
  }

  void DateTimeEditElement::FieldValueChanged() noexcept
  {
    if (_editControlOwner)
    {
      _editControlOwner->DidChangeValueFromControl();
    }
  }

  bool DateTimeEditElement::FocusOnNextFocusableField(size_t startIndex) noexcept
  {
    for (size_t i = startIndex; i < _fields.size(); ++i)
    {
      Ref<DateTimeFieldElement> field = _fields[i];
      if (FocusAlgorithms::IsFocusableArea(*field))
      {
        field->Focus();
        return true;
      }
    }
    return false;
  }

  void DateTimeEditElement::FocusByOwner() noexcept
  {
    FocusOnNextFocusableField(0);
  }

  bool DateTimeEditElement::FocusOnNextField(const DateTimeFieldElement &field) noexcept
  {
    auto startFieldIndex = FieldIndexOf(field);
    if (startFieldIndex == NotFound)
    {
      return false;
    }

    return FocusOnNextFocusableField(startFieldIndex + 1);
  }

  bool DateTimeEditElement::FocusOnPreviousField(const DateTimeFieldElement &field) noexcept
  {
    auto startFieldIndex = FieldIndexOf(field);
    if (startFieldIndex == NotFound)
    {
      return false;
    }

    auto fieldIndex = startFieldIndex;
    while (fieldIndex > 0)
    {
      --fieldIndex;
      Ref<DateTimeFieldElement> previousField = _fields[fieldIndex];
      if (FocusAlgorithms::IsFocusableArea(*previousField))
      {
        previousField->Focus();
        return true;
      }
    }

    return false;
  }

  bool DateTimeEditElement::IsFieldOwnerDisabled() const noexcept
  {
    return _editControlOwner && _editControlOwner->IsEditControlOwnerDisabled();
  }

  bool DateTimeEditElement::IsFieldOwnerReadOnly() const noexcept
  {
    return _editControlOwner && _editControlOwner->IsEditControlOwnerReadOnly();
  }

  bool DateTimeEditElement::IsFieldOwnerHorizontal() const noexcept
  {
    if (auto *renderer = FieldsWrapperElement().Renderer())
    {
      return renderer->IsHorizontalWritingMode();
    }
    return true;
  }

  dom::DOMString DateTimeEditElement::LocaleIdentifier() const noexcept
  {
    return _editControlOwner ? _editControlOwner->LocaleIdentifier() : dom::DOMString();
  }

  const DateComponents &DateTimeEditElement::PlaceholderDate() const noexcept
  {
    return _placeholderDate;
  }

  void DateTimeEditElement::ResetFields() noexcept
  {
    // The fields being replaced are removed once the new ones are built; they no longer belong to this.
    for (auto &field : _fields)
    {
      field->RemoveFieldOwner();
    }
    _fields.clear();
  }

  void DateTimeEditElement::SetValueAsDate(const LayoutParameters &layoutParameters,
                                           const DateComponents &date) noexcept
  {
    Layout(layoutParameters);
    for (auto &field : _fields)
    {
      field->SetValueAsDate(date);
    }
  }

  void DateTimeEditElement::SetEmptyValue(const LayoutParameters &layoutParameters) noexcept
  {
    Layout(layoutParameters);
    for (auto &field : _fields)
    {
      field->SetEmptyValue();
    }
  }

  dom::DOMString DateTimeEditElement::Value() const noexcept
  {
    return _editControlOwner ? _editControlOwner->FormatDateTimeFieldsState(ValueAsDateTimeFieldsState())
                             : dom::DOMString();
  }

  dom::DOMString DateTimeEditElement::PlaceholderValue() const noexcept
  {
    return _editControlOwner ? _editControlOwner->FormatDateTimeFieldsState(
                                 ValueAsDateTimeFieldsState(DateTimePlaceholderIfNoValue::Yes))
                             : dom::DOMString();
  }

  DateTimeFieldsState DateTimeEditElement::ValueAsDateTimeFieldsState(
    DateTimePlaceholderIfNoValue placeholderIfNoValue) const noexcept
  {
    DateTimeFieldsState dateTimeFieldsState;
    for (auto &field : _fields)
    {
      field->PopulateDateTimeFieldsState(dateTimeFieldsState, placeholderIfNoValue);
    }
    return dateTimeFieldsState;
  }

  bool DateTimeEditElement::EditableFieldsHaveValues() const noexcept
  {
    return std::ranges::any_of(_fields, [](auto &field) { return field->HasValue(); });
  }
}
