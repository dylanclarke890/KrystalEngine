/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/ComputedStyleDependencies.hpp"
#include <algorithm>

namespace krys::boo::css
{
  namespace
  {
    void AppendIfNotContains(List<PropertyId> &list, PropertyId property) noexcept
    {
      if (std::ranges::find(list, property) == list.end())
      {
        list.push_back(property);
      }
    }
  }

  // MARK: - Computed Style Dependencies

  void ComputedStyleDependenciesCollector<LengthUnit>::operator()(ComputedStyleDependencies &dependencies,
                                                                  LengthUnit lengthUnit) noexcept
  {
    using enum LengthUnit;

    switch (lengthUnit)
    {
      case Rcap:
      case Rch:
      case Rex:
      case Ric:
      case Rem:
      {
        AppendIfNotContains(dependencies.RootProperties, PropertyId::FontSize);
        break;
      }
      case Rlh:
      {
        AppendIfNotContains(dependencies.RootProperties, PropertyId::FontSize);
        AppendIfNotContains(dependencies.RootProperties, PropertyId::LineHeight);
        break;
      }
      case Em:
      case QuirkyEm:
      case Ex:
      case Cap:
      case Ch:
      case Ic:
      {
        AppendIfNotContains(dependencies.Properties, PropertyId::FontSize);
        break;
      }
      case Lh:
      {
        AppendIfNotContains(dependencies.Properties, PropertyId::FontSize);
        AppendIfNotContains(dependencies.Properties, PropertyId::LineHeight);
        break;
      }
      case Cqw:
      case Cqh:
      case Cqi:
      case Cqb:
      case Cqmin:
      case Cqmax:
      {
        dependencies.ContainerDimensions = true;
        break;
      }
      case Vw:
      case Vh:
      case Vmin:
      case Vmax:
      case Vb:
      case Vi:
      case Svw:
      case Svh:
      case Svmin:
      case Svmax:
      case Svb:
      case Svi:
      case Lvw:
      case Lvh:
      case Lvmin:
      case Lvmax:
      case Lvb:
      case Lvi:
      case Dvw:
      case Dvh:
      case Dvmin:
      case Dvmax:
      case Dvb:
      case Dvi:
      {
        dependencies.ViewportDimensions = true;
        break;
      }
      case Px:
      case Cm:
      case Mm:
      case In:
      case Pt:
      case Pc:
      case Q:
      {
        break;
      }
    }
  }
}
