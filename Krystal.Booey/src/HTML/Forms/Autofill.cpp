/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/Forms/Autofill.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::boo::html
{
  namespace
  {
    enum class AutofillCategory : uint8
    {
      Off,
      Automatic,
      Normal,
      Contact,
      Credential
    };

    struct AutofillFieldCategory
    {
      utf8_stringview Token;
      AutofillCategory Category;
    };

    constexpr Array<AutofillFieldCategory, 57> AutofillFieldCategories = {{
      {u8"off", AutofillCategory::Off},
      {u8"on", AutofillCategory::Automatic},
      {u8"name", AutofillCategory::Normal},
      {u8"honorific-prefix", AutofillCategory::Normal},
      {u8"given-name", AutofillCategory::Normal},
      {u8"additional-name", AutofillCategory::Normal},
      {u8"family-name", AutofillCategory::Normal},
      {u8"honorific-suffix", AutofillCategory::Normal},
      {u8"nickname", AutofillCategory::Normal},
      {u8"organization-title", AutofillCategory::Normal},
      {u8"username", AutofillCategory::Normal},
      {u8"new-password", AutofillCategory::Normal},
      {u8"current-password", AutofillCategory::Normal},
      {u8"one-time-code", AutofillCategory::Normal},
      {u8"organization", AutofillCategory::Normal},
      {u8"street-address", AutofillCategory::Normal},
      {u8"address-line1", AutofillCategory::Normal},
      {u8"address-line2", AutofillCategory::Normal},
      {u8"address-line3", AutofillCategory::Normal},
      {u8"address-level4", AutofillCategory::Normal},
      {u8"address-level3", AutofillCategory::Normal},
      {u8"address-level2", AutofillCategory::Normal},
      {u8"address-level1", AutofillCategory::Normal},
      {u8"country", AutofillCategory::Normal},
      {u8"country-name", AutofillCategory::Normal},
      {u8"postal-code", AutofillCategory::Normal},
      {u8"cc-name", AutofillCategory::Normal},
      {u8"cc-given-name", AutofillCategory::Normal},
      {u8"cc-additional-name", AutofillCategory::Normal},
      {u8"cc-family-name", AutofillCategory::Normal},
      {u8"cc-number", AutofillCategory::Normal},
      {u8"cc-exp", AutofillCategory::Normal},
      {u8"cc-exp-month", AutofillCategory::Normal},
      {u8"cc-exp-year", AutofillCategory::Normal},
      {u8"cc-csc", AutofillCategory::Normal},
      {u8"cc-type", AutofillCategory::Normal},
      {u8"transaction-currency", AutofillCategory::Normal},
      {u8"transaction-amount", AutofillCategory::Normal},
      {u8"language", AutofillCategory::Normal},
      {u8"bday", AutofillCategory::Normal},
      {u8"bday-day", AutofillCategory::Normal},
      {u8"bday-month", AutofillCategory::Normal},
      {u8"bday-year", AutofillCategory::Normal},
      {u8"sex", AutofillCategory::Normal},
      {u8"url", AutofillCategory::Normal},
      {u8"photo", AutofillCategory::Normal},
      {u8"tel", AutofillCategory::Contact},
      {u8"tel-country-code", AutofillCategory::Contact},
      {u8"tel-national", AutofillCategory::Contact},
      {u8"tel-area-code", AutofillCategory::Contact},
      {u8"tel-local", AutofillCategory::Contact},
      {u8"tel-local-prefix", AutofillCategory::Contact},
      {u8"tel-local-suffix", AutofillCategory::Contact},
      {u8"tel-extension", AutofillCategory::Contact},
      {u8"email", AutofillCategory::Contact},
      {u8"impp", AutofillCategory::Contact},
      {u8"webauthn", AutofillCategory::Credential},
    }};

    /// @see https://html.spec.whatwg.org/#determine-a-field's-category
    KRYS_NODISCARD Maybe<AutofillCategory> DetermineFieldCategory(dom::DOMStringView field) noexcept
    {
      // 1. If the field is not an ASCII case-insensitive match for one of the tokens given in the first
      // column
      //    of the following table, then return null.
      // 2. Return the value of the cell in the second column of that row.
      for (const auto &[token, category] : AutofillFieldCategories)
      {
        if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(field, token))
        {
          return category;
        }
      }
      return std::nullopt;
    }

    /// @brief The string in @p strings that @p token is an ASCII case-insensitive match for, if any.
    template <size_t N>
    KRYS_NODISCARD Maybe<utf8_stringview> MatchOneOf(dom::DOMStringView token,
                                                     const Array<utf8_stringview, N> &strings) noexcept
    {
      for (auto string : strings)
      {
        if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(token, string))
        {
          return string;
        }
      }
      return std::nullopt;
    }

    /// @see https://html.spec.whatwg.org/#autofill-anchor-mantle
    KRYS_NODISCARD AutofillMantle GetAutofillMantle(const HTMLFormControlElement &element) noexcept
    {
      auto *input = DynamicDowncast<HTMLInputElement>(element);
      return input && input->IsInputTypeHidden() ? AutofillMantle::Anchor : AutofillMantle::Expectation;
    }
  }

  // https://html.spec.whatwg.org/#autofill-processing-model
  AutofillData AutofillData::Create(const HTMLFormControlElement &element) noexcept
  {
    auto mantle = GetAutofillMantle(element);

    auto defaultLabel = [&]
    {
      // 31. Default: Set the element's IDL-exposed autofill value to the empty string, and its autofill hint
      // set
      //     and autofill scope to empty.
      AutofillData data;

      // 32. If the element's autocomplete attribute is wearing the autofill anchor mantle, then set the
      //     element's autofill field name to the empty string and return.
      if (mantle == AutofillMantle::Anchor)
      {
        return data;
      }

      // 33. Let form be the element's form owner, if any, or null otherwise.
      // 34. If form is not null and form's autocomplete attribute is in the Off state, then set the element's
      //     autofill field name to "off". Otherwise, set the element's autofill field name to "on".
      auto *form = element.Form();
      data.FieldName = form && form->Autocomplete() == u8"off" ? u8"off" : u8"on";
      return data;
    };

    // 1. If the element has no autocomplete attribute, then jump to the step labeled default.
    auto attribute = element.GetAttribute(u8"autocomplete");
    if (!attribute)
    {
      return defaultLabel();
    }

    // 2. Let tokens be the result of splitting the attribute's value on ASCII whitespace.
    auto tokens = infra::StringAlgorithms::SplitOnWhitespace(*attribute);

    // 3. If tokens is empty, then jump to the step labeled default.
    if (tokens.empty())
    {
      return defaultLabel();
    }

    // 4. Let index be the index of the last token in tokens.
    size_t index = tokens.size() - 1;

    // 5. Let field be the indexth token in tokens.
    dom::DOMString field = tokens[index];

    // 6. Let category be the result of determining a field's category given field.
    auto category = DetermineFieldCategory(field);

    // 7. If category is null, then jump to the step labeled default.
    if (!category)
    {
      return defaultLabel();
    }

    // 8. If category is Off or Automatic, and either tokens's size is greater than 1 or the element's
    //    autocomplete attribute is wearing the autofill anchor mantle, then jump to the step labeled default.
    if ((*category == AutofillCategory::Off || *category == AutofillCategory::Automatic)
        && (tokens.size() > 1 || mantle == AutofillMantle::Anchor))
    {
      return defaultLabel();
    }

    // 9. If category is Off, set the element's autofill field name to the string "off", set its autofill hint
    //    set to empty, and set its IDL-exposed autofill value to the string "off". Then, return.
    if (*category == AutofillCategory::Off)
    {
      return {.FieldName = u8"off", .IDLExposedValue = u8"off"};
    }

    // 10. If category is Automatic, set the element's autofill field name to the string "on", set its
    // autofill
    //     hint set to empty, and set its IDL-exposed autofill value to the string "on". Then, return.
    if (*category == AutofillCategory::Automatic)
    {
      return {.FieldName = u8"on", .IDLExposedValue = u8"on"};
    }

    // 11. Let scope tokens be an empty list.
    List<dom::DOMString> scopeTokens;

    // 12. Let hint tokens be an empty set.
    List<dom::DOMString> hintTokens;

    // 13. Let credential type be null.
    auto credentialType = NonAutofillCredentialType::None;

    // 14. Let IDL value have the same value as field.
    dom::DOMString idlValue = field;

    auto done = [&]
    {
      // 25. Done: Set the element's autofill hint set to hint tokens.
      // 26. Set the element's non-autofill credential type to credential type.
      // 27. Set the element's autofill scope to scope tokens.
      // 28. Set the element's autofill field name to field.
      // 29. Set the element's IDL-exposed autofill value to IDL value.
      // 30. Return.
      return AutofillData {.FieldName = field,
                           .HintSet = krys::move(hintTokens),
                           .Scope = krys::move(scopeTokens),
                           .CredentialType = credentialType,
                           .IDLExposedValue = krys::move(idlValue)};
    };

    // 15. If category is Credential and the indexth token in tokens is an ASCII case-insensitive match for
    //     "webauthn", then run the substeps that follow:
    if (*category == AutofillCategory::Credential
        && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(tokens[index], u8"webauthn"))
    {
      // 15a. Set credential type to "webauthn".
      credentialType = NonAutofillCredentialType::WebAuthn;

      // 15b. If the indexth token in tokens is the first entry, then skip to the step labeled done.
      if (index == 0)
      {
        return done();
      }

      // 15c. Decrement index by one.
      --index;

      // 15d. Set category to the result of determining a field's category given the indexth token in tokens.
      category = DetermineFieldCategory(tokens[index]);

      // 15e. If category is not Normal and category is not Contact, then jump to the step labeled default.
      if (category != AutofillCategory::Normal && category != AutofillCategory::Contact)
      {
        return defaultLabel();
      }

      // 15f. Set IDL value to the concatenation of the indexth token in tokens, a U+0020 SPACE character, and
      //      the previous value of IDL value.
      idlValue = tokens[index] + u8" " + idlValue;
    }

    // 16. If the indexth token in tokens is the first entry, then skip to the step labeled done.
    if (index == 0)
    {
      return done();
    }

    // 17. Decrement index by one.
    --index;

    // 18. If category is Contact and the indexth token in tokens is an ASCII case-insensitive match for one
    // of
    //     the strings in the following list: "home"; "work"; "mobile"; "fax"; "pager" then:
    constexpr Array<utf8_stringview, 5> contactTokens = {u8"home", u8"work", u8"mobile", u8"fax", u8"pager"};
    if (auto contactToken = MatchOneOf(tokens[index], contactTokens);
        category == AutofillCategory::Contact && contactToken)
    {
      // 18a. Let contact be the matching string from the list above.
      dom::DOMString contact(*contactToken);

      // 18b. Insert contact at the start of scope tokens.
      scopeTokens.insert(scopeTokens.begin(), contact);

      // 18c. Add contact to hint tokens.
      hintTokens.push_back(contact);

      // 18d. Let IDL value be the concatenation of contact, a U+0020 SPACE character, and the previous value
      // of
      //      IDL value.
      idlValue = contact + u8" " + idlValue;

      // 18e. If the indexth entry in tokens is the first entry, then skip to the step labeled done.
      if (index == 0)
      {
        return done();
      }

      // 18f. Decrement index by one.
      --index;
    }

    // 19. If the indexth token in tokens is an ASCII case-insensitive match for one of the strings in the
    //     following list: "shipping"; "billing" then:
    constexpr Array<utf8_stringview, 2> modeTokens = {u8"shipping", u8"billing"};
    if (auto modeToken = MatchOneOf(tokens[index], modeTokens))
    {
      // 19a. Let mode be the matching string from the list above.
      dom::DOMString mode(*modeToken);

      // 19b. Insert mode at the start of scope tokens.
      scopeTokens.insert(scopeTokens.begin(), mode);

      // 19c. Add mode to hint tokens.
      hintTokens.push_back(mode);

      // 19d. Let IDL value be the concatenation of mode, a U+0020 SPACE character, and the previous value of
      // IDL
      //      value.
      idlValue = mode + u8" " + idlValue;

      // 19e. If the indexth entry in tokens is the first entry, then skip to the step labeled done.
      if (index == 0)
      {
        return done();
      }

      // 19f. Decrement index by one.
      --index;
    }

    // 20. If the indexth entry in tokens is not the first entry, then jump to the step labeled default.
    if (index != 0)
    {
      return defaultLabel();
    }

    // 21. If the first eight characters of the indexth token in tokens are not an ASCII case-insensitive
    // match
    //     for the string "section-", then jump to the step labeled default.
    auto &sectionToken = tokens[index];
    if (sectionToken.size() < 8
        || !infra::StringAlgorithms::ASCIICaseInsensitiveMatch(dom::DOMStringView(sectionToken).substr(0, 8),
                                                               u8"section-"))
    {
      return defaultLabel();
    }

    // 22. Let section be the indexth token in tokens, converted to ASCII lowercase.
    auto section = text::ToASCIILower(dom::DOMStringView(sectionToken));

    // 23. Insert section at the start of scope tokens.
    scopeTokens.insert(scopeTokens.begin(), section);

    // 24. Let IDL value be the concatenation of section, a U+0020 SPACE character, and the previous value of
    // IDL
    //     value.
    idlValue = section + u8" " + idlValue;
    return done();
  }
}
