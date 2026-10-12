#pragma once

#include "Krystal.Booey/CSS/Calc/CalcType.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::style
{
  enum class AnchorSizeDimension : uint8;
}

namespace krys::boo::css
{
  enum class UnitType : uint8;
}

namespace krys::boo::css::calc
{
  // Math Operators.
  struct Sum;
  struct Product;
  struct Negate;
  struct Invert;

  // Math Functions.
  struct Min;
  struct Max;
  struct Clamp;
  struct RoundNearest;
  struct RoundUp;
  struct RoundDown;
  struct RoundToZero;
  struct Mod;
  struct Rem;
  struct Sin;
  struct Cos;
  struct Tan;
  struct Asin;
  struct Acos;
  struct Atan;
  struct Atan2;
  struct Pow;
  struct Sqrt;
  struct Hypot;
  struct Log;
  struct Exp;
  struct Abs;
  struct Sign;
  struct Random;
  struct Progress;

  // CSS Anchor Positioning functions.
  struct Anchor;
  struct AnchorSize;

  template <typename Operation>
  concept Leaf = requires(Operation) { Operation::IsLeaf == true; };

  template <typename Operation>
  concept Numeric = requires(Operation) { Operation::IsNumeric == true; };

  // Leaf Values
  struct Number
  {
    constexpr static bool IsLeaf = true;
    constexpr static bool IsNumeric = true;

    double Value;

    bool operator==(const Number &) const noexcept = default;
  };

  struct Percentage
  {
    constexpr static bool IsLeaf = true;
    constexpr static bool IsNumeric = true;

    double Value;
    Type::PercentHintValue Hint;

    bool operator==(const Percentage &) const noexcept = default;
  };

  enum class Dimension : uint8
  {
    Length,
    Angle,
    Time,
    Frequency,
    Resolution,
    Flex
  };

  struct CanonicalDimension
  {
    constexpr static bool IsLeaf = true;
    constexpr static bool IsNumeric = true;

    double Value;
    calc::Dimension Dimension;

    bool operator==(const CanonicalDimension &) const noexcept = default;
  };

  struct NonCanonicalDimension
  {
    constexpr static bool IsLeaf = true;
    constexpr static bool IsNumeric = true;

    double Value;
    UnitType Unit;

    bool operator==(const NonCanonicalDimension &) const noexcept = default;
  };

  struct Symbol
  {
    constexpr static bool IsLeaf = true;

    ValueId Id;
    UnitType Unit;

    bool operator==(const Symbol &) const noexcept = default;
  };

  // Tree Counting Functions - https://drafts.csswg.org/css-values-5/#tree-counting
  // NOTE: As these functions don't have any parameters, they are leaf nodes.

  struct SiblingCount
  {
    constexpr static bool IsLeaf = true;
    constexpr static auto Id = ValueId::SiblingCount;

    // <sibling-count()> = sibling-count()
    //     - INPUT: none
    //     - OUTPUT: integer

    bool operator==(const SiblingCount &) const noexcept = default;
  };

  struct SiblingIndex
  {
    constexpr static bool IsLeaf = true;
    constexpr static auto Id = ValueId::SiblingIndex;

    // <sibling-index()> = sibling-index()
    //     - INPUT: none
    //     - OUTPUT: integer

    bool operator==(const SiblingIndex &) const noexcept = default;
  };

  template <typename Operation>
  struct IndirectNode
  {
    calc::Type Type;
    UniquePtr<Operation> Op;

    KRYS_NODISCARD const Operation &operator*() const noexcept
    {
      return *Op;
    }

    KRYS_NODISCARD Operation &operator*() noexcept
    {
      return *Op;
    }

    KRYS_NODISCARD const Operation *operator->() const noexcept
    {
      return Op.get();
    }

    KRYS_NODISCARD Operation *operator->() noexcept
    {
      return Op.get();
    }

    KRYS_NODISCARD operator const Operation &() const noexcept
    {
      return *Op;
    }

    KRYS_NODISCARD operator Operation &() noexcept
    {
      return *Op;
    }

    KRYS_NODISCARD bool operator==(const IndirectNode<Operation> &other) const noexcept
    {
      return Type == other.Type && (Op == other.Op || (Op && other.Op && *Op == *other.Op));
    }
  };

