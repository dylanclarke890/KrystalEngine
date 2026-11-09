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

#include "Krystal.Booey/CSS/Calc/TreeComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Calc/TreeTraversal.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"

namespace krys::boo::css::calc
{
  static void CollectComputedStyleDependencies(const Child &root,
                                               ComputedStyleDependencies &dependencies) noexcept
  {
    krys::SwitchOn(
      root,
      [&](const Number &)
      {
        // No potential dependencies.
      },
      [&](const Percentage &)
      {
        // No potential dependencies.
      },
      [&](const CanonicalDimension &)
      {
        // No potential dependencies.
      },
      [&](const NonCanonicalDimension &root)
      {
        if (auto lengthUnit = css::ToLengthUnit(root.Unit))
        {
          css::CollectComputedStyleDependencies(dependencies, *lengthUnit);
        }
      },
      [&](const Symbol &root)
      {
        if (auto lengthUnit = css::ToLengthUnit(root.Unit))
        {
          css::CollectComputedStyleDependencies(dependencies, *lengthUnit);
        }
      },
      [&](const SiblingCount &)
      {
        // No potential dependencies.
      },
      [&](const SiblingIndex &)
      {
        // No potential dependencies.
      },
      [&](const IndirectNode<Anchor> &anchor)
      {
        dependencies.Anchors = true;
        if (anchor->Fallback)
        {
          CollectComputedStyleDependencies(*anchor->Fallback, dependencies);
        }
      },
      [&](const IndirectNode<AnchorSize> &anchorSize)
      {
        dependencies.Anchors = true;
        if (anchorSize->Fallback)
        {
          CollectComputedStyleDependencies(*anchorSize->Fallback, dependencies);
        }
      },
      [&](const auto &root)
      {
        ForAllChildNodes(*root,
                         [&](const auto &root) { CollectComputedStyleDependencies(root, dependencies); });
      });
  }

  void CollectComputedStyleDependencies(const Tree &tree, ComputedStyleDependencies &dependencies) noexcept
  {
    CollectComputedStyleDependencies(tree.Root, dependencies);
  }
}