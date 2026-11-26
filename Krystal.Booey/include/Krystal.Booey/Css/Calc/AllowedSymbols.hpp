#pragma once

#include "Krystal.Booey/Css/Values/Units.hpp"
#include "Krystal.Booey/Css/Values/ValueId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <algorithm>
#include <tuple>

namespace krys::boo::css
{
  /// @brief The symbols a calc() may reference, and the unit each one resolves to.
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
      return Get(id).has_value();
    }
  };
}