  using Node =
    Variant<Number, Percentage, CanonicalDimension, NonCanonicalDimension, Symbol, SiblingCount, SiblingIndex,
            IndirectNode<Sum>, IndirectNode<Product>, IndirectNode<Negate>, IndirectNode<Invert>,
            IndirectNode<Min>, IndirectNode<Max>, IndirectNode<Clamp>, IndirectNode<RoundNearest>,
            IndirectNode<RoundUp>, IndirectNode<RoundDown>, IndirectNode<RoundToZero>, IndirectNode<Mod>,
            IndirectNode<Rem>, IndirectNode<Sin>, IndirectNode<Cos>, IndirectNode<Tan>, IndirectNode<Asin>,
            IndirectNode<Acos>, IndirectNode<Atan>, IndirectNode<Atan2>, IndirectNode<Pow>,
            IndirectNode<Sqrt>, IndirectNode<Hypot>, IndirectNode<Log>, IndirectNode<Exp>, IndirectNode<Abs>,
            IndirectNode<Sign>, IndirectNode<Random>, IndirectNode<Progress>, IndirectNode<Anchor>,
            IndirectNode<AnchorSize>>;

  struct Child
  {
    Node Value;

    template <typename T>
    requires std::constructible_from<Node, T>
    Child(T &&) noexcept;

    KRYS_FORWARD_VARIANT_FUNCTIONS(Child, Value)

    bool operator==(const Child &) const noexcept = default;
  };

  struct ChildOrNone
  {
    Variant<Child, keywords::None> Value;

    ChildOrNone(Child &&) noexcept;
    ChildOrNone(keywords::None) noexcept;

    KRYS_FORWARD_VARIANT_FUNCTIONS(ChildOrNone, Value)

    bool operator==(const ChildOrNone &) const noexcept = default;
  };

  struct Children
  {
    using iterator = typename List<Child>::iterator;
    using reverse_iterator = typename List<Child>::reverse_iterator;
    using const_iterator = typename List<Child>::const_iterator;
    using const_reverse_iterator = typename List<Child>::const_reverse_iterator;
    using value_type = typename List<Child>::value_type;

    List<Child> Value;

    Children(Children &&) noexcept;
    Children(List<Child> &&) noexcept;
    Children &operator=(Children &&) noexcept;
    Children &operator=(List<Child> &&) noexcept;

    KRYS_NODISCARD iterator begin() noexcept;
    KRYS_NODISCARD iterator end() noexcept;
    KRYS_NODISCARD reverse_iterator rbegin() noexcept;
    KRYS_NODISCARD reverse_iterator rend() noexcept;

    KRYS_NODISCARD const_iterator begin() const noexcept;
    KRYS_NODISCARD const_iterator end() const noexcept;
    KRYS_NODISCARD const_reverse_iterator rbegin() const noexcept;
    KRYS_NODISCARD const_reverse_iterator rend() const noexcept;

    KRYS_NODISCARD bool empty() const noexcept;
    KRYS_NODISCARD size_t size() const noexcept;

    Child &operator[](size_t i) noexcept;
    const Child &operator[](size_t i) const noexcept;

    bool operator==(const Children &) const noexcept = default;
  };

  enum class Stage : bool
  {
    Specified,
    Computed
  };

  struct Tree
  {
    Child Root;
    calc::Type Type;
    calc::Stage Stage;

    // `RequiresConversionData` is used both to both indicate whether eager evaluation of the tree (at parse
    // time) is possible or not and to trigger a warning in `css::calc::Value::DoubleValueDeprecated` that the
    // evaluation results will be incorrect.
    bool RequiresConversionData = false;

    bool operator==(const Tree &) const noexcept = default;
  };

  struct Sum
  {
    calc::Children Children;

    bool operator==(const Sum &) const noexcept = default;
  };

  struct Product
  {
    calc::Children Children;

    bool operator==(const Product &) const noexcept = default;
  };

  struct Negate
  {
    Child A;

    bool operator==(const Negate &) const noexcept = default;
  };

  struct Invert
  {
    Child A;

    bool operator==(const Invert &) const noexcept = default;
  };

  // Math Functions

  // Comparison Functions - https://drafts.csswg.org/css-values-4/#comp-func
  struct Min
  {
    constexpr static auto Id = ValueId::Min;

    // <min()>   = min( <calc-sum># )
    //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
    //     - OUTPUT: consistent type
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    calc::Children Children;

    bool operator==(const Min &) const noexcept = default;
  };

  struct Max
  {
    constexpr static auto Id = ValueId::Max;

    // <max()>   = max( <calc-sum># )
    //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
    //     - OUTPUT: consistent type
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    calc::Children Children;

