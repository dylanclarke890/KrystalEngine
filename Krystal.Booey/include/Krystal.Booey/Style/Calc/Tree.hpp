#pragma once

#include "Krystal.Booey/CSS/Calc/Operator.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include <tuple>

/// @summary `style::calc::Tree` is A reduced representation of `css::calc::Tree` used in cases where
/// everything else (e.g all non-canonical dimensions) has been resolved except percentages, which can't be
/// resolved until they are used as they need some Value To resolve against. Currently, these are only used by
/// the `Length` type To represent <length-dimension> values, but are implemented generically so can be used
/// for any <*-dimension> type if the need ever arises.
namespace krys::boo::style::calc
{
  // Container.
  struct Tree;

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
  struct Exp;
  struct Log;
  struct Asin;
  struct Acos;
  struct Atan;
  struct Atan2;
  struct Pow;
  struct Sqrt;
  struct Hypot;
  struct Abs;
  struct Sign;
  struct Progress;
  struct Random;

  // Non-standard
  struct Blend;

  template <typename TOperation>
  concept Leaf = requires(TOperation) { TOperation::IsLeaf == true; };

  template <typename TOperation>
  concept Numeric = requires(TOperation) { TOperation::IsNumeric == true; };

  // Leaf Values
  struct Number
  {
    constexpr static bool IsLeaf = true;
    constexpr static bool IsNumeric = true;

    double Value;

    KRYS_NODISCARD bool operator==(const Number &) const noexcept = default;
  };

  struct Percentage
  {
    constexpr static bool IsLeaf = true;
    constexpr static bool IsNumeric = true;

    double Value;

    KRYS_NODISCARD bool operator==(const Percentage &) const noexcept = default;
  };

  struct Dimension
  {
    constexpr static bool IsLeaf = true;
    constexpr static bool IsNumeric = true;

    double Value;

    KRYS_NODISCARD bool operator==(const Dimension &) const noexcept = default;
  };

  template <typename TOperation>
  struct IndirectNode
  {
    UniquePtr<TOperation> Op;

    // Forward * and -> To the operation for convenience.
    KRYS_NODISCARD const TOperation &operator*() const noexcept
    {
      return *Op;
    }

    KRYS_NODISCARD TOperation &operator*() noexcept
    {
      return *Op;
    }

    KRYS_NODISCARD const TOperation *operator->() const noexcept
    {
      return Op.get();
    }

    KRYS_NODISCARD TOperation *operator->() noexcept
    {
      return Op.get();
    }

    operator const TOperation &() const noexcept
    {
      return *Op;
    }

    operator TOperation &() noexcept
    {
      return *Op;
    }

    KRYS_NODISCARD bool operator==(const IndirectNode<TOperation> &other) const noexcept
    {
      return *Op == *other.Op;
    }
  };

  using Node =
    Variant<Number, Percentage, Dimension, IndirectNode<Sum>, IndirectNode<Product>, IndirectNode<Negate>,
            IndirectNode<Invert>, IndirectNode<Min>, IndirectNode<Max>, IndirectNode<Clamp>,
            IndirectNode<RoundNearest>, IndirectNode<RoundUp>, IndirectNode<RoundDown>,
            IndirectNode<RoundToZero>, IndirectNode<Mod>, IndirectNode<Rem>, IndirectNode<Sin>,
            IndirectNode<Cos>, IndirectNode<Tan>, IndirectNode<Asin>, IndirectNode<Acos>, IndirectNode<Atan>,
            IndirectNode<Atan2>, IndirectNode<Pow>, IndirectNode<Sqrt>, IndirectNode<Hypot>,
            IndirectNode<Log>, IndirectNode<Exp>, IndirectNode<Abs>, IndirectNode<Sign>,
            IndirectNode<Progress>, IndirectNode<Random>, IndirectNode<Blend>>;

  struct Child
  {
    Node Value;

    template <typename T>
    requires std::constructible_from<Node, T>
    Child(T &&);

    KRYS_FORWARD_VARIANT_FUNCTIONS(Child, Value)

    KRYS_NODISCARD bool operator==(const Child &) const noexcept = default;
  };

  struct ChildOrNone
  {
    Variant<Child, css::keywords::None> Value;

    ChildOrNone(Child &&);
    ChildOrNone(css::keywords::None);

    KRYS_FORWARD_VARIANT_FUNCTIONS(ChildOrNone, Value)

    KRYS_NODISCARD bool operator==(const ChildOrNone &) const noexcept = default;
  };

  struct Children
  {
    using iterator = typename SmallList<Child>::iterator;
    using reverse_iterator = typename SmallList<Child>::reverse_iterator;
    using const_iterator = typename SmallList<Child>::const_iterator;
    using const_reverse_iterator = typename SmallList<Child>::const_reverse_iterator;
    using value_type = typename SmallList<Child>::value_type;

