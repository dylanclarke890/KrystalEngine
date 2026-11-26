#include "Krystal.Web/Dom/Abort/AbortAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Dom/Abort/AbortController.hpp"
#include "Krystal.Web/Dom/Abort/AbortSignal.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventListenerHandle.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Types/Exception.hpp"

namespace krys::web::dom::tests
{
  TEST_CASE("Add: appends an algorithm, owned by the returned handle, unless the signal is aborted",
            "[AbortAlgorithms][dom:abortsignal-add]")
  {
    AbortController controller;
    int runs = 0;
    auto count = [&](const Exception &)
    {
      ++runs;
    };

    SECTION("The algorithm runs once when the signal aborts, and the handle then owns none")
    {
      auto algorithm = AbortAlgorithms::Add(count, controller.Signal());
      CHECK(algorithm);
      controller.Abort();
      CHECK(runs == 1);
      CHECK_FALSE(algorithm);
    }

    SECTION("Each algorithm added runs, even the same function added twice")
    {
      auto first = AbortAlgorithms::Add(count, controller.Signal());
      auto second = AbortAlgorithms::Add(count, controller.Signal());
      controller.Abort();
      CHECK(runs == 2);
    }

    SECTION("An aborted signal doesn't take it, and the handle owns none")
    {
      auto signal = AbortSignal::Abort();
      auto algorithm = AbortAlgorithms::Add(count, *signal);
      CHECK_FALSE(algorithm);
      AbortAlgorithms::SignalAbort(*signal);
      CHECK(runs == 0);
    }
  }

  TEST_CASE("Remove: the algorithm no longer runs", "[AbortAlgorithms][dom:abortsignal-remove]")
  {
    AbortController controller;
    int runs = 0;
    auto count = [&](const Exception &)
    {
      ++runs;
    };

    SECTION("Removing it through its handle")
    {
      auto algorithm = AbortAlgorithms::Add(count, controller.Signal());
      algorithm.Remove();
      CHECK_FALSE(algorithm);
      controller.Abort();
      CHECK(runs == 0);
    }

    SECTION("Destroying its handle")
    {
      {
        auto algorithm = AbortAlgorithms::Add(count, controller.Signal());
      }
      controller.Abort();
      CHECK(runs == 0);
    }

    SECTION("A released algorithm stays with its signal")
    {
      AbortAlgorithms::Add(count, controller.Signal()).Release();
      controller.Abort();
      CHECK(runs == 1);
    }

    SECTION("A handle moved from owns none, and the one moved to removes it")
    {
      auto algorithm = AbortAlgorithms::Add(count, controller.Signal());
      auto moved = krys::move(algorithm);
      CHECK_FALSE(algorithm); // NOLINT(bugprone-use-after-move): a moved-from handle is empty.
      CHECK(moved);
      moved.Remove();
      controller.Abort();
      CHECK(runs == 0);
    }
  }

  TEST_CASE("Signal abort: sets the reason once, then runs the abort steps for the signal and its dependents",
            "[AbortAlgorithms][dom:abortsignal-signal-abort]")
  {
    AbortController controller;
    auto &signal = controller.Signal();

    SECTION("An aborted signal keeps its reason")
    {
      AbortAlgorithms::SignalAbort(signal, Exception(ExceptionCode::AbortError, u8"1"));
      AbortAlgorithms::SignalAbort(signal, Exception(ExceptionCode::AbortError, u8"2"));
      CHECK(signal.Reason()->Message() == u8"1");
    }

    SECTION("Dependents are aborted before the signal's abort steps and run theirs after")
    {
      auto dependent = AbortSignal::Any({ShareRef(signal)});
      List<std::string> log;
      auto signalAlgorithm = AbortAlgorithms::Add(
        [&](const Exception &)
        {
          log.emplace_back("signal algorithm");
          CHECK(dependent->Aborted());
          CHECK(dependent->Reason()->Message() == u8"7");
        },
        signal);
      auto dependentAlgorithm =
        AbortAlgorithms::Add([&](const Exception &) { log.emplace_back("dependent algorithm"); }, *dependent);
      auto signalListener =
        signal.AddEventListener(event_names::Abort, [&](Event &) { log.emplace_back("signal event"); });
      auto dependentListener = dependent->AddEventListener(event_names::Abort, [&](Event &)
                                                           { log.emplace_back("dependent event"); });

      AbortAlgorithms::SignalAbort(signal, Exception(ExceptionCode::AbortError, u8"7"));
      CHECK(
        log
        == List<std::string> {"signal algorithm", "signal event", "dependent algorithm", "dependent event"});
    }
  }

  TEST_CASE("Run the abort steps: runs and empties the abort algorithms, then fires abort",
            "[AbortAlgorithms][dom:run-the-abort-steps][dom:abortsignal-abort-algorithms]")
  {
    AbortController controller;
    auto &signal = controller.Signal();
    List<std::string> log;

    SECTION("Algorithms run in order, with the reason, before the event")
    {
      auto first = AbortAlgorithms::Add(
        [&](const Exception &reason)
        {
          log.emplace_back("first");
          CHECK(reason.Message() == u8"4");
        },
        signal);
      auto second = AbortAlgorithms::Add([&](const Exception &) { log.emplace_back("second"); }, signal);
      auto listener =
        signal.AddEventListener(event_names::Abort, [&](Event &) { log.emplace_back("event"); });

      controller.Abort(Exception(ExceptionCode::AbortError, u8"4"));
      CHECK(log == List<std::string> {"first", "second", "event"});
    }

    SECTION("An algorithm that removes itself doesn't skip the next")
    {
      AbortAlgorithmHandle self;
      self = AbortAlgorithms::Add(
        [&](const Exception &)
        {
          log.emplace_back("first");
          self.Remove();
        },
        signal);
      auto second = AbortAlgorithms::Add([&](const Exception &) { log.emplace_back("second"); }, signal);

      controller.Abort();
      CHECK(log == List<std::string> {"first", "second"});
    }
  }
}