    bool operator==(const Max &) const noexcept = default;
  };

  struct Clamp
  {
    constexpr static auto Id = ValueId::Clamp;

    // <clamp()> = clamp( [ <calc-sum> | none ], <calc-sum>, [ <calc-sum> | none ] )
    //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
    //     - OUTPUT: consistent type
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    ChildOrNone Min;
    Child Val;
    ChildOrNone Max;

    bool operator==(const Clamp &) const noexcept = default;
  };

  // Stepped Value Functions - https://drafts.csswg.org/css-values-4/#round-func
  struct RoundNearest
  {
    constexpr static auto Id = ValueId::Nearest;

    // <round()> = round( <rounding-strategy>?, <calc-sum>, <calc-sum> )
    //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
    //     - OUTPUT: consistent type
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    // -- and --

    // <round()> = round( <rounding-strategy>?, <calc-sum> )
    //     - INPUT: "consistent" <number>
    //     - OUTPUT: consistent type

    // NOTE: This is special cased in the code.

    Child A;
    Maybe<Child> B;

    bool operator==(const RoundNearest &) const noexcept = default;
  };

  struct RoundUp
  {
    constexpr static auto Id = ValueId::Up;

    // <round()> = round( <rounding-strategy>?, <calc-sum>, <calc-sum> )
    //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
    //     - OUTPUT: consistent type
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    // -- and --

    // <round()> = round( <rounding-strategy>?, <calc-sum> )
    //     - INPUT: "consistent" <number>
    //     - OUTPUT: consistent type

    // NOTE: This is special cased in the code.

    Child A;
    Maybe<Child> B;

    bool operator==(const RoundUp &) const noexcept = default;
  };

  struct RoundDown
  {
    constexpr static auto Id = ValueId::Down;

    // <round()> = round( <rounding-strategy>?, <calc-sum>, <calc-sum> )
    //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
    //     - OUTPUT: consistent type
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    // -- and --

    // <round()> = round( <rounding-strategy>?, <calc-sum> )
    //     - INPUT: "consistent" <number>
    //     - OUTPUT: consistent type

    // NOTE: This is special cased in the code.

    Child A;
    Maybe<Child> B;

    bool operator==(const RoundDown &) const noexcept = default;
  };

  struct RoundToZero
  {
    constexpr static auto Id = ValueId::ToZero;

    // <round()> = round( <rounding-strategy>?, <calc-sum>, <calc-sum> )
    //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
    //     - OUTPUT: consistent type
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    // -- and --

    // <round()> = round( <rounding-strategy>?, <calc-sum> )
    //     - INPUT: "consistent" <number>
    //     - OUTPUT: consistent type

    // NOTE: This is special cased in the code.

    Child A;
    Maybe<Child> B;

    bool operator==(const RoundToZero &) const noexcept = default;
  };

  struct Mod
  {
    constexpr static auto Id = ValueId::Mod;

    // <mod()>   = mod( <calc-sum>, <calc-sum> )
    //     - INPUT: "same" <number>, <dimension>, or <percentage>
    //     - OUTPUT: same type
    constexpr static auto Input = AllowedTypes::Any;
    // FIXME: Spec says this should enforce same type, but tests indicate it should be consistent type.
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    Child A;
    Child B;

    bool operator==(const Mod &) const noexcept = default;
  };

  struct Rem
  {
    constexpr static auto Id = ValueId::Rem;

    // <rem()>   = rem( <calc-sum>, <calc-sum> )
    //     - INPUT: "same" <number>, <dimension>, or <percentage>
    //     - OUTPUT: same type
    constexpr static auto Input = AllowedTypes::Any;
    // FIXME: Spec says this should enforce same type, but tests indicate it should be consistent type.
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    Child A;
    Child B;

    bool operator==(const Rem &) const noexcept = default;
  };

  // Trigonometric Functions - https://drafts.csswg.org/css-values-4/#trig-funcs
  struct Sin
  {
    constexpr static auto Id = ValueId::Sin;

    // <sin()>   = sin( <calc-sum> )
    //     - INPUT: <number> or <angle>
    //     - OUTPUT: <number> "made consistent"
    constexpr static auto Input = AllowedTypes::NumberOrAngle;
    constexpr static auto Output = OutputTransform::NumberMadeConsistent;

    Child A;

    bool operator==(const Sin &) const noexcept = default;
  };

  struct Cos
  {
    constexpr static auto Id = ValueId::Cos;

