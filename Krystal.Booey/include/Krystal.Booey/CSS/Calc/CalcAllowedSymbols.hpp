#pragma once

#include "Krystal.Booey/CSS/Values/Enums/CSSUnitType.hpp"
#include "Krystal.Booey/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <tuple>

namespace krys::boo::css
{
  class CalcAllowedSymbols
  {
    using AllowedSymbolsList = SmallList<std::tuple<CSSValueId, CSSUnitType>, 4uz>;

  private:
    AllowedSymbolsList _table;

  public:
    CalcAllowedSymbols() noexcept = default;

    CalcAllowedSymbols(AllowedSymbolsList &&mappings) noexcept : _table(krys::move(mappings))
    {
    }

    CalcAllowedSymbols &operator=(const CalcAllowedSymbols &) noexcept = default;
    CalcAllowedSymbols(const CalcAllowedSymbols &) noexcept = default;

    CalcAllowedSymbols &operator=(CalcAllowedSymbols &&) noexcept = default;
    CalcAllowedSymbols(CalcAllowedSymbols &&) noexcept = default;

    KRYS_NODISCARD Maybe<CSSUnitType> Get(CSSValueId id) const noexcept
    {
      auto it = std::ranges::find_if(_table, [id](const auto &pair) { return std::get<0>(pair) == id; });
      if (it != _table.end())
      {
        return std::get<1>(*it);
      }

      return null;
    }

    KRYS_NODISCARD bool Contains(CSSValueId id) const noexcept
    {
      auto it = std::ranges::find_if(_table, [id](const auto &pair) { return std::get<0>(pair) == id; });
      return it != _table.end();
    }
  };
}