    SmallList<Child> Value;

    Children(Children &&) noexcept;
    Children(SmallList<Child> &&) noexcept;
    Children &operator=(Children &&) noexcept;
    Children &operator=(SmallList<Child> &&) noexcept;

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

    KRYS_NODISCARD Child &operator[](size_t i) noexcept;
    KRYS_NODISCARD const Child &operator[](size_t i) const noexcept;

    KRYS_NODISCARD bool operator==(const Children &) const noexcept = default;
  };

  struct Tree
  {
    Child Root;

    KRYS_NODISCARD bool operator==(const Tree &) const noexcept = default;
  };

  size_t ComputeDepth(const Tree &);

  // Math Operators.

  struct Sum
  {
    constexpr static auto Op = css::calc::Operator::Sum;

    Children Children;

    KRYS_NODISCARD bool operator==(const Sum &) const noexcept = default;
  };

  struct Product
  {
    constexpr static auto Op = css::calc::Operator::Product;

    Children Children;

    KRYS_NODISCARD bool operator==(const Product &) const noexcept = default;
  };

  struct Negate
  {
    constexpr static auto Op = css::calc::Operator::Negate;

    Child A;

    KRYS_NODISCARD bool operator==(const Negate &) const noexcept = default;
  };

  struct Invert
  {
    constexpr static auto Op = css::calc::Operator::Invert;

    Child A;

    KRYS_NODISCARD bool operator==(const Invert &) const noexcept = default;
  };

  // Math Functions

  // Comparison Functions - https://drafts.csswg.org/css-values-4/#comp-func
  struct Min
  {
    constexpr static auto Op = css::calc::Operator::Min;

    Children Children;

    KRYS_NODISCARD bool operator==(const Min &) const noexcept = default;
  };

  struct Max
  {
    constexpr static auto Op = css::calc::Operator::Max;

    Children Children;

    KRYS_NODISCARD bool operator==(const Max &) const noexcept = default;
  };

  struct Clamp
  {
    constexpr static auto Op = css::calc::Operator::Clamp;

    ChildOrNone Min;
    Child Val;
    ChildOrNone Max;

    KRYS_NODISCARD bool operator==(const Clamp &) const noexcept = default;
  };

  // Stepped Value Functions - https://drafts.csswg.org/css-values-4/#round-func
  struct RoundNearest
  {
    constexpr static auto Op = css::calc::Operator::RoundNearest;

    Child A;
    Maybe<Child> B;

    KRYS_NODISCARD bool operator==(const RoundNearest &) const noexcept = default;
  };

  struct RoundUp
  {
    constexpr static auto Op = css::calc::Operator::RoundUp;

    Child A;
    Maybe<Child> B;

    KRYS_NODISCARD bool operator==(const RoundUp &) const noexcept = default;
  };

  struct RoundDown
  {
    constexpr static auto Op = css::calc::Operator::RoundDown;

    Child A;
    Maybe<Child> B;

    KRYS_NODISCARD bool operator==(const RoundDown &) const noexcept = default;
  };

  struct RoundToZero
  {
    constexpr static auto Op = css::calc::Operator::RoundToZero;

    Child A;
    Maybe<Child> B;

    KRYS_NODISCARD bool operator==(const RoundToZero &) const noexcept = default;
  };

  struct Mod
  {
    constexpr static auto Op = css::calc::Operator::Mod;

    Child A;
    Child B;

    KRYS_NODISCARD bool operator==(const Mod &) const noexcept = default;
  };

  struct Rem
  {
    constexpr static auto Op = css::calc::Operator::Rem;

    Child A;
    Child B;

    KRYS_NODISCARD bool operator==(const Rem &) const noexcept = default;
  };

  // Trigonometric Functions - https://drafts.csswg.org/css-values-4/#trig-funcs
  struct Sin
  {
    constexpr static auto Op = css::calc::Operator::Sin;

    Child A;

    KRYS_NODISCARD bool operator==(const Sin &) const noexcept = default;
  };

  struct Cos
  {
    constexpr static auto Op = css::calc::Operator::Cos;

    Child A;

    KRYS_NODISCARD bool operator==(const Cos &) const noexcept = default;
  };

  struct Tan
  {
    constexpr static auto Op = css::calc::Operator::Tan;

    Child A;

    KRYS_NODISCARD bool operator==(const Tan &) const noexcept = default;
  };

  struct Asin
  {
    constexpr static auto Op = css::calc::Operator::Asin;

    Child A;

    KRYS_NODISCARD bool operator==(const Asin &) const noexcept = default;
  };

  struct Acos
  {
    constexpr static auto Op = css::calc::Operator::Acos;

    Child A;