    // <cos()>   = cos( <calc-sum> )
    //     - INPUT: <number> or <angle>
    //     - OUTPUT: <number> "made consistent"
    constexpr static auto Input = AllowedTypes::NumberOrAngle;
    constexpr static auto Output = OutputTransform::NumberMadeConsistent;

    Child A;

    bool operator==(const Cos &) const noexcept = default;
  };

  struct Tan
  {
    constexpr static auto Id = ValueId::Tan;

    // <tan()>   = tan( <calc-sum> )
    //     - INPUT: <number> or <angle>
    //     - OUTPUT: <number> "made consistent"
    constexpr static auto Input = AllowedTypes::NumberOrAngle;
    constexpr static auto Output = OutputTransform::NumberMadeConsistent;

    Child A;

    bool operator==(const Tan &) const noexcept = default;
  };

  struct Asin
  {
    constexpr static auto Id = ValueId::Asin;

    // <asin()>  = asin( <calc-sum> )
    //     - INPUT: <number>
    //     - OUTPUT: <angle> "made consistent"
    constexpr static auto Input = AllowedTypes::Number;
    constexpr static auto Output = OutputTransform::AngleMadeConsistent;

    Child A;

    bool operator==(const Asin &) const noexcept = default;
  };

  struct Acos
  {
    constexpr static auto Id = ValueId::Acos;

    // <acos()>  = acos( <calc-sum> )
    //     - INPUT: <number>
    //     - OUTPUT: <angle> "made consistent"
    constexpr static auto Input = AllowedTypes::Number;
    constexpr static auto Output = OutputTransform::AngleMadeConsistent;

    Child A;

    bool operator==(const Acos &) const noexcept = default;
  };

  struct Atan
  {
    constexpr static auto Id = ValueId::Atan;

    // <atan()>  = atan( <calc-sum> )
    //     - INPUT: <number>
    //     - OUTPUT: <angle> "made consistent"
    constexpr static auto Input = AllowedTypes::Number;
    constexpr static auto Output = OutputTransform::AngleMadeConsistent;

    Child A;

    bool operator==(const Atan &) const noexcept = default;
  };

  struct Atan2
  {
    constexpr static auto Id = ValueId::Atan2;

    // <atan2()> = atan2( <calc-sum>, <calc-sum> )
    //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
    //     - OUTPUT: <angle> "made consistent"
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::AngleMadeConsistent;

    Child A;
    Child B;

    bool operator==(const Atan2 &) const noexcept = default;
  };

  // Exponential Functions - https://drafts.csswg.org/css-values-4/#exponent-funcs
  struct Pow
  {
    constexpr static auto Id = ValueId::Pow;

    // <pow()>   = pow( <calc-sum>, <calc-sum> )
    //     - INPUT: "consistent" <number>
    //     - OUTPUT: consistent type
    constexpr static auto Input = AllowedTypes::Number;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    Child A;
    Child B;

    bool operator==(const Pow &) const noexcept = default;
  };

  struct Sqrt
  {
    constexpr static auto Id = ValueId::Sqrt;

    // <sqrt()>  = sqrt( <calc-sum> )
    //     - INPUT: <number>
    //     - OUTPUT: <number> "made consistent"
    constexpr static auto Input = AllowedTypes::Number;
    constexpr static auto Output = OutputTransform::NumberMadeConsistent;

    Child A;

    bool operator==(const Sqrt &) const noexcept = default;
  };

  struct Hypot
  {
    constexpr static auto Id = ValueId::Hypot;

    // <hypot()> = hypot( <calc-sum># )
    //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
    //     - OUTPUT: consistent type
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::None;

    calc::Children Children;

    bool operator==(const Hypot &) const noexcept = default;
  };

  struct Log
  {
    constexpr static auto Id = ValueId::Log;

    // <log()>   = log( <calc-sum>, <calc-sum>? )
    //     - INPUT: <number>
    //     - OUTPUT: <number> "made consistent"
    constexpr static auto Input = AllowedTypes::Number;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::NumberMadeConsistent;

    Child A;
    Maybe<Child> B;

    bool operator==(const Log &) const noexcept = default;
  };

  struct Exp
  {
    constexpr static auto Id = ValueId::Exp;

    // <exp()>   = exp( <calc-sum> )
    //     - INPUT: <number>
    //     - OUTPUT: <number> "made consistent"
    constexpr static auto Input = AllowedTypes::Number;
    constexpr static auto Output = OutputTransform::NumberMadeConsistent;

