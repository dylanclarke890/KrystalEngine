#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Utils/NativeHandle.hpp"
#include "Krystal.PAL/Pointer.hpp"

namespace krys::pal
{
  /// @brief Initialises platform-specific functionality.
  void Initialise() noexcept;

  /// @brief Shuts down platform-specific functionality.
  void Shutdown() noexcept;

  /// @brief Gets dpi of the given window.
  KRYS_NODISCARD int GetDPIForWindow(NativeHandle windowHandle = {nullptr}) noexcept;

  KRYS_NODISCARD NativeHandle GetActiveWindow() noexcept;

  /// @brief Sets the timer precision for the application.
  /// @param min The requested precision. nullopt can be passed to request the lowest possible
  /// precision.
  /// @returns The actual timer precision set, in milliseconds.
  uint SetTimerPrecision(Maybe<uint> min = null) noexcept;

  /// @brief Sleep for the specified number of milliseconds.
  void Sleep(milliseconds duration) noexcept;

  /// @brief How far the user's system settings say a notch of the mouse wheel scrolls.
  struct WheelScrollSettings
  {
    /// @brief Lines per notch of the wheel, or none to scroll a page per notch.
    Maybe<uint32> Lines {3};
    /// @brief Characters per notch of the horizontal wheel.
    uint32 Chars {3};
  };

  /// @brief Gets the system's wheel scroll settings, read when the user last changed them.
  KRYS_NODISCARD WheelScrollSettings GetWheelScrollSettings() noexcept;

  /// @brief Gets the pointing devices the system has: a mouse, a pen, a touch screen and how many touches it
  /// takes.
  KRYS_NODISCARD PointingDevices GetPointingDevices() noexcept;

  /// @brief Puts @p text (UTF-8, with LF line breaks) on the system clipboard, replacing what's there.
  /// @param windowHandle The window that owns the clipboard's contents; the active window if null.
  /// @return Whether the clipboard took the text.
  bool SetClipboardText(utf8_stringview text, NativeHandle windowHandle = {nullptr}) noexcept;

  /// @brief Gets the system clipboard's text, as UTF-8 with LF line breaks.
  /// @return The text, or none if the clipboard holds no text or can't be opened.
  KRYS_NODISCARD Maybe<utf8_string> GetClipboardText() noexcept;

  /// @brief What can be put on the system clipboard at once.
  struct ClipboardContent
  {
    /// @brief UTF-8 text, with LF line breaks.
    Maybe<utf8_string> Text;
    /// @brief UTF-8 markup, put on the clipboard as a fragment of an HTML document.
    Maybe<utf8_string> HTML;
    /// @brief Bytes in formats of the application's own, by the formats' names.
    List<Pair<utf8_string, List<uint8>>> Data;
  };

  /// @brief Puts @p content on the system clipboard, replacing what's there.
  /// @param windowHandle The window that owns the clipboard's contents; the active window if null.
  /// @return Whether the clipboard took all of it.
  bool SetClipboardContent(const ClipboardContent &content, NativeHandle windowHandle = {nullptr}) noexcept;

  /// @brief Empties the system clipboard.
  /// @param windowHandle The window that owns the clipboard afterwards; the active window if null.
  /// @return Whether the clipboard was emptied.
  bool ClearClipboard(NativeHandle windowHandle = {nullptr}) noexcept;

  /// @brief The formats of the system clipboard Krystal reads.
  enum class ClipboardFormat : uint8
  {
    Text,
    HTML,
    Files
  };

  /// @brief Whether the system clipboard holds @p format.
  KRYS_NODISCARD bool IsClipboardFormatAvailable(ClipboardFormat format) noexcept;
  /// @brief Whether the system clipboard holds bytes in the application's format named @p formatName.
  KRYS_NODISCARD bool IsClipboardDataAvailable(utf8_stringview formatName) noexcept;

  /// @brief Gets the system clipboard's HTML fragment, as UTF-8 markup.
  /// @return The markup, or none if the clipboard holds no HTML or can't be opened.
  KRYS_NODISCARD Maybe<utf8_string> GetClipboardHTML() noexcept;
  /// @brief Gets the paths of the files on the system clipboard (copied in a file manager, say).
  KRYS_NODISCARD List<utf8_string> GetClipboardFiles() noexcept;
  /// @brief Gets the system clipboard's bytes in the application's format named @p formatName.
  /// @return The bytes, or none if the clipboard doesn't hold that format or can't be opened.
  KRYS_NODISCARD Maybe<List<uint8>> GetClipboardData(utf8_stringview formatName) noexcept;
  /// @brief A count that changes whenever the system clipboard's contents do.
  KRYS_NODISCARD uint32 GetClipboardChangeCount() noexcept;
}
