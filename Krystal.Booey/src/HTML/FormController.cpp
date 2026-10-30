#include "Krystal.Booey/HTML/FormController.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Booey/HTML/ValidatedFormListedElement.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/Queue.hpp"
#include <charconv>
#include <functional>

namespace krys::boo::html
{
  HTMLFormElement *FormController::OwnerForm(const FormListedElement &control) noexcept
  {
    // Assume controls with form attribute have no owners because we restore state during parsing and form
    // owners of such controls might be indeterminate.
    return control.AsHTMLElement().HasAttribute(u8"form") ? nullptr : control.Form();
  }

  namespace
  {
    struct StringVectorReader
    {
      const List<dom::DOMString> &Vector;
      size_t Index {0};

      /// @brief The next string; null at the end.
      KRYS_NODISCARD Maybe<dom::DOMStringView> ConsumeString() noexcept
      {
        if (Index == Vector.size())
        {
          return null;
        }
        return dom::DOMStringView(Vector[Index++]);
      }

      KRYS_NODISCARD List<dom::DOMString> ConsumeSubvector(size_t subvectorSize) noexcept
      {
        if (subvectorSize > Vector.size() - Index)
        {
          return {};
        }
        auto subvectorIndex = Index;
        Index += subvectorSize;
        return List<dom::DOMString>(Vector.begin() + static_cast<ptrdiff_t>(subvectorIndex),
                                    Vector.begin() + static_cast<ptrdiff_t>(subvectorIndex + subvectorSize));
      }
    };

    KRYS_NODISCARD Maybe<size_t> ParseSize(Maybe<dom::DOMStringView> string) noexcept
    {
      if (!string)
      {
        return null;
      }
      size_t value = 0;
      auto *begin = reinterpret_cast<const char *>(string->data());
      auto *end = begin + string->size();
      auto [pointer, error] = std::from_chars(begin, end, value);
      if (error != std::errc() || pointer != end)
      {
        return null;
      }
      return value;
    }

    KRYS_NODISCARD dom::DOMString SizeString(size_t size) noexcept
    {
      auto string = std::to_string(size);
      return dom::DOMString(string.begin(), string.end());
    }

    // ----------------------------------------------------------------------------

    // Serialized form of FormControlState:
    //  (',' means strings around it are separated in stateVector.)
    //
    // SerializedControlState ::= SkipState | RestoreState
    // SkipState ::= '0'
    // RestoreState ::= UnsignedNumber, ControlValue+
    // UnsignedNumber ::= [0-9]+
    // ControlValue ::= arbitrary string
    //
    // The UnsignedNumber in RestoreState is the length of the sequence of ControlValues.

    void AppendSerializedFormControlState(List<dom::DOMString> &vector,
                                          const FormControlState &state) noexcept
    {
      vector.push_back(SizeString(state.size()));
      for (auto &value : state)
      {
        vector.push_back(value);
      }
    }

    KRYS_NODISCARD Maybe<FormControlState>
      ConsumeSerializedFormControlState(StringVectorReader &reader) noexcept
    {
      auto sizeString = reader.ConsumeString();
      if (!sizeString)
      {
        return null;
      }
      return reader.ConsumeSubvector(ParseSize(sizeString).value_or(0));
    }
  }

  // ----------------------------------------------------------------------------

  class FormController::SavedFormState
  {
  public:
    using FormElementKey = Pair<dom::DOMString, dom::DOMString>;

  private:
    struct FormElementKeyHash
    {
      KRYS_NODISCARD size_t operator()(const FormElementKey &key) const noexcept
      {
        return UnsignedIntegerPairHash(std::hash<dom::DOMString>()(key.first),
                                       std::hash<dom::DOMString>()(key.second));
      }
    };

    HashMap<FormElementKey, Queue<FormControlState>, FormElementKeyHash> _map;

  public:
    KRYS_NODISCARD static SavedFormState ConsumeSerializedState(StringVectorReader &reader) noexcept
    {
      auto isNotFormControlTypeCharacter = [](char8 character)
      {
        return !(character == u8'-' || text::IsASCIILower(character));
      };

      SavedFormState result;
      auto count = ParseSize(reader.ConsumeString()).value_or(0);
      while (count--)
      {
        auto name = reader.ConsumeString();
        auto type = reader.ConsumeString();
        if (!type || type->empty() || std::ranges::any_of(*type, isNotFormControlTypeCharacter))
        {
          return {};
        }
        auto state = ConsumeSerializedFormControlState(reader);
        if (!state)
        {
          return {};
        }
        result._map[{dom::DOMString(name.value_or(dom::DOMStringView())), dom::DOMString(*type)}].push(
          krys::move(*state));
      }
      return result;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _map.empty();
    }