    Child A;

    bool operator==(const Exp &) const noexcept = default;
  };

  // Sign-Related Functions - https://drafts.csswg.org/css-values-4/#sign-funcs
  struct Abs
  {
    constexpr static auto Id = ValueId::Abs;

    // <abs()>   = abs( <calc-sum> )
    //     - INPUT: any
    //     - OUTPUT: input type
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Output = OutputTransform::None;

    Child A;

    bool operator==(const Abs &) const noexcept = default;
  };

  struct Sign
  {
    constexpr static auto Id = ValueId::Sign;

    // <sign()>  = sign( <calc-sum> )
    //     - INPUT: any
    //     - OUTPUT: <number> "made consistent"
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Output = OutputTransform::NumberMadeConsistent;

    Child A;

    bool operator==(const Sign &) const noexcept = default;
  };

  // Random Function - https://drafts.csswg.org/css-values-5/#random
  struct Random
  {
    constexpr static auto Id = ValueId::Random;

    // <random-value-sharing> = [ [ auto | <dashed-ident> ] || element-shared ] | fixed <number [0,1]>
    struct SharingOptions
    {
      struct Auto
      {
        PropertyId Property;
        uint32 Index;

        bool operator==(const Auto &) const noexcept = default;
      };

      Variant<Auto, CSSOMStringAtom> Identifier;
      Maybe<keywords::ElementShared> ElementShared;

      bool operator==(const SharingOptions &) const noexcept = default;
    };

    struct SharingFixed
    {
      css::Number<css::ClosedUnitRange> Value;

      bool operator==(const SharingFixed &) const noexcept = default;
    };

    using SharingType = Variant<SharingOptions, SharingFixed>;

    // <random()> = random( <random-value-sharing>? , <calc-sum>, <calc-sum>, <calc-sum>? )
    //     - INPUT: "same" <number>, <dimension>, or <percentage>
    //     - OUTPUT: same type
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Same;
    constexpr static auto Output = OutputTransform::None;

    SharingType Sharing;
    Child Min;
    Child Max;
    Maybe<Child> Step;

    bool operator==(const Random &) const noexcept = default;
  };

  // Progress-Related Functions - https://drafts.csswg.org/css-values-5/#progress
  struct Progress
  {
    constexpr static auto Id = ValueId::Progress;

    // <progress()> = progress( <calc-sum>, <calc-sum>, <calc-sum> )
    //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
    //     - OUTPUT: <number> "made consistent"
    constexpr static auto Input = AllowedTypes::Any;
    constexpr static auto Merge = MergePolicy::Consistent;
    constexpr static auto Output = OutputTransform::NumberMadeConsistent;

    Child Value;
    Child Start;
    Child End;

    bool operator==(const Progress &) const noexcept = default;
  };

  // Anchor Positioning Related Functions - https://drafts.csswg.org/css-anchor-position-1/

  struct AnchorSide
  {
    // <anchor-side> = inside | outside | top | left | right | bottom | start | end | self-start | self-end
    // | <percentage> | center
    Variant<ValueId, Child> Value;

    AnchorSide(ValueId) noexcept;
    AnchorSide(Child &&) noexcept;

    KRYS_FORWARD_VARIANT_FUNCTIONS(AnchorSide, Value)

    bool operator==(const AnchorSide &) const noexcept = default;
  };

  struct Anchor
  {
    constexpr static auto Id = ValueId::Anchor;

    // <anchor()> = anchor( <anchor-element>? && <anchor-side>, <length-percentage>? )

    // Can't use Style::ScopedName here, since the scope ordinal is not available at
    // parsing time.
    CSSOMStringAtom ElementName;
    AnchorSide Side;
    Maybe<Child> Fallback;

    bool operator==(const Anchor &) const noexcept = default;
  };

  struct AnchorSize
  {
    constexpr static auto Id = ValueId::AnchorSize;

    // anchor-size() = anchor-size( [ <anchor-element> || <anchor-size> ]? , <length-percentage>? )
    // <anchor-element> = <dashed-ident>
    // <anchor-size> = width | height | block | inline | self-block | self-inline

    // Can't use Style::ScopedName here, since the scope ordinal is not available at
    // parsing time.
    CSSOMStringAtom ElementName;                 // <anchor-element>
    Maybe<style::AnchorSizeDimension> Dimension; // <anchor-size>
    Maybe<Child> Fallback;

