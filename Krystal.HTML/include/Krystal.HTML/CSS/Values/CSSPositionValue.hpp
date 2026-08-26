#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPosition.hpp"

namespace Krys::HTML
{
  class CSSPositionValue final : public CSSValue
  {
  private:
    CSSPosition _position;

    CSSPositionValue(CSSPosition &&);

  public:
    KRYS_NODISCARD static Ref<CSSPositionValue> Create(CSSPosition &&position) noexcept;

    KRYS_NODISCARD const CSSPosition &Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD bool Equals(const CSSPositionValue &other) const noexcept;
  };

  class CSSPositionXValue final : public CSSValue
  {
  private:
    CSSPositionX _position;

    CSSPositionXValue(CSSPositionX &&);

  public:
    KRYS_NODISCARD static Ref<CSSPositionXValue> Create(CSSPositionX &&position) noexcept;

    KRYS_NODISCARD const CSSPositionX &Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD bool Equals(const CSSPositionXValue &other) const noexcept;
  };

  class CSSPositionYValue final : public CSSValue
  {
  private:
    CSSPositionY _position;

    CSSPositionYValue(CSSPositionY &&);

  public:
    KRYS_NODISCARD static Ref<CSSPositionYValue> Create(CSSPositionY &&position) noexcept;

    KRYS_NODISCARD const CSSPositionY &Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD bool Equals(const CSSPositionYValue &other) const noexcept;
  };
}