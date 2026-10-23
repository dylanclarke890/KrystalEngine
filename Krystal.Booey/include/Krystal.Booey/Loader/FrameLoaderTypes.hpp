#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief What the host decides to do with a navigation.
  enum class PolicyAction : uint8
  {
    /// @brief Navigate.
    Use,
    /// @brief Download the resource instead of showing it.
    Download,
    /// @brief Do nothing: the host may have handled it itself.
    Ignore,
  };

  /// @brief Why a frame is loading.
  enum class FrameLoadType : uint8
  {
    Standard,
    Back,
    Forward,
    /// @brief A hop of several items in the back and forward list.
    IndexedBackForward,
    Reload,
    /// @brief The user loads the same URL again (not the reload button).
    Same,
    RedirectWithLockedBackForwardList,
    MultipartReplace,
    ReloadFromOrigin,
    ReloadExpiredOnly,
    NavigationAPIReplace
  };

  /// @brief Whether a new browsing context opened by a navigation gets its opener.
  enum class NewFrameOpenerPolicy : uint8
  {
    Suppress,
    Allow
  };

  /// @brief What started a navigation.
  enum class NavigationType : uint8
  {
    LinkClicked,
    FormSubmitted,
    BackForward,
    Reload,
    FormResubmitted,
    Other
  };

  /// @see https://html.spec.whatwg.org/#navigationhistorybehavior
  enum class NavigationHistoryBehavior : uint8
  {
    Auto,
    Push,
    Replace,
    /// @brief Internal, not part of the specification.
    Reload
  };

  /// @brief Whether @p type reloads the current document.
  KRYS_NODISCARD constexpr bool IsReload(FrameLoadType type) noexcept
  {
    return type == FrameLoadType::Reload || type == FrameLoadType::ReloadFromOrigin
           || type == FrameLoadType::ReloadExpiredOnly;
  }

  /// @brief Whether @p type goes back or forward in the session history.
  KRYS_NODISCARD constexpr bool IsBackForwardLoadType(FrameLoadType type) noexcept
  {
    return type == FrameLoadType::Back || type == FrameLoadType::Forward
           || type == FrameLoadType::IndexedBackForward;
  }
}
