#pragma once

#include "Krystal.HTML/CSS/Values/Enums/CSSUnitType.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <tuple>

namespace Krys::HTML
{
  class CSSCalcAllowedSymbols
  {
    using AllowedSymbolsList = SmallList<std::tuple<CSSValueId, CSSUnitType>, 4uz>;

  private:
    AllowedSymbolsList _table;

  public:
    CSSCalcAllowedSymbols() noexcept = default;

    CSSCalcAllowedSymbols(AllowedSymbolsList &&mappings) noexcept : _table(Krys::Move(mappings))
    {
    }

    CSSCalcAllowedSymbols &operator=(const CSSCalcAllowedSymbols &) noexcept = default;
    CSSCalcAllowedSymbols(const CSSCalcAllowedSymbols &) noexcept = default;

    CSSCalcAllowedSymbols &operator=(CSSCalcAllowedSymbols &&) noexcept = default;
    CSSCalcAllowedSymbols(CSSCalcAllowedSymbols &&) noexcept = default;

    KRYS_NODISCARD Maybe<CSSUnitType> Get(CSSValueId id) const noexcept
    {
      auto it = std::ranges::find_if(_table, [id](const auto &pair) { return std::get<0>(pair) == id; });
      if (it != _table.end())
      {
        return std::get<1>(*it);
      }

      return Null;
    }

    KRYS_NODISCARD bool Contains(CSSValueId id) const noexcept
    {
      auto it = std::ranges::find_if(_table, [id](const auto &pair) { return std::get<0>(pair) == id; });
      return it != _table.end();
    }
  };

}