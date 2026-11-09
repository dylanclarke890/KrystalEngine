/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 * Copyright (C) 2013, 2014 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Values/GridTemplateAreasValue.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::css
{
  GridTemplateAreasValue::GridTemplateAreasValue(GridTemplateAreas &&areas) noexcept
      : Value(ValueType::GridTemplateAreas), _areas(krys::move(areas))
  {
    krys_debug_assert(_areas.Map.RowCount);
    krys_debug_assert(_areas.Map.ColumnCount);
  }

  Ref<GridTemplateAreasValue> GridTemplateAreasValue::Create(GridTemplateAreas areas) noexcept
  {
    return AdoptRef(*new GridTemplateAreasValue(krys::move(areas)));
  }

  CSSOMString GridTemplateAreasValue::StringForRow(size_t row) const noexcept
  {
    List<const CSSOMString *> columns(_areas.Map.ColumnCount);
    for (const auto &[name, area] : _areas.Map.Map)
    {
      if (row >= area.Rows.StartLine() && row < area.Rows.EndLine())
      {
        for (unsigned i = area.Columns.StartLine(); i < area.Columns.EndLine(); i++)
        {
          columns[i] = &name;
        }
      }
    }

    StringBuilder builder;
    bool first = true;
    for (const auto *name : columns)
    {
      if (!first)
      {
        builder.Append(u8' ');
      }
      first = false;

      if (!name)
      {
        builder.Append(u8'.');
      }
      else
      {
        builder.Append(*name);
      }
    }

    return krys::move(builder).ToString();
  }

  CSSOMString GridTemplateAreasValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _areas);
  }

  bool GridTemplateAreasValue::Equals(const GridTemplateAreasValue &other) const noexcept
  {
    return _areas == other._areas;
  }
}
