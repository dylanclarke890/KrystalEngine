#pragma once

namespace krys::boo::css
{
  struct ComputedStyleDependencies;
}

namespace krys::boo::css::calc
{
  struct Tree;

  void CollectComputedStyleDependencies(const Tree &tree, ComputedStyleDependencies &dependencies) noexcept;
}