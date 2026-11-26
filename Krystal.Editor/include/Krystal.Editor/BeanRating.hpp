#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Web/Dom/Collections/DOMTokenList.hpp"
#include "Krystal.Web/Dom/Events/EventListenerHandle.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Web/Events/UI/KeyboardEvent.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElement.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Web/Html/CustomElement/CustomStateSet.hpp"
#include "Krystal.Web/Html/CustomElement/ElementInternals.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include <algorithm>
#include <charconv>

namespace krys
{
  /// @brief `<bean-rating>`, the editor page's custom element: a rating of one to five coffee beans, chosen
  /// with a click or the arrow keys. It's form-associated (it submits its rating under its `name`, is invalid
  /// while unrated if it's `required`, is cleared by its form's reset and disabled with its fieldset) and has
  /// the custom state `rated`, which the page styles with `:state(rated)`.
  class BeanRating final : public web::html::CustomElement
  {
    constexpr static int32 MaxRating = 5;

  private:
    RefPtr<web::html::ElementInternals> _internals;
    List<Ref<web::Element>> _beans;
    int32 _rating {0};
    bool _disabled {false};

  public:
    explicit BeanRating(web::HTMLElement &element) noexcept : CustomElement(element)
    {
      auto internals = element.AttachInternals();
      krys_assert(!internals.HasException());
      _internals = internals.ReleaseValue();

      // The beans are in a closed shadow root, which only the internals reach.
      auto shadowRoot = element.AttachShadow({.Mode = web::dom::ShadowRootMode::Closed});
      krys_assert(!shadowRoot.HasException());
      auto &document = element.NodeDocument();

      auto style = document.CreateElement(u8"style").ReleaseValue();
      (void)style->TextContent(web::DOMString(Styles));
      (void)shadowRoot.Value()->AppendChild(*style);

      for (int32 bean = 0; bean < MaxRating; ++bean)
      {
        auto span = document.CreateElement(u8"span").ReleaseValue();
        (void)span->SetAttribute(u8"class", u8"bean");
        (void)shadowRoot.Value()->AppendChild(*span);
        // The beans go with the element, and their listeners with them: the target keeps them.
        span
          ->AddEventListener<web::MouseEvent>(u8"click",
                                              [this, bean](web::MouseEvent &) { Choose(bean + 1); })
          .Release();
        _beans.push_back(krys::move(span));
      }

      element
        .AddEventListener<web::KeyboardEvent>(u8"keydown",
                                              [this](web::KeyboardEvent &event) { HandleKey(event); })
        .Release();

      Update();
    }

    /// @brief The options to define it with.
    KRYS_NODISCARD static web::html::CustomElementOptions Options() noexcept
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

    void AttributeChangedCallback(const web::DOMStringAtom &localName, const web::DOMStringAtom &,
                                  const web::DOMStringAtom &newValue,
                                  const web::DOMStringAtom &) noexcept override
    {
      if (localName == u8"value")
      {
        _rating = Parse(newValue);
      }
      Update();
    }

    void FormResetCallback() noexcept override
    {
      _rating = Parse(web::DOMStringAtom(Element().GetAttribute(u8"value").value_or(u8"")));
      Update();
    }

    void FormDisabledCallback(bool disabled) noexcept override
    {
      _disabled = disabled;
    }

  private:
    constexpr static utf8_stringview Styles = u8R"(
      :host { display: inline-flex; gap: 5px; align-items: center; padding: 3px 6px; border-radius: 6px;
              cursor: pointer; }
      :host(:focus-visible) { outline: 2px solid #4f6df5; }
      :host(:disabled) { opacity: 0.45; cursor: default; }
      .bean { display: block; width: 11px; height: 17px; border-radius: 50%; border: 1px solid #8a5a3c;
              background-color: #f1e7de; transform: rotate(28deg); }
      .bean.filled { background-color: #6f4326; }
    )";

    KRYS_NODISCARD static int32 Parse(const web::DOMStringAtom &value) noexcept
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

    void HandleKey(web::KeyboardEvent &event) noexcept
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
        (void)_beans[static_cast<size_t>(bean)]->ClassList().Toggle(web::DOMString(u8"filled"),
                                                                    bean < _rating);
      }

      if (_rating > 0)
      {
        (void)_internals->SetFormValue(web::DOMString(utf8_string(1, static_cast<char8_t>(u8'0' + _rating))));
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