    bool operator==(const AnchorSize &) const noexcept = default;
  };

  // MARK: Construction

  // Default implementation of ChildConstruction used for all indirect nodes.
  template <typename Operation>
  struct ChildConstruction
  {
    KRYS_NODISCARD static Child Create(Operation &&op, Type type) noexcept
    {
      return Child {IndirectNode<Operation> {type, CreateUnique<Operation>(krys::move(op))}};
    }
  };

  // Specialized implementation of ChildConstruction for leaf nodes, needed to avoid `CreateUnique`.
  template <Leaf T>
  struct ChildConstruction<T>
  {
    KRYS_NODISCARD static Child Create(T &&op, Type) noexcept
    {
      return Child {krys::move(op)};
    }

    KRYS_NODISCARD static Child Create(T &&op) noexcept
    {
      return Child {krys::move(op)};
    }
  };

  template <typename Operation>
  KRYS_NODISCARD Child CreateChild(Operation op) noexcept
  {
    return ChildConstruction<Operation>::Create(krys::move(op));
  }

  template <typename Operation>
  KRYS_NODISCARD Child CreateChild(Operation op, Type type) noexcept
  {
    return ChildConstruction<Operation>::Create(krys::move(op), type);
  }

  // MARK: Type Evaluation

  // Gets the type stored in the IndirectNode.
  template <typename T>
  KRYS_NODISCARD inline Type GetType(const IndirectNode<T> &root) noexcept
  {
    return root.Type;
  }

  // Gets the Type representing a Dimension.
  KRYS_NODISCARD Type GetType(Dimension) noexcept;

  // Gets the type of the leaf node.
  KRYS_NODISCARD Type GetType(const Number &) noexcept;
  KRYS_NODISCARD Type GetType(const Percentage &) noexcept;
  KRYS_NODISCARD Type GetType(const NonCanonicalDimension &) noexcept;
  KRYS_NODISCARD Type GetType(const CanonicalDimension &) noexcept;
  KRYS_NODISCARD Type GetType(const Symbol &) noexcept;
  KRYS_NODISCARD Type GetType(const SiblingCount &) noexcept;
  KRYS_NODISCARD Type GetType(const SiblingIndex &) noexcept;

  // Gets the type of the child node.
  KRYS_NODISCARD Type GetType(const Child &) noexcept;

  // Constructs the nodes's type from it the types of its arguments. Returns `null` on failure.
  KRYS_NODISCARD Maybe<Type> ToType(const Sum &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Product &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Negate &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Invert &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Min &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Max &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Clamp &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const RoundNearest &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const RoundUp &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const RoundDown &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const RoundToZero &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Mod &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Rem &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Sin &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Cos &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Tan &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Asin &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Acos &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Atan &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Atan2 &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Pow &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Sqrt &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Hypot &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Log &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Exp &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Abs &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Sign &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Random &) noexcept;
  KRYS_NODISCARD Maybe<Type> ToType(const Progress &) noexcept;

  // MARK: UnitType Evaluation

  // Maps Dimension to its UnitType counterpart.
  KRYS_NODISCARD constexpr UnitType ToUnit(const Dimension &dimension) noexcept
  {
    switch (dimension)
    {
      case Dimension::Length:     return UnitType::px;
      case Dimension::Angle:      return UnitType::deg;
      case Dimension::Time:       return UnitType::s;
      case Dimension::Frequency:  return UnitType::hz;
      case Dimension::Resolution: return UnitType::dppx;
      case Dimension::Flex:       return UnitType::fr;
    }

    krys_unreachable();
  }

  // Maps Numeric type to its UnitType counterpart.
  KRYS_NODISCARD constexpr UnitType ToUnit(const Number &) noexcept
  {
    return UnitType::Number;
  }

  KRYS_NODISCARD constexpr UnitType ToUnit(const Percentage &) noexcept
  {
    return UnitType::Percentage;
  }

  KRYS_NODISCARD constexpr UnitType ToUnit(const CanonicalDimension &dimension) noexcept
  {
    return ToUnit(dimension.Dimension);
  }

  KRYS_NODISCARD constexpr UnitType ToUnit(const NonCanonicalDimension &dimension) noexcept
  {
    return dimension.Unit;
  }

  // MARK: Predicates

  KRYS_NODISCARD inline bool IsNumeric(const Child &root) noexcept
  {
    return krys::SwitchOn(
      root, []<Numeric T>(const T &) { return true; }, [](const auto &) { return false; });
  }

