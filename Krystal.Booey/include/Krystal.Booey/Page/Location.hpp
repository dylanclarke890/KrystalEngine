#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/URLDecomposition.hpp"
#include "Krystal.Booey/Loader/FrameLoaderTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LocalDOMWindow;

  /// @brief A window's `location`: its document's URL, whose parts navigate the window when they're set.
  /// @see https://html.spec.whatwg.org/#the-location-interface
  class Location final : public RefCounted<Location>, public html::URLDecomposition
  {
    KRYS_NON_COPY_MOVABLE(Location);

  private:
    WeakPtr<LocalDOMWindow> _window;

  public:
    explicit Location(LocalDOMWindow &window) noexcept;
    ~Location() noexcept override;

    KRYS_NODISCARD static Ref<Location> Create(LocalDOMWindow &window) noexcept
    {
      return CreateRef<Location>(window);
    }

    /// @see https://html.spec.whatwg.org/#dom-location-href
    KRYS_NODISCARD dom::USVString Href() const noexcept;

    /// @brief Navigates to @p url, parsed relative to the document's base URL.
    /// @return `SyntaxError` if @p url can't be parsed.
    /// @see https://html.spec.whatwg.org/#dom-location-href
    dom::ExceptionOr<void> SetHref(utf8_stringview url) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-location-assign
    dom::ExceptionOr<void> Assign(utf8_stringview url) noexcept;

    /// @brief Navigates to @p url, replacing the current history entry.
    /// @see https://html.spec.whatwg.org/#dom-location-replace
    dom::ExceptionOr<void> Replace(utf8_stringview url) noexcept;

    /// @brief Loads the document again.
    /// @see https://html.spec.whatwg.org/#dom-location-reload
    void Reload() noexcept;

    /// @brief The origins of the frame's ancestors: none, as there's no frame tree.
    /// @see https://html.spec.whatwg.org/#dom-location-ancestororigins
    KRYS_NODISCARD List<dom::DOMString> AncestorOrigins() const noexcept
    {
      return {};
    }

    /// @see https://html.spec.whatwg.org/#dom-location-href
    KRYS_NODISCARD dom::USVString ToString() const noexcept
    {
      return Href();
    }

  private:
    KRYS_NODISCARD url::URL FullURL() const noexcept override;
    void SetFullURL(const url::URL &url) noexcept override;

    /// @see https://html.spec.whatwg.org/#location-object-navigate
    void Navigate(const url::URL &url,
                  NavigationHistoryBehavior historyHandling = NavigationHistoryBehavior::Auto) noexcept;
  };
}