    KRYS_NODISCARD FormControlState TakeControlState(const FormElementKey &key) noexcept
    {
      auto iterator = _map.find(key);
      if (iterator == _map.end())
      {
        return {};
      }
      auto state = krys::move(iterator->second.front());
      iterator->second.pop();
      if (iterator->second.empty())
      {
        _map.erase(iterator);
      }
      return state;
    }
  };

  // ----------------------------------------------------------------------------

  class FormController::FormKeyGenerator
  {
    KRYS_NON_COPY_MOVABLE(FormKeyGenerator);

  private:
    HashMap<const HTMLFormElement *, dom::DOMString> _formToKeyMap;
    HashMap<dom::DOMString, uint32> _formSignatureToNextIndexMap;

  public:
    FormKeyGenerator() noexcept = default;

    KRYS_NODISCARD dom::DOMString FormKey(const ValidatedFormListedElement &control) noexcept;

    void WillDeleteForm(HTMLFormElement &form) noexcept
    {
      _formToKeyMap.erase(&form);
    }
  };

  namespace
  {
    KRYS_NODISCARD bool ShouldBeUsedForFormSignature(dom::Element &element) noexcept
    {
      if (auto *formControl = DynamicDowncast<HTMLFormControlElement>(element))
      {
        return formControl->ShouldSaveAndRestoreFormControlState();
      }
      if (auto *customElement = DynamicDowncast<HTMLMaybeFormAssociatedCustomElement>(element))
      {
        return customElement->HasFormAssociatedInterface() || element.IsCustomElementUpgradeCandidate();
      }
      return false;
    }

    KRYS_NODISCARD dom::DOMString FormSignature(const HTMLFormElement &form) noexcept
    {
      // Remove the query part because it might contain volatile parameters such as a session key.
      // FIXME: But leave the fragment identifier? Perhaps we should switch to
      // removeQueryAndFragmentIdentifier.
      auto actionURL =
        form.NodeDocument().CompleteURL(form.GetAttribute(u8"action").value_or(dom::DOMString()));
      actionURL.SetQuery({});
      dom::DOMString builder(actionURL.String());

      // Two named controls seems to be enough to distinguish similar but different forms.
      constexpr uint32 maxNamedControlsToBeRecorded = 2;

      uint32 count = 0;
      builder.append(u8" [");
      for (auto *node = dom::TreeQueries::Next(form, &form); node;
           node = dom::TreeQueries::Next(*node, &form))
      {
        auto *element = DynamicDowncast<dom::Element>(node);
        if (!element || !ShouldBeUsedForFormSignature(*element) || element->HasAttribute(u8"form"))
        {
          continue;
        }
        auto name = element->GetAttribute(u8"name");
        if (!name || name->empty())
        {
          continue;
        }
        builder.append(*name);
        builder.push_back(u8' ');
        if (++count >= maxNamedControlsToBeRecorded)
        {
          break;
        }
      }
      builder.push_back(u8']');
      return builder;
    }
  }

  dom::DOMString FormController::FormKeyGenerator::FormKey(const ValidatedFormListedElement &control) noexcept
  {
    auto *form = OwnerForm(control);
    if (!form)
    {
      return dom::DOMString(u8"No owner");
    }
    auto iterator = _formToKeyMap.find(form);
    if (iterator == _formToKeyMap.end())
    {
      auto signature = FormSignature(*form);
      auto nextIndex = _formSignatureToNextIndexMap[signature]++;
      auto key = signature + u8" #" + SizeString(nextIndex);
      iterator = _formToKeyMap.emplace(form, krys::move(key)).first;
    }
    return iterator->second;
  }

  // ----------------------------------------------------------------------------

  FormController::FormController() noexcept = default;

  FormController::~FormController() noexcept = default;

  namespace
  {
    // In the legacy version of serialized state, the first item was a name attribute value of a form control.
    // The following string literal contains some characters which are rarely used for name attribute values
    // so it won't match.
    constexpr utf8_stringview FormStateSignature = u8"\n\r?% Booey serialized form state version 1 \n\r=&";
  }

