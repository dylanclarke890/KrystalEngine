#pragma once

#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Text/FreeTypeFace.hpp"
#include <filesystem>

namespace krys::boo
{
  /// @brief The weights, widths and slopes a face provides, for font matching: its own style.
  KRYS_NODISCARD FontSelectionCapabilities CapabilitiesOfFace(const text::FreeTypeFace &face) noexcept;

  /// @brief The fonts documents can use: the faces of the font files the host registers, by family, and the
  /// bundled Liberation Sans, the last resort.
  /// @note The database is process-wide. Families are matched ASCII case-insensitively.
  class FontDatabase final
  {
    KRYS_NON_COPY_MOVABLE(FontDatabase);

  private:
    List<Ref<text::FreeTypeFace>> _faces;
    // Lowercased family name to the faces of the family, in registration order.
    HashMap<utf8_string, List<Ref<text::FreeTypeFace>>> _families;
    List<std::filesystem::path> _registeredFiles;
    Ref<text::FreeTypeFace> _bundledFace;
    uint32 _generation {0};

  public:
    FontDatabase() noexcept;

    KRYS_NODISCARD static FontDatabase &Singleton() noexcept;

    /// @brief Registers the faces of the font file at @p path. A file that's already registered isn't read
    /// again.
    /// @return The number of faces registered: 0 if the file can't be read or isn't a scalable font.
    uint32 AddFontFile(const std::filesystem::path &path);

    /// @brief Registers the font files (`.ttf`, `.otf`, `.ttc` and `.otc`) directly in @p path.
    /// @return The number of faces registered.
    uint32 AddFontDirectory(const std::filesystem::path &path);

    /// @brief Registers the faces of a font file's contents.
    /// @return The number of faces registered: 0 if @p bytes aren't a scalable font.
    uint32 AddFontData(List<byte> &&bytes) noexcept;

    /// @brief The faces of @p family, in registration order; empty if there's no such family.
    KRYS_NODISCARD Span<const Ref<text::FreeTypeFace>> FacesInFamily(utf8_stringview family) const noexcept;

    /// @brief The face of @p family that best matches @p request (CSS font matching), or null if there's no
    /// such family.
    KRYS_NODISCARD text::FreeTypeFace *MatchFace(utf8_stringview family,
                                                 const FontSelectionRequest &request) const noexcept;

    /// @brief The names of the registered families, as the fonts give them.
    KRYS_NODISCARD List<utf8_string> Families() const noexcept;

    /// @brief Every registered face, in registration order (the bundled face first).
    KRYS_NODISCARD Span<const Ref<text::FreeTypeFace>> Faces() const noexcept
    {
      return _faces;
    }

    /// @brief The bundled Liberation Sans, used when no registered font fits.
    KRYS_NODISCARD text::FreeTypeFace &BundledFace() const noexcept
    {
      return *_bundledFace;
    }

    /// @brief Increases whenever fonts are registered.
    KRYS_NODISCARD uint32 Generation() const noexcept
    {
      return _generation;
    }

  private:
    uint32 AddFontFileData(Ref<text::FontFileData> &&data) noexcept;
    void AddFace(Ref<text::FreeTypeFace> &&face) noexcept;
  };
}
