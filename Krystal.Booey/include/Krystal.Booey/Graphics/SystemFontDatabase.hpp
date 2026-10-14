#pragma once

#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  /// @brief The platform's fonts for the system font keywords of the `font` shorthand (`caption`, `menu`,
  /// ...).
  class SystemFontDatabase
  {
    KRYS_NON_COPY_MOVABLE(SystemFontDatabase);

  public:
    enum class FontShorthand : uint8
    {
      // This needs to be kept in sync with the system font keywords and `css::LowerFontShorthand()`.
      Caption,
      Icon,
      Menu,
      MessageBox,
      SmallCaption,
      StatusBar, // This has to be kept in sync with SystemFontShorthandCache below.
    };

  private:
    struct SystemFontShorthandInfo
    {
      StringAtom Family;
      float Size;
      FontSelectionValue Weight;
    };

    using SystemFontShorthandCache =
      Array<Maybe<SystemFontShorthandInfo>, static_cast<size_t>(FontShorthand::StatusBar) + 1>;
    SystemFontShorthandCache _systemFontShorthandCache;

  public:
    KRYS_NODISCARD static SystemFontDatabase &Singleton() noexcept;

    KRYS_NODISCARD const StringAtom &SystemFontShorthandFamily(FontShorthand) noexcept;
    KRYS_NODISCARD float SystemFontShorthandSize(FontShorthand) noexcept;
    KRYS_NODISCARD FontSelectionValue SystemFontShorthandWeight(FontShorthand) noexcept;

  protected:
    SystemFontDatabase() noexcept = default;

  private:
    void Invalidate() noexcept;
    void PlatformInvalidate() noexcept;

    KRYS_NODISCARD const SystemFontShorthandInfo &GetSystemFontShorthandInfo(FontShorthand) noexcept;
    KRYS_NODISCARD static SystemFontShorthandInfo PlatformSystemFontShorthandInfo(FontShorthand) noexcept;
  };
}