    KRYS_NODISCARD bool operator==(const Acos &) const noexcept = default;
  };

  struct Atan
  {
    constexpr static auto Op = css::calc::Operator::Atan;

    Child A;

    KRYS_NODISCARD bool operator==(const Atan &) const noexcept = default;
  };

  struct Atan2
  {
    constexpr static auto Op = css::calc::Operator::Atan2;

    Child A;
    Child B;

    KRYS_NODISCARD bool operator==(const Atan2 &) const noexcept = default;
  };

  // Exponential Functions - https://drafts.csswg.org/css-values-4/#exponent-funcs
  struct Pow
  {
    constexpr static auto Op = css::calc::Operator::Pow;

    Child A;
    Child B;

    KRYS_NODISCARD bool operator==(const Pow &) const noexcept = default;
  };

  struct Sqrt
  {
    constexpr static auto Op = css::calc::Operator::Sqrt;

    Child A;

    KRYS_NODISCARD bool operator==(const Sqrt &) const noexcept = default;
  };

  struct Hypot
  {
    constexpr static auto Op = css::calc::Operator::Hypot;

    Children Children;

    KRYS_NODISCARD bool operator==(const Hypot &) const noexcept = default;
  };

  struct Log
  {
    constexpr static auto Op = css::calc::Operator::Log;

    Child A;
    Maybe<Child> B;

    KRYS_NODISCARD bool operator==(const Log &) const noexcept = default;
  };

  struct Exp
  {
    constexpr static auto Op = css::calc::Operator::Exp;

    Child A;

    KRYS_NODISCARD bool operator==(const Exp &) const noexcept = default;
  };

  // Sign-Related Functions - https://drafts.csswg.org/css-values-4/#sign-funcs
  struct Abs
  {
    constexpr static auto Op = css::calc::Operator::Abs;

    Child A;

    KRYS_NODISCARD bool operator==(const Abs &) const noexcept = default;
  };

  struct Sign
  {
    constexpr static auto Op = css::calc::Operator::Sign;

    Child A;

    KRYS_NODISCARD bool operator==(const Sign &) const noexcept = default;
  };

  // Progress-Related Functions - https://drafts.csswg.org/css-values-5/#CurrentProgress
  struct Progress
  {
    constexpr static auto Op = css::calc::Operator::Progress;

    Child CurrentProgress;
    Child From;
    Child To;

    KRYS_NODISCARD bool operator==(const Progress &) const noexcept = default;
  };

  // Random Function - https://drafts.csswg.org/css-values-5/#random
  struct Random
  {
    constexpr static auto Op = css::calc::Operator::Random;

    struct Fixed
    {
      double BaseValue;

      KRYS_NODISCARD bool operator==(const Fixed &) const noexcept = default;
    };

    Fixed FixedValue;
    Child Min;
    Child Max;
    Maybe<Child> Step;

    KRYS_NODISCARD bool operator==(const Random &) const noexcept = default;
  };

  // Non-standard
  struct Blend
  {
    constexpr static auto Op = css::calc::Operator::Blend;

    double CurrentProgress;
    Child From;
    Child To;

    KRYS_NODISCARD bool operator==(const Blend &) const noexcept = default;
  };

  static_assert(sizeof(Child) <= 16, "Child should stay small");

  // MARK: Construction

  // Default implementation of ChildConstruction used for all indirect nodes.
  template <typename TOperation>
  struct ChildConstruction
  {
    KRYS_NODISCARD static Child Create(TOperation &&Op) noexcept
    {
      return Child {IndirectNode<TOperation> {CreateUnique<TOperation>(krys::move(Op))}};
    }
  };

  // Specialized implementation of ChildConstruction for Number, needed To avoid `CreateUnique`.
  template <>
  struct ChildConstruction<Number>
  {
    KRYS_NODISCARD static Child Create(Number &&Op) noexcept
    {
      return Child {krys::move(Op)};
    }
  };

  // Specialized implementation of ChildConstruction for Percentage, needed To avoid `CreateUnique`.
  template <>
  struct ChildConstruction<Percentage>
  {
    KRYS_NODISCARD static Child Create(Percentage &&Op) noexcept
    {
      return Child {krys::move(Op)};
    }
  };

  // Specialized implementation of ChildConstruction for Dimension, needed To avoid `CreateUnique`.
  template <>
  struct ChildConstruction<Dimension>
  {
    KRYS_NODISCARD static Child Create(Dimension &&Op) noexcept
    {
      return Child {krys::move(Op)};
    }
  };

  template <typename TOperation>
  KRYS_NODISCARD Child CreateChild(TOperation &&Op) noexcept
  {
    return ChildConstruction<TOperation>::Create(krys::move(Op));
  }

  // Convenience constructors

  KRYS_NODISCARD inline Child CreateNumber(double value) noexcept
  {
    return CreateChild(Number {.Value = value});
  }

