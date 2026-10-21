#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief Maps an animation's input progress to its output progress (an easing function).
  class TimingFunction : public RefCounted<TimingFunction>
  {
  public:
    enum class Type : uint8
    {
      LinearFunction,
      CubicBezierFunction,
      StepsFunction,
      SpringFunction
    };

    enum class Before : bool
    {
      No,
      Yes
    };

    virtual ~TimingFunction() noexcept = default;

    KRYS_NODISCARD virtual Ref<TimingFunction> Clone() const noexcept = 0;
    KRYS_NODISCARD virtual Type GetType() const noexcept = 0;

    KRYS_NODISCARD bool IsLinearTimingFunction() const noexcept
    {
      return GetType() == Type::LinearFunction;
    }
    KRYS_NODISCARD bool IsCubicBezierTimingFunction() const noexcept
    {
      return GetType() == Type::CubicBezierFunction;
    }
    KRYS_NODISCARD bool IsStepsTimingFunction() const noexcept
    {
      return GetType() == Type::StepsFunction;
    }
    KRYS_NODISCARD bool IsSpringTimingFunction() const noexcept
    {
      return GetType() == Type::SpringFunction;
    }

    KRYS_NODISCARD bool operator==(const TimingFunction &other) const noexcept
    {
      return Equals(other);
    }
    KRYS_NODISCARD virtual bool Equals(const TimingFunction &) const noexcept = 0;

    KRYS_NODISCARD double TransformProgress(double progress, double duration,
                                            Before = Before::No) const noexcept;
    KRYS_NODISCARD utf8_string CssText() const noexcept;
  };

  class LinearTimingFunction final : public TimingFunction
  {
  public:
    struct Point
    {
      double Value;
      double Progress;

      KRYS_NODISCARD bool operator==(const Point &) const noexcept = default;
    };

  private:
    const List<Point> _points;

  public:
    KRYS_NODISCARD static Ref<LinearTimingFunction> Create() noexcept
    {
      return Create({});
    }

    KRYS_NODISCARD static Ref<LinearTimingFunction> Create(const List<Point> &points) noexcept
    {
      return AdoptRef(*new LinearTimingFunction(points));
    }

    KRYS_NODISCARD bool Equals(const TimingFunction &other) const noexcept final;

    KRYS_NODISCARD const List<Point> &Points() const noexcept
    {
      return _points;
    }

    KRYS_NODISCARD static const LinearTimingFunction &Identity() noexcept
    {
      static NeverDestroyed<Ref<LinearTimingFunction>> function {Create()};
      return *function.get();
    }

  private:
    explicit LinearTimingFunction(const List<Point> &points) noexcept : _points(points)
    {
      krys_debug_assert(_points.empty() || _points.size() >= 2);
    }

    KRYS_NODISCARD Type GetType() const noexcept final
    {
      return Type::LinearFunction;
    }
    KRYS_NODISCARD Ref<TimingFunction> Clone() const noexcept final
    {
      return AdoptRef(*new LinearTimingFunction(_points));
    }
  };

  class CubicBezierTimingFunction final : public TimingFunction
  {
  public:
    enum class TimingFunctionPreset : uint8
    {
      Ease,
      EaseIn,
      EaseOut,
      EaseInOut,
      Custom
    };

  private:
    const double _x1;
    const double _y1;
    const double _x2;
    const double _y2;
    TimingFunctionPreset _timingFunctionPreset;

  public:
    KRYS_NODISCARD static Ref<CubicBezierTimingFunction> Create() noexcept
    {
      return Create(TimingFunctionPreset::Ease, 0.25, 0.1, 0.25, 1.0);
    }

    KRYS_NODISCARD static Ref<CubicBezierTimingFunction> Create(double x1, double y1, double x2,
                                                                double y2) noexcept
    {
      return AdoptRef(*new CubicBezierTimingFunction(TimingFunctionPreset::Custom, x1, y1, x2, y2));
    }

    KRYS_NODISCARD static Ref<CubicBezierTimingFunction> Create(TimingFunctionPreset preset, double x1,
                                                                double y1, double x2, double y2) noexcept
    {
      return AdoptRef(*new CubicBezierTimingFunction(preset, x1, y1, x2, y2));
    }

    KRYS_NODISCARD static Ref<CubicBezierTimingFunction> Create(TimingFunctionPreset) noexcept;

    KRYS_NODISCARD bool Equals(const TimingFunction &other) const noexcept final;

    KRYS_NODISCARD double X1() const noexcept
    {
      return _x1;
    }
    KRYS_NODISCARD double Y1() const noexcept
    {
      return _y1;
    }
    KRYS_NODISCARD double X2() const noexcept
    {
      return _x2;
    }
    KRYS_NODISCARD double Y2() const noexcept
    {
      return _y2;
    }

    KRYS_NODISCARD TimingFunctionPreset GetTimingFunctionPreset() const noexcept
    {
      return _timingFunctionPreset;
    }
    void SetTimingFunctionPreset(TimingFunctionPreset preset) noexcept
    {
      _timingFunctionPreset = preset;
    }

    KRYS_NODISCARD static const CubicBezierTimingFunction &DefaultTimingFunction() noexcept
    {
      static NeverDestroyed<Ref<CubicBezierTimingFunction>> function {Create()};
      return *function.get();
    }

    KRYS_NODISCARD Ref<CubicBezierTimingFunction> CreateReversed() const noexcept
    {
      return Create(TimingFunctionPreset::Custom, 1.0 - _x2, 1.0 - _y2, 1.0 - _x1, 1.0 - _y1);
    }

    KRYS_NODISCARD bool IsLinear() const noexcept
    {
      return (!_x1 && !_y1 && !_x2 && !_y2) || (_x1 == 1.0 && _y1 == 1.0 && _x2 == 1.0 && _y2 == 1.0)
             || (_x1 == 0.0 && _y1 == 0.0 && _x2 == 1.0 && _y2 == 1.0);
    }

  private:
    explicit CubicBezierTimingFunction(TimingFunctionPreset preset, double x1, double y1, double x2,
                                       double y2) noexcept
        : _x1(x1), _y1(y1), _x2(x2), _y2(y2), _timingFunctionPreset(preset)
    {
    }

    KRYS_NODISCARD Type GetType() const noexcept final
    {
      return Type::CubicBezierFunction;
    }
    KRYS_NODISCARD Ref<TimingFunction> Clone() const noexcept final
    {
      return AdoptRef(*new CubicBezierTimingFunction(_timingFunctionPreset, _x1, _y1, _x2, _y2));
    }
  };

  class StepsTimingFunction final : public TimingFunction
  {
  public:
    enum class StepPosition : uint8
    {
      JumpStart,
      JumpEnd,
      JumpNone,
      JumpBoth,
      Start,
      End,
    };

  private:
    const int _steps;
    const Maybe<StepPosition> _stepPosition;

  public:
    KRYS_NODISCARD static Ref<StepsTimingFunction> Create(int steps,
                                                          Maybe<StepPosition> stepPosition) noexcept
    {
      return AdoptRef(*new StepsTimingFunction(steps, stepPosition));
    }
    KRYS_NODISCARD static Ref<StepsTimingFunction> Create() noexcept
    {
      return AdoptRef(*new StepsTimingFunction(1, StepPosition::End));
    }

    KRYS_NODISCARD bool Equals(const TimingFunction &other) const noexcept final;

    KRYS_NODISCARD int NumberOfSteps() const noexcept
    {
      return _steps;
    }

    KRYS_NODISCARD Maybe<StepPosition> GetStepPosition() const noexcept
    {
      return _stepPosition;
    }

  private:
    StepsTimingFunction(int steps, Maybe<StepPosition> stepPosition) noexcept
        : _steps(steps), _stepPosition(stepPosition)
    {
    }

    KRYS_NODISCARD Type GetType() const noexcept final
    {
      return Type::StepsFunction;
    }
    KRYS_NODISCARD Ref<TimingFunction> Clone() const noexcept final
    {
      return AdoptRef(*new StepsTimingFunction(_steps, _stepPosition));
    }
  };

  class SpringTimingFunction final : public TimingFunction
  {
  private:
    const double _mass;
    const double _stiffness;
    const double _damping;
    const double _initialVelocity;

  public:
    KRYS_NODISCARD static Ref<SpringTimingFunction> Create(double mass, double stiffness, double damping,
                                                           double initialVelocity) noexcept
    {
      return AdoptRef(*new SpringTimingFunction(mass, stiffness, damping, initialVelocity));
    }

    KRYS_NODISCARD bool Equals(const TimingFunction &other) const noexcept final;

    KRYS_NODISCARD double Mass() const noexcept
    {
      return _mass;
    }
    KRYS_NODISCARD double Stiffness() const noexcept
    {
      return _stiffness;
    }
    KRYS_NODISCARD double Damping() const noexcept
    {
      return _damping;
    }
    KRYS_NODISCARD double InitialVelocity() const noexcept
    {
      return _initialVelocity;
    }

  private:
    explicit SpringTimingFunction(double mass, double stiffness, double damping,
                                  double initialVelocity) noexcept
        : _mass(mass), _stiffness(stiffness), _damping(damping), _initialVelocity(initialVelocity)
    {
    }

    KRYS_NODISCARD Type GetType() const noexcept final
    {
      return Type::SpringFunction;
    }
    KRYS_NODISCARD Ref<TimingFunction> Clone() const noexcept final
    {
      return AdoptRef(*new SpringTimingFunction(_mass, _stiffness, _damping, _initialVelocity));
    }
  };

  TextStream &operator<<(TextStream &ts, const TimingFunction &timingFunction) noexcept;
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_TIMING_FUNCTION(ToValueTypeName, predicate)                              \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::TimingFunction &function) noexcept                    \
    {                                                                                                        \
      return function.predicate;                                                                             \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_TRAITS_TIMING_FUNCTION(krys::boo::LinearTimingFunction, IsLinearTimingFunction())
KRYS_SPECIALIZE_TYPE_TRAITS_TIMING_FUNCTION(krys::boo::CubicBezierTimingFunction,
                                            IsCubicBezierTimingFunction())
KRYS_SPECIALIZE_TYPE_TRAITS_TIMING_FUNCTION(krys::boo::StepsTimingFunction, IsStepsTimingFunction())
KRYS_SPECIALIZE_TYPE_TRAITS_TIMING_FUNCTION(krys::boo::SpringTimingFunction, IsSpringTimingFunction())
