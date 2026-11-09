/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <initializer_list>
#include <tuple>

namespace krys::boo::css
{
  enum class UnitType : uint8;

  /// @brief Values for the symbols (e.g. the channel keywords of relative colours) a calc() may reference.
  class SymbolTable
  {
  public:
    struct Entry
    {
      UnitType Unit;
      double Value;
    };

  private:
    HashMap<ValueId, Entry> _table;

  public:
    SymbolTable() noexcept = default;

    SymbolTable(std::initializer_list<std::tuple<ValueId, UnitType, double>> initializer)
    {
      for (const auto &[id, unit, value] : initializer)
      {
        _table.emplace(id, Entry {unit, value});
      }
    }

    KRYS_NODISCARD Maybe<Entry> Get(ValueId id) const noexcept
    {
      if (const auto it = _table.find(id); it != _table.end())
      {
        return it->second;
      }

      return null;
    }

    KRYS_NODISCARD bool Contains(ValueId id) const noexcept
    {
      return _table.contains(id);
    }
  };
}
