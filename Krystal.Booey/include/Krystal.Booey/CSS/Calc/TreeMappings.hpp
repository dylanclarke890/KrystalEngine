#pragma once

#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Calc/Tree.hpp"

namespace krys::boo::css::calc
{
  template <typename>
  struct ToCalculationMapping;

  template <>
  struct ToCalculationMapping<Sum>
  {
    using type = style::calc::Sum;
  };

  template <>
  struct ToCalculationMapping<Product>
  {
    using type = style::calc::Product;
  };

  template <>
  struct ToCalculationMapping<Negate>
  {
    using type = style::calc::Negate;
  };

  template <>
  struct ToCalculationMapping<Invert>
  {
    using type = style::calc::Invert;
  };

  template <>
  struct ToCalculationMapping<Min>
  {
    using type = style::calc::Min;
  };

  template <>
  struct ToCalculationMapping<Max>
  {
    using type = style::calc::Max;
  };

  template <>
  struct ToCalculationMapping<Clamp>
  {
    using type = style::calc::Clamp;
  };

  template <>
  struct ToCalculationMapping<RoundNearest>
  {
    using type = style::calc::RoundNearest;
  };

  template <>
  struct ToCalculationMapping<RoundUp>
  {
    using type = style::calc::RoundUp;
  };

  template <>
  struct ToCalculationMapping<RoundDown>
  {
    using type = style::calc::RoundDown;
  };

  template <>
  struct ToCalculationMapping<RoundToZero>
  {
    using type = style::calc::RoundToZero;
  };

  template <>
  struct ToCalculationMapping<Mod>
  {
    using type = style::calc::Mod;
  };

  template <>
  struct ToCalculationMapping<Rem>
  {
    using type = style::calc::Rem;
  };

  template <>
  struct ToCalculationMapping<Sin>
  {
    using type = style::calc::Sin;
  };

  template <>
  struct ToCalculationMapping<Cos>
  {
    using type = style::calc::Cos;
  };

  template <>
  struct ToCalculationMapping<Tan>
  {
    using type = style::calc::Tan;
  };

  template <>
  struct ToCalculationMapping<Asin>
  {
    using type = style::calc::Asin;
  };

  template <>
  struct ToCalculationMapping<Acos>
  {
    using type = style::calc::Acos;
  };

  template <>
  struct ToCalculationMapping<Atan>
  {
    using type = style::calc::Atan;
  };

  template <>
  struct ToCalculationMapping<Atan2>
  {
    using type = style::calc::Atan2;
  };

  template <>
  struct ToCalculationMapping<Pow>
  {
    using type = style::calc::Pow;
  };

  template <>
  struct ToCalculationMapping<Sqrt>
  {
    using type = style::calc::Sqrt;
  };

  template <>
  struct ToCalculationMapping<Hypot>
  {
    using type = style::calc::Hypot;
  };

  template <>
  struct ToCalculationMapping<Log>
  {
    using type = style::calc::Log;
  };

  template <>
  struct ToCalculationMapping<Exp>
  {
    using type = style::calc::Exp;
  };

  template <>
  struct ToCalculationMapping<Abs>
  {
    using type = style::calc::Abs;
  };

  template <>
  struct ToCalculationMapping<Sign>
  {
    using type = style::calc::Sign;
  };

  template <>
  struct ToCalculationMapping<Random>
  {
    using type = style::calc::Random;
  };

  template <>
  struct ToCalculationMapping<Progress>
  {
    using type = style::calc::Progress;
  };

  template <typename>
  struct ToCalcMapping;
  template <>

  struct ToCalcMapping<style::calc::Sum>
  {
    using type = Sum;
  };

  template <>
  struct ToCalcMapping<style::calc::Product>
  {
    using type = Product;
  };

  template <>
  struct ToCalcMapping<style::calc::Negate>
  {
    using type = Negate;
  };

  template <>
  struct ToCalcMapping<style::calc::Invert>
  {
    using type = Invert;
  };

  template <>
  struct ToCalcMapping<style::calc::Min>
  {
    using type = Min;
  };

  template <>
  struct ToCalcMapping<style::calc::Max>
  {
    using type = Max;
  };

  template <>
  struct ToCalcMapping<style::calc::Clamp>
  {
    using type = Clamp;
  };

  template <>
  struct ToCalcMapping<style::calc::RoundNearest>
  {
    using type = RoundNearest;
  };

  template <>
  struct ToCalcMapping<style::calc::RoundUp>
  {
    using type = RoundUp;
  };

  template <>
  struct ToCalcMapping<style::calc::RoundDown>
  {
    using type = RoundDown;
  };

  template <>
  struct ToCalcMapping<style::calc::RoundToZero>
  {
    using type = RoundToZero;
  };

  template <>
  struct ToCalcMapping<style::calc::Mod>
  {
    using type = Mod;
  };

  template <>
  struct ToCalcMapping<style::calc::Rem>
  {
    using type = Rem;
  };

  template <>
  struct ToCalcMapping<style::calc::Sin>
  {
    using type = Sin;
  };

  template <>
  struct ToCalcMapping<style::calc::Cos>
  {
    using type = Cos;
  };

  template <>
  struct ToCalcMapping<style::calc::Tan>
  {
    using type = Tan;
  };

  template <>
  struct ToCalcMapping<style::calc::Asin>
  {
    using type = Asin;
  };

  template <>
  struct ToCalcMapping<style::calc::Acos>
  {
    using type = Acos;
  };

  template <>
  struct ToCalcMapping<style::calc::Atan>
  {
    using type = Atan;
  };

  template <>
  struct ToCalcMapping<style::calc::Atan2>
  {
    using type = Atan2;
  };

  template <>
  struct ToCalcMapping<style::calc::Pow>
  {
    using type = Pow;
  };

  template <>
  struct ToCalcMapping<style::calc::Sqrt>
  {
    using type = Sqrt;
  };

  template <>
  struct ToCalcMapping<style::calc::Hypot>
  {
    using type = Hypot;
  };

  template <>
  struct ToCalcMapping<style::calc::Log>
  {
    using type = Log;
  };

  template <>
  struct ToCalcMapping<style::calc::Exp>
  {
    using type = Exp;
  };

  template <>
  struct ToCalcMapping<style::calc::Abs>
  {
    using type = Abs;
  };

  template <>
  struct ToCalcMapping<style::calc::Sign>
  {
    using type = Sign;
  };

  template <>
  struct ToCalcMapping<style::calc::Random>
  {
    using type = Random;
  };

  template <>
  struct ToCalcMapping<style::calc::Progress>
  {
    using type = Progress;
  };

  template <typename T>
  using ToCalculationTreeOp = typename ToCalculationMapping<T>::type;

  template <typename T>
  using ToCalcTreeOp = typename ToCalcMapping<T>::type;
}
