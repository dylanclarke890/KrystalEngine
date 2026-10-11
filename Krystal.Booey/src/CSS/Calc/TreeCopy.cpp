#include "Krystal.Booey/CSS/Calc/TreeCopy.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Core/Utils/Apply.hpp"

namespace krys::boo::css::calc
{
  KRYS_NODISCARD static auto Copy(const Random::SharingType &) noexcept -> Random::SharingType;
  KRYS_NODISCARD static auto Copy(const ValueId &) noexcept -> ValueId;
  KRYS_NODISCARD static auto Copy(const keywords::None &) noexcept -> keywords::None;
  KRYS_NODISCARD static auto Copy(const Maybe<Child> &root) noexcept -> Maybe<Child>;
  KRYS_NODISCARD static auto Copy(const ChildOrNone &) noexcept -> ChildOrNone;
  KRYS_NODISCARD static auto Copy(const Children &) noexcept -> Children;
  KRYS_NODISCARD static auto Copy(const Child &) noexcept -> Child;

  template <Leaf Op>
  Child Copy(const Op &) noexcept;
  template <typename Op>
  static auto Copy(const IndirectNode<Op> &) noexcept -> Child;
  static auto Copy(const IndirectNode<Anchor> &) noexcept -> Child;
  static auto Copy(const IndirectNode<AnchorSize> &) noexcept -> Child;

  // MARK: Copying

  Random::SharingType Copy(const Random::SharingType &root) noexcept
  {
    return root;
  }

  ValueId Copy(const ValueId &root) noexcept
  {
    return root;
  }

  keywords::None Copy(const keywords::None &root) noexcept
  {
    return root;
  }

  Maybe<Child> Copy(const Maybe<Child> &root) noexcept
  {
    if (root)
    {
      return Copy(*root);
    }

    return null;
  }

  ChildOrNone Copy(const ChildOrNone &root) noexcept
  {
    return krys::SwitchOn(root, [&](const auto &root) { return ChildOrNone {Copy(root)}; });
  }

  Children Copy(const Children &children) noexcept
  {
    SmallList<Child> copyChildren;
    for (const auto &child : children)
    {
      copyChildren.push_back(Copy(child));
    }

    return copyChildren;
  }

  Child Copy(const Child &root) noexcept
  {
    return krys::SwitchOn(root, [&](const auto &root) { return Copy(root); });
  }

  template <Leaf Op>
  Child Copy(const Op &root) noexcept
  {
    return {root};
  }

  template <typename Op>
  Child Copy(const IndirectNode<Op> &root) noexcept
  {
    return CreateChild(krys::apply([](const auto &...x) { return Op {Copy(x)...}; }, *root), root.Type);
  }

  AnchorSide Copy(const AnchorSide &root) noexcept
  {
    return krys::SwitchOn(root, [&](const auto &root) { return AnchorSide {Copy(root)}; });
  }

  Child Copy(const IndirectNode<Anchor> &anchor) noexcept
  {
    return CreateChild(Anchor {.ElementName = anchor->ElementName,
                               .Side = Copy(anchor->Side),
                               .Fallback = Copy(anchor->Fallback)},
                       anchor.Type);
  }

  Child Copy(const IndirectNode<AnchorSize> &anchorSize) noexcept
  {
    AnchorSize copyAnchorSize {.ElementName = anchorSize->ElementName,
                               .Dimension = anchorSize->Dimension,
                               .Fallback = Copy(anchorSize->Fallback)};

    return CreateChild(krys::move(copyAnchorSize), anchorSize.Type);
  }

  // MARK: Exposed functions

  Tree Copy(const Tree &tree) noexcept
  {
    return Tree {.Root = Copy(tree.Root),
                 .Type = tree.Type,
                 .Stage = tree.Stage,
                 .RequiresConversionData = tree.RequiresConversionData};
  }

}