  // Convenience constructors

  // Creates the appropriate child type (number, percentage, canonical-dimensions, non-canonical-dimension)
  // based on the UnitType.
  KRYS_NODISCARD Child CreateNumeric(double, UnitType) noexcept;

  KRYS_NODISCARD inline Sum Add(Child &&a, Child &&b) noexcept
  {
    List<Child> sumChildren;
    sumChildren.push_back(krys::move(a));
    sumChildren.push_back(krys::move(b));

    return Sum {.Children = krys::move(sumChildren)};
  }

  KRYS_NODISCARD inline Product Multiply(Child &&a, Child &&b) noexcept
  {
    List<Child> productChildren;
    productChildren.push_back(krys::move(a));
    productChildren.push_back(krys::move(b));

    return Product {.Children = krys::move(productChildren)};
  }

  KRYS_NODISCARD inline Sum Subtract(Child &&a, Child &&b) noexcept
  {
    return Add(krys::move(a), CreateChild(Negate {.A = krys::move(b)}, GetType(b)));
  }

  KRYS_NODISCARD inline Child CreateChildWithValueBasedOn(double value, const Number &) noexcept
  {
    return CreateChild(Number {.Value = value});
  }

  KRYS_NODISCARD inline Child CreateChildWithValueBasedOn(double value, const Percentage &a) noexcept
  {
    return CreateChild(Percentage {.Value = value, .Hint = a.Hint});
  }

  KRYS_NODISCARD inline Child CreateChildWithValueBasedOn(double value, const CanonicalDimension &a) noexcept
  {
    return CreateChild(CanonicalDimension {.Value = value, .Dimension = a.Dimension});
  }

  KRYS_NODISCARD inline Child CreateChildWithValueBasedOn(double value,
                                                          const NonCanonicalDimension &a) noexcept
  {
    return CreateChild(NonCanonicalDimension {.Value = value, .Unit = a.Unit});
  }

  // MARK: Tuple Conformance

  // get<> overload (along with std::tuple_size and std::tuple_element below) to support destructuring of
  // operation nodes.

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Sum &root) noexcept
  {
    static_assert(I == 0);
    return root.Children;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Product &root) noexcept
  {
    static_assert(I == 0);
    return root.Children;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Negate &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Invert &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Min &root) noexcept
  {
    static_assert(I == 0);
    return root.Children;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Max &root) noexcept
  {
    static_assert(I == 0);
    return root.Children;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Clamp &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.Min;
    }
    else if constexpr (I == 1)
    {
      return root.Val;
    }
    else if constexpr (I == 2)
    {
      return root.Max;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const RoundNearest &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.A;
    }
    else if constexpr (I == 1)
    {
      return root.B;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const RoundUp &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.A;
    }
    else if constexpr (I == 1)
    {
      return root.B;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const RoundDown &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.A;
    }
    else if constexpr (I == 1)
    {
      return root.B;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const RoundToZero &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.A;
    }
    else if constexpr (I == 1)
    {
      return root.B;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Mod &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.A;
    }
    else if constexpr (I == 1)
    {
      return root.B;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Rem &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.A;
    }
    else if constexpr (I == 1)
    {
      return root.B;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Sin &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Cos &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Tan &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Asin &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Acos &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Atan &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Atan2 &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.A;
    }
    else if constexpr (I == 1)
    {
      return root.B;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Pow &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.A;
    }
    else if constexpr (I == 1)
    {
      return root.B;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Sqrt &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Hypot &root) noexcept
  {
    static_assert(I == 0);
    return root.Children;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Log &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.A;
    }
    else if constexpr (I == 1)
    {
      return root.B;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Exp &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Abs &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Sign &root) noexcept
  {
    static_assert(I == 0);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Random &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.Sharing;
    }
    else if constexpr (I == 1)
    {
      return root.Min;
    }
    else if constexpr (I == 2)
    {
      return root.Max;
    }
    else if constexpr (I == 3)
    {
      return root.Step;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Progress &root) noexcept
  {
    if constexpr (I == 0)
    {
      return root.Value;
    }
    else if constexpr (I == 1)
    {
      return root.Start;
    }
    else if constexpr (I == 2)
    {
      return root.End;
    }
  }

  // MARK: Child Definition

  template <typename T>
  requires std::constructible_from<Node, T>
  Child::Child(T &&value) noexcept : Value(std::forward<T>(value))
  {
  }

  // MARK: ChildOrNone Definition

  inline ChildOrNone::ChildOrNone(Child &&child) noexcept : Value(krys::move(child))
  {
  }

  inline ChildOrNone::ChildOrNone(keywords::None none) noexcept : Value(none)
  {
  }

  // MARK: Children Definition

  inline Children::Children(Children &&other) noexcept : Value(krys::move(other.Value))
  {
  }

  inline Children::Children(List<Child> &&other) noexcept : Value(krys::move(other))
  {
  }

  inline Children &Children::operator=(Children &&other) noexcept
  {
    Value = krys::move(other.Value);
    return *this;
  }

  inline Children &Children::operator=(List<Child> &&other) noexcept
  {
    Value = krys::move(other);
    return *this;
  }

  inline Children::iterator Children::begin() noexcept
  {
    return Value.begin();
  }

  inline Children::iterator Children::end() noexcept
  {
    return Value.end();
  }

  inline Children::reverse_iterator Children::rbegin() noexcept
  {
    return Value.rbegin();
  }

  inline Children::reverse_iterator Children::rend() noexcept
  {
    return Value.rend();
  }

  inline Children::const_iterator Children::begin() const noexcept
  {
    return Value.begin();
  }

  inline Children::const_iterator Children::end() const noexcept
  {
    return Value.end();
  }

  inline Children::const_reverse_iterator Children::rbegin() const noexcept
  {
    return Value.rbegin();
  }

  inline Children::const_reverse_iterator Children::rend() const noexcept
  {
    return Value.rend();
  }

  inline bool Children::empty() const noexcept
  {
    return Value.empty();
  }

  inline size_t Children::size() const noexcept
  {
    return Value.size();
  }

  inline Child &Children::operator[](size_t i) noexcept
  {
    return Value[i];
  }

  inline const Child &Children::operator[](size_t i) const noexcept
  {
    return Value[i];
  }

  // AnchorSize

  inline AnchorSide::AnchorSide(ValueId valueId) noexcept : Value(valueId)
  {
  }

  inline AnchorSide::AnchorSide(Child &&child) noexcept : Value(krys::move(child))
  {
  }

  // MARK: Size assertions

  static_assert(sizeof(Child) <= 24, "Child should stay small");
}

