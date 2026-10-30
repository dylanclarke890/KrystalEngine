#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief A span in a single direction (either rows or columns). Note that `StartLine` and `EndLine` are
  /// grid lines' indexes.
  /// @note Despite line numbers in the spec start in "1", the indexes here start in "0".
  class GridSpan
  {
  private:
    enum GridSpanType : uint8
    {
      UntranslatedDefinite,
      TranslatedDefinite,
      Indefinite
    };

    int _startLine;
    int _endLine;
    GridSpanType _type;

  public:
    struct GridSpanIterator
    {
      unsigned Value;

      GridSpanIterator(unsigned value) noexcept : Value(value)
      {
      }

      operator unsigned &() noexcept
      {
        return Value;
      }

      KRYS_NODISCARD unsigned operator*() const noexcept
      {
        return Value;
      }
    };

    KRYS_NODISCARD static GridSpan UntranslatedDefiniteGridSpan(int startLine, int endLine) noexcept;
    KRYS_NODISCARD static GridSpan TranslatedDefiniteGridSpan(unsigned startLine, unsigned endLine) noexcept;
    KRYS_NODISCARD static GridSpan IndefiniteGridSpan() noexcept;

    KRYS_NODISCARD static GridSpan MasonryAxisTranslatedDefiniteGridSpan() noexcept;

    KRYS_NODISCARD friend bool operator==(const GridSpan &, const GridSpan &) noexcept = default;

    KRYS_NODISCARD unsigned IntegerSpan() const noexcept;

    KRYS_NODISCARD int UntranslatedStartLine() const noexcept;
    KRYS_NODISCARD int UntranslatedEndLine() const noexcept;

    KRYS_NODISCARD unsigned StartLine() const noexcept;
    KRYS_NODISCARD unsigned EndLine() const noexcept;

    KRYS_NODISCARD GridSpanIterator begin() const noexcept;
    KRYS_NODISCARD GridSpanIterator end() const noexcept;

    KRYS_NODISCARD bool IsTranslatedDefinite() const noexcept;
    KRYS_NODISCARD bool IsIndefinite() const noexcept;

    void Translate(unsigned offset) noexcept;

    /// @brief Moves this span to be in the same coordinate space as `parent`.
    /// @param reverse Swaps the direction to handle RTL/LTR changes.
    void TranslateTo(const GridSpan &parent, bool reverse) noexcept;

    void Clamp(int max) noexcept;

    bool Clamp(int min, int max) noexcept;

  private:
    GridSpan(int startLine, int endLine, GridSpanType type) noexcept;
  };
}
