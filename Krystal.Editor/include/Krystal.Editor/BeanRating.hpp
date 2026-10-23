#pragma once

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElement.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Booey/HTML/CustomStateSet.hpp"
#include "Krystal.Booey/HTML/ElementInternals.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <algorithm>
#include <charconv>

namespace krys
{
  /// @brief `<bean-rating>`, the editor page's custom element: a rating of one to five coffee beans, chosen
  /// with a click or the arrow keys. It's form-associated (it submits its rating under its `name`, is invalid
  /// while unrated if it's `required`, is cleared by its form's reset and disabled with its fieldset) and has
  /// the custom state `rated`, which the page styles with `:state(rated)`.
  class BeanRating final : public boo::html::CustomElement
  {
    static constexpr int32 MaxRating = 5;

  private:
    RefPtr<boo::html::ElementInternals> _internals;
    List<Ref<boo::dom::Element>> _beans;
    int32 _rating {0};
    bool _disabled {false};

  public:
    explicit BeanRating(boo::html::HTMLElement &element) noexcept : CustomElement(element)
    {
      auto internals = element.AttachInternals();
      krys_assert(!internals.HasException());
      _internals = internals.ReleaseValue();

      // The beans are in a closed shadow root, which only the internals reach.
      auto shadowRoot = element.AttachShadow({.Mode = boo::dom::ShadowRootMode::Closed});
      krys_assert(!shadowRoot.HasException());
      auto &document = element.NodeDocument();

      auto style = document.CreateElement(u8"style").ReleaseValue();
      (void)style->TextContent(boo::dom::DOMString(Styles));
      (void)shadowRoot.Value()->AppendChild(*style);

      for (int32 bean = 0; bean < MaxRating; ++bean)
      {
        auto span = document.CreateElement(u8"span").ReleaseValue();
        (void)span->SetAttribute(u8"class", u8"bean");
        (void)shadowRoot.Value()->AppendChild(*span);
        // The beans go with the element, and their listeners with them: the target keeps them.
        span
          ->AddEventListener<boo::dom::MouseEvent>(u8"click",
                                                   [this, bean](boo::dom::MouseEvent &) { Choose(bean + 1); })
          .Release();
        _beans.push_back(krys::move(span));
      }

      element
        .AddEventListener<boo::dom::KeyboardEvent>(u8"keydown", [this](boo::dom::KeyboardEvent &event)
                                                   { HandleKey(event); })
        .Release();

      Update();
    }

    /// @brief The options to define it with.
    KRYS_NODISCARD static boo::html::CustomElementOptions Options() noexcept
    {
      return {.ObservedAttributes = {u8"value", u8"required"}, .FormAssociated = true};
    }

    void ConnectedCallback() noexcept override
    {
      // Focusable, so the arrow keys reach it.
      if (!Element().HasAttribute(u8"tabindex"))
      {
        (void)Element().SetAttribute(u8"tabindex", u8"0");
      }
    }

    void AttributeChangedCallback(boo::dom::DOMStringAtom localName, boo::dom::DOMStringAtom,
                                  boo::dom::DOMStringAtom newValue, boo::dom::DOMStringAtom) noexcept override
    {
      if (localName == u8"value")
      {
        _rating = Parse(newValue);
      }
      Update();
    }

    void FormResetCallback() noexcept override
    {
      _rating = Parse(boo::dom::DOMStringAtom(Element().GetAttribute(u8"value").value_or(u8"")));
      Update();
    }

    void FormDisabledCallback(bool disabled) noexcept override
    {
      _disabled = disabled;
    }

  private:
    static constexpr utf8_stringview Styles = u8R"(
      :host { display: inline-flex; gap: 5px; align-items: center; padding: 3px 6px; border-radius: 6px;
              cursor: pointer; }
      :host(:focus-visible) { outline: 2px solid #4f6df5; }
      :host(:disabled) { opacity: 0.45; cursor: default; }
      .bean { display: block; width: 11px; height: 17px; border-radius: 50%; border: 1px solid #8a5a3c;
              background-color: #f1e7de; transform: rotate(28deg); }
      .bean.filled { background-color: #6f4326; }
    )";

    KRYS_NODISCARD static int32 Parse(boo::dom::DOMStringAtom value) noexcept
    {
      auto text = value.View();
      int32 rating = 0;
      std::from_chars(reinterpret_cast<const char *>(text.data()),
                      reinterpret_cast<const char *>(text.data() + text.size()), rating);
      return std::clamp(rating, 0, MaxRating);
    }

    void Choose(int32 rating) noexcept
    {
      if (_disabled)
      {
        return;
      }
      _rating = std::clamp(rating, 0, MaxRating);
      Update();
    }

    void HandleKey(boo::dom::KeyboardEvent &event) noexcept
    {
      auto key = event.Key();
      if (key == u8"ArrowRight" || key == u8"ArrowUp")
      {
        Choose(_rating + 1);
      }
      else if (key == u8"ArrowLeft" || key == u8"ArrowDown")
      {
        Choose(_rating - 1);
      }
      else if (key.size() == 1 && key[0] >= u8'0' && key[0] <= u8'5')
      {
        Choose(key[0] - u8'0');
      }
      else
      {
        return;
      }
      event.PreventDefault();
    }

    /// @brief Shows the rating, and gives the form its value, validity and state.
    void Update() noexcept
    {
      for (int32 bean = 0; bean < MaxRating; ++bean)
      {
        (void)_beans[static_cast<size_t>(bean)]->ClassList().Toggle(boo::dom::DOMString(u8"filled"),
                                                                    bean < _rating);
      }

      if (_rating > 0)
      {
        (void)_internals->SetFormValue(
          boo::dom::DOMString(utf8_string(1, static_cast<char8_t>(u8'0' + _rating))));
        (void)_internals->States().Add(u8"rated");
      }
      else
      {
        (void)_internals->SetFormValue(nullptr);
        (void)_internals->States().Delete(u8"rated");
      }

      if (_rating == 0 && Element().HasAttribute(u8"required"))
      {
        (void)_internals->SetValidity({.ValueMissing = true}, u8"Rate your last cup.");
      }
      else
      {
        (void)_internals->SetValidity();
      }
    }
  };
}