namespace std
{

#define OP_TUPLE_LIKE_CONFORMANCE(Op, numberOfArguments)                                                     \
  template <>                                                                                                \
  class tuple_size<krys::boo::css::calc::Op> : public std::integral_constant<size_t, numberOfArguments>      \
  {                                                                                                          \
  };                                                                                                         \
  template <size_t I>                                                                                        \
  class tuple_element<I, krys::boo::css::calc::Op>                                                           \
  {                                                                                                          \
  public:                                                                                                    \
    using type = decltype(krys::boo::css::calc::get<I>(std::declval<krys::boo::css::calc::Op>()));           \
  }

  OP_TUPLE_LIKE_CONFORMANCE(Sum, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Product, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Negate, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Invert, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Min, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Max, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Clamp, 3);
  OP_TUPLE_LIKE_CONFORMANCE(RoundNearest, 2);
  OP_TUPLE_LIKE_CONFORMANCE(RoundUp, 2);
  OP_TUPLE_LIKE_CONFORMANCE(RoundDown, 2);
  OP_TUPLE_LIKE_CONFORMANCE(RoundToZero, 2);
  OP_TUPLE_LIKE_CONFORMANCE(Mod, 2);
  OP_TUPLE_LIKE_CONFORMANCE(Rem, 2);
  OP_TUPLE_LIKE_CONFORMANCE(Sin, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Cos, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Tan, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Asin, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Acos, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Atan, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Atan2, 2);
  OP_TUPLE_LIKE_CONFORMANCE(Pow, 2);
  OP_TUPLE_LIKE_CONFORMANCE(Sqrt, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Hypot, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Log, 2);
  OP_TUPLE_LIKE_CONFORMANCE(Exp, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Abs, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Sign, 1);
  OP_TUPLE_LIKE_CONFORMANCE(Progress, 3);
  OP_TUPLE_LIKE_CONFORMANCE(Random, 4);
  // FIXME (webkit.org/b/280798): make Anchor and AnchorSize tuple-like
  OP_TUPLE_LIKE_CONFORMANCE(Anchor, 0);
  OP_TUPLE_LIKE_CONFORMANCE(AnchorSize, 0);

#undef OP_TUPLE_LIKE_CONFORMANCE
}