  List<dom::DOMString> FormController::FormElementsState(const dom::Document &document) const noexcept
  {
    List<Pair<dom::DOMString, List<const ValidatedFormListedElement *>>> formKeyToControlsMap;

    {
      // FIXME: We should be saving the state of form controls in shadow trees, too.
      FormKeyGenerator keyGenerator;
      for (auto *node = dom::TreeQueries::Next(document); node; node = dom::TreeQueries::Next(*node))
      {
        auto *element = DynamicDowncast<HTMLElement>(node);
        auto *control = element ? element->AsValidatedFormListedElement() : nullptr;
        if (!control || !control->IsCandidateForSavingAndRestoringState())
        {
          continue;
        }

        auto formKey = keyGenerator.FormKey(*control);
        auto entry =
          std::ranges::find(formKeyToControlsMap, formKey, [](auto &entry) { return entry.first; });
        if (entry == formKeyToControlsMap.end())
        {
          entry = formKeyToControlsMap.insert(formKeyToControlsMap.end(), {formKey, {}});
        }
        entry->second.push_back(control);
      }
    }

    if (formKeyToControlsMap.empty())
    {
      return {};
    }

    List<dom::DOMString> stateVector;
    stateVector.push_back(dom::DOMString(FormStateSignature));

    for (const auto &[formKey, controls] : formKeyToControlsMap)
    {
      stateVector.push_back(formKey);
      stateVector.push_back(SizeString(controls.size()));

      for (const auto *control : controls)
      {
        stateVector.push_back(control->Name());
        stateVector.push_back(control->FormControlType());
        AppendSerializedFormControlState(stateVector, control->SaveFormControlState());
      }
    }

    stateVector.shrink_to_fit();
    return stateVector;
  }

  void FormController::SetStateForNewFormElements(const List<dom::DOMString> &stateVector) noexcept
  {
    _savedFormStateMap = CreateUnique<SavedFormStateMap>(ParseStateVector(stateVector));
  }

  FormControlState FormController::TakeStateForFormElement(const ValidatedFormListedElement &control) noexcept
  {
    if (!HasFormStateToRestore())
    {
      return {};
    }
    if (!_formKeyGenerator)
    {
      _formKeyGenerator = CreateUnique<FormKeyGenerator>();
    }
    auto iterator = _savedFormStateMap->find(_formKeyGenerator->FormKey(control));
    if (iterator == _savedFormStateMap->end())
    {
      return {};
    }
    auto state = iterator->second.TakeControlState({control.Name(), control.FormControlType()});
    if (iterator->second.IsEmpty())
    {
      _savedFormStateMap->erase(iterator);
    }
    return state;
  }

  FormController::SavedFormStateMap
    FormController::ParseStateVector(const List<dom::DOMString> &stateVector) noexcept
  {
    StringVectorReader reader {stateVector};

    if (reader.ConsumeString() != FormStateSignature)
    {
      return {};
    }

    SavedFormStateMap map;
    while (true)
    {
      auto formKey = reader.ConsumeString();
      if (!formKey)
      {
        return map;
      }
      auto state = SavedFormState::ConsumeSerializedState(reader);
      if (state.IsEmpty())
      {
        return {};
      }
      map.emplace(dom::DOMString(*formKey), krys::move(state));
    }
  }

  void FormController::WillDeleteForm(HTMLFormElement &form) noexcept
  {
    if (_formKeyGenerator)
    {
      _formKeyGenerator->WillDeleteForm(form);
    }
  }

  void FormController::RestoreControlStateFor(ValidatedFormListedElement &control) noexcept
  {
    // We don't save state of a control when shouldSaveAndRestoreFormControlState() is false. But we need to
    // skip restoring process too because a control in another form might have the same pair of name and type
    // and saved its state.
    if (!control.IsCandidateForSavingAndRestoringState())
    {
      return;
    }
    auto state = TakeStateForFormElement(control);
    if (!state.empty())
    {
      control.RestoreFormControlState(state);
    }
  }

  void FormController::RestoreControlStateIn(HTMLFormElement &form) noexcept
  {
    for (auto &element : form.CopyValidatedListedElementsVector())
    {
      auto *control = element->AsValidatedFormListedElement();
      if (!control || !control->IsCandidateForSavingAndRestoringState() || OwnerForm(*control) != &form)
      {
        continue;
      }
      auto state = TakeStateForFormElement(*control);
      if (!state.empty())
      {
        control->RestoreFormControlState(state);
      }
    }
  }

  bool FormController::HasFormStateToRestore() const noexcept
  {
    return _savedFormStateMap && !_savedFormStateMap->empty();
  }
}
