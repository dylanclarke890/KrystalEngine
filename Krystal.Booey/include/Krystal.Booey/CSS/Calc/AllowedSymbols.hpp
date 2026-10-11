#pragma once

#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <tuple>
#include <ranges>

namespace krys::boo::css
{
  class AllowedSymbols
  {
    using AllowedSymbolsList = SmallList<std::tuple<ValueId, UnitType>, 4uz>;

  private:
    AllowedSymbolsList _table;

  public:
    AllowedSymbols() noexcept = default;

    AllowedSymbols(AllowedSymbolsList &&mappings) noexcept : _table(krys::move(mappings))
    {
    }

    AllowedSymbols &operator=(const AllowedSymbols &) noexcept = default;
    AllowedSymbols(const AllowedSymbols &) noexcept = default;

    AllowedSymbols &operator=(AllowedSymbols &&) noexcept = default;
    AllowedSymbols(AllowedSymbols &&) noexcept = default;

    KRYS_NODISCARD Maybe<UnitType> Get(ValueId id) const noexcept
    {
      auto it = std::ranges::find_if(_table, [id](const auto &pair) { return std::get<0>(pair) == id; });
      if (it != _table.end())
      {
        return std::get<1>(*it);
      }

      return null;
    }

    KRYS_NODISCARD bool Contains(ValueId id) const noexcept
    {
      auto it = std::ranges::find_if(_table, [id](const auto &pair) { return std::get<0>(pair) == id; });
      return it != _table.end();
    }
  };
}