  KRYS_NODISCARD inline Child CreatePercentage(double value) noexcept
  {
    return CreateChild(Percentage {.Value = value});
  }

  KRYS_NODISCARD inline Child CreateDimension(double value) noexcept
  {
    return CreateChild(Dimension {.Value = value});
  }

  KRYS_NODISCARD inline Child Add(Child &&a, Child &&b) noexcept
  {
    SmallList<Child> sumChildren;
    sumChildren.push_back(krys::move(a));
    sumChildren.push_back(krys::move(b));
    return CreateChild(Sum {.Children = krys::move(sumChildren)});
  }

  KRYS_NODISCARD inline Child Multiply(Child &&a, Child &&b) noexcept
  {
    SmallList<Child> productChildren;
    productChildren.push_back(krys::move(a));
    productChildren.push_back(krys::move(b));
    return CreateChild(Product {.Children = krys::move(productChildren)});
  }

  KRYS_NODISCARD inline Child Subtract(Child &&a, Child &&b) noexcept
  {
    return Add(krys::move(a), CreateChild(Negate {.A = krys::move(b)}));
  }

  KRYS_NODISCARD inline Child BlendChildren(Child &&from, Child &&to, double progress) noexcept
  {
    return CreateChild(Blend {.CurrentProgress = progress, .From = krys::move(from), .To = krys::move(to)});
  }

  // MARK: Tuple Conformance

  // get<> overload (along with std::tuple_size and std::tuple_element below) To support destructuring of
  // operation nodes.

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Sum &root) noexcept
  {
    static_assert(!I);
    return root.Children;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Product &root) noexcept
  {
    static_assert(!I);
    return root.Children;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Negate &root) noexcept
  {
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Invert &root) noexcept
  {
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Min &root) noexcept
  {
    static_assert(!I);
    return root.Children;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Max &root) noexcept
  {
    static_assert(!I);
    return root.Children;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Clamp &root) noexcept
  {
    if constexpr (!I)
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
    if constexpr (!I)
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
    if constexpr (!I)
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
    if constexpr (!I)
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
    if constexpr (!I)
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
    if constexpr (!I)
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
    if constexpr (!I)
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
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Cos &root) noexcept
  {
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Tan &root) noexcept
  {
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Asin &root) noexcept
  {
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Acos &root) noexcept
  {
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Atan &root) noexcept
  {
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Atan2 &root) noexcept
  {
    if constexpr (!I)
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
    if constexpr (!I)
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
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Hypot &root) noexcept
  {
    static_assert(!I);
    return root.Children;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Log &root) noexcept
  {
    if constexpr (!I)
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
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Abs &root) noexcept
  {
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Sign &root) noexcept
  {
    static_assert(!I);
    return root.A;
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Progress &root) noexcept
  {
    if constexpr (!I)
    {
      return root.CurrentProgress;
    }
    else if constexpr (I == 1)
    {
      return root.From;
    }
    else if constexpr (I == 2)
    {
      return root.To;
    }
  }

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Random &root) noexcept
  {
    if constexpr (!I)
    {
      return root.FixedValue;
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
  KRYS_NODISCARD const auto &get(const Blend &root) noexcept
  {
    if constexpr (!I)
    {
      return root.CurrentProgress;
    }
    else if constexpr (I == 1)
    {
      return root.From;
    }
    else if constexpr (I == 2)
    {
      return root.To;
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

  inline ChildOrNone::ChildOrNone(css::keywords::None none) noexcept : Value(none)
  {
  }

  // MARK: Children Definition

  inline Children::Children(Children &&other) noexcept : Value(krys::move(other.Value))
  {
  }

  inline Children::Children(SmallList<Child> &&other) noexcept : Value(krys::move(other))
  {
  }

  inline Children &Children::operator=(Children &&other) noexcept
  {
    Value = krys::move(other.Value);
    return *this;
  }

  inline Children &Children::operator=(SmallList<Child> &&other) noexcept
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
}

namespace std
{

#define OP_TUPLE_LIKE_CONFORMANCE(Op, numberOfArguments)                                                     \
  template <>                                                                                                \
  class tuple_size<krys::boo::style::calc::Op> : public std::integral_constant<size_t, numberOfArguments>    \
  {                                                                                                          \
  };                                                                                                         \
  template <size_t I>                                                                                        \
  class tuple_element<I, krys::boo::style::calc::Op>                                                         \
  {                                                                                                          \
  public:                                                                                                    \
    using type = decltype(krys::boo::style::calc::get<I>(std::declval<krys::boo::style::calc::Op>()));       \
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
  OP_TUPLE_LIKE_CONFORMANCE(Blend, 3);

#undef OP_TUPLE_LIKE_CONFORMANCE
}
