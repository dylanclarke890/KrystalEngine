# Job system (design)

A proposal for how the engine runs work on more than one thread. Nothing here is built yet; it's the design to agree on before the first commit. Open questions are at the end.

## Goals

- Use the machine's other cores for work that's self-contained: image decoding first, then file reads, glyph rasterisation, canvas encoding, and the game's own systems (physics, animation, asset streaming).
- Keep the rules simple enough that data races are hard to write by accident.
- Keep tests deterministic: code that dispatches jobs can be tested on one thread, with the test choosing when jobs run.
- Grow in steps: a working pool first, faster scheduling only once profiling asks for it.

Not goals, for now: a render thread, threaded compositing, fibers, or making Booey's DOM usable from more than one thread.

## The model

There are three kinds of thread.

| Thread | Count | Runs |
|---|---|---|
| **Main** | 1 | The window and its messages, input, game logic, and all of Booey (DOM, events, style, layout, painting) through `ui::View`. |
| **Workers** | Cores − 1 (at least 1) | Jobs: short, CPU-bound tasks that finish. They sleep when there's nothing to do. |
| **Dedicated** | A few, as needed | Work that blocks, has a deadline or never ends: audio, IO, later perhaps rendering. Each has its own named `Thread`. |

The main thread owns everything that isn't explicitly handed off. A job gets its inputs when it's dispatched, works only on them, and gives its result back by posting it to the thread that asked. That thread applies the result later, in its own time. A job never reaches into anyone else's state.

## API

It's a module of its own, **Krystal.Threading** (namespace `krys::threads`), laid out like the others: `include/Krystal.Threading/`, `src/`, and its tests in **Krystal.Threading.Tests**. It depends only on Krystal.Core. Booey and Krystal.UI depend on it for `WorkQueue` and `TaskInbox`, and the editor creates its `JobSystem`. `MoveOnlyFunc` is the exception: it's a general type, so it goes next to `Func` in Krystal.Core.

### `MoveOnlyFunc`

`Func` is `std::function`, which must be copyable, so a job couldn't own a `UniquePtr` or a `List` it was moved. Jobs use a new alias:

```cpp
template <typename T>
using MoveOnlyFunc = std::move_only_function<T>;
```

### `JobSystem`

```cpp
enum class JobPriority : uint8
{
  /// @brief Needed soon, such as a frame about to be drawn.
  Normal,
  /// @brief Needed eventually, such as streaming or a cache warm-up; runs when no Normal job is waiting.
  Background,
};

/// @brief A pool of worker threads that run jobs: functions that take what they need when they're dispatched
/// and touch nothing else.
class JobSystem
{
public:
  /// @brief Starts @p workerCount workers; none means one fewer than the machine has cores (at least one).
  explicit JobSystem(Maybe<uint32> workerCount = null) noexcept;
  /// @brief Stops taking jobs, runs the Normal ones queued, drops the Background ones, and joins the workers.
  ~JobSystem() noexcept;

  /// @brief Runs @p job on a worker, some time after this returns.
  void Dispatch(MoveOnlyFunc<void()> job, JobPriority priority = JobPriority::Normal) noexcept;

  /// @brief Runs @p job on a worker, then @p completion with its result on @p inbox.
  template <typename TResult>
  void Dispatch(MoveOnlyFunc<TResult()> job, TaskInbox &inbox, MoveOnlyFunc<void(TResult)> completion,
                JobPriority priority = JobPriority::Normal) noexcept;

  KRYS_NODISCARD uint32 WorkerCount() const noexcept;
};
```

The application creates one `JobSystem` at start-up, and destroys it at shutdown before anything its jobs post to. Everything else is handed it, or a `WorkQueue` (below); there's no hidden global.

### Getting results back: `TaskInbox`

A worker can't run a completion itself: that would touch the requester's state from the wrong thread. It posts the completion to a **task inbox**: a thread-safe queue of functions that its owning thread runs when it next drains it.

```cpp
/// @brief Somewhere a job can send its completion: a thread-safe queue of functions that its owning thread
/// runs, in the order they arrived, when it next drains it.
class TaskInbox
{
public:
  /// @brief Adds @p task; callable from any thread.
  void Post(MoveOnlyFunc<void()> task) noexcept;
  /// @brief Runs the tasks posted so far; only on the owning thread.
  void Drain() noexcept;
};
```

The main thread drains its inbox once a frame. Booey's `EventLoop` holds one, and `EventLoop::Run` drains it at the start of each run (and `RunUntilIdle` until it's empty), before the timers and tasks, with a microtask checkpoint after. A completion is the way back from steps the specifications run in parallel, so it queues a task, on the right task source and for the right document, for whatever touches the DOM; `NextWakeTime` is now while completions wait. The editor's game loop runs every frame, so nothing needs waking. A host that sleeps when idle can give an inbox a wake callback (`SetWakeHandler`), called from the posting thread, that pokes its message loop.

### Cancellation

A requester can disappear while its job runs: an `<img>` removed before its decode finishes. There are three tools, in order of preference:

1. **Check on arrival.** The completion holds a `WeakPtr` to the requester and does nothing if it's gone. The check happens on the requester's own thread, so an ordinary (not thread-safe) `WeakPtr` is fine, as long as it's only dereferenced in the completion, never in the job.
2. **Stop early.** For long jobs, the requester holds a `CancellationToken`, a shared atomic flag, and sets it when it goes away. The job checks it between steps (a decoder between rows) and returns early.
3. **Keep alive.** The completion holds a `Ref`, when the requester must see the result even if nothing else wants it.

### Dedicated threads: `Thread`

```cpp
/// @brief A named thread of its own that runs @p body until it returns; joined on destruction.
class Thread
{
public:
  Thread(utf8_string name, MoveOnlyFunc<void()> body) noexcept;
  ~Thread() noexcept;
};
```

It's a thin wrapper over `std::jthread` that sets the name the debugger and profilers show (`SetThreadDescription` on Windows). Audio and IO threads are built from it, with a `TaskInbox` each if they take requests.

### Main-thread checks

`IsMainThread()` (the thread the program started on) and `krys_debug_assert(IsMainThread())` go in the places that must never be reached from a worker: `EventLoop`, the `View` entry points, `Document`, `ValuePool`. Debug builds then catch a job that strays where it shouldn't.

## What a job may touch

This is the part that matters most. A job may use:

- **Values it owns,** moved into its lambda: a `List<byte>`, a `utf8_string`, a `UniquePtr`.
- **Immutable shared data** through a `Ref`: `RefCounted`'s counts are atomic, so passing a `Ref<SharedBuffer>` to a worker is safe, provided nobody changes the buffer. Booey's `SharedBuffer` is already immutable.
- **Thread-safe services** documented as such: the `JobSystem` and `TaskInbox`s, and later a thread-safe log.

A job may never use:

- **Booey's world:** nodes, documents, render objects, styles, `ValuePool`, `EventLoop::Current()`, `PlatformStrategies`, or a `WeakPtr` to any of them.
- **Anything mutable that another thread uses,** unless it's behind a lock and documented as thread-safe.
- **The calling thread's stack:** capture by value or move, never `[&]`, because the caller has moved on by the time the job runs.

A job must not throw (the engine is `noexcept`) and must not block on IO or on other jobs; blocking work goes to a dedicated thread.

## Implementation

**First version: one shared queue.** A mutex and condition variable guard two FIFO queues, one per priority. Workers wait on the condition variable, take the oldest Normal job (else the oldest Background one), and run it with the lock released. This is a couple of hundred lines, easy to get right, and fast enough for decodes and file reads, which take milliseconds each.

**Later, if profiling shows contention:** per-worker deques with work stealing (each worker pushes and pops its own jobs and steals from others when empty). This is what Unreal's and Unity's schedulers do, and it suits thousands of tiny jobs a frame, like a parallel physics step. The API doesn't change.

**Fork–join (`JobGroup`), when a system needs it:** dispatch many jobs and wait for all of them, for a parallel-for over particles or physics bodies. The waiting thread runs queued jobs while it waits rather than sleeping, so a wait can't deadlock the pool. This isn't needed for Booey and can come with the first game system that uses it.

**Shutdown:** the `JobSystem` destructor stops new dispatches, lets the Normal jobs finish, drops the Background ones, and joins the workers. Completions posted to inboxes afterwards are dropped when their inboxes are destroyed.

## Testing

- **Krystal.Threading.Tests** covers the pool itself with real threads: every job runs exactly once, priorities are respected, shutdown joins cleanly, and completions arrive on the inbox in order.
- **Everything else tests on one thread.** Booey and Krystal.UI don't take the `JobSystem` directly; they take a `WorkQueue` interface (`Dispatch(job, inbox, completion)`). In production that's the `JobSystem`; in tests it's a `ManualWorkQueue` that keeps jobs until the test calls `RunAll()`. A test can then check what happens before a decode finishes, after, and when the image goes away in between, with no timing involved. Booey's tests stay single-threaded and deterministic, as now.

## How Booey and Krystal.UI use it

1. **Image decoding.** `BitmapImageSource` dispatches "decode frame *n* from this `Ref<SharedBuffer>`" with a fresh decoder for the job. Decoders aren't shared between threads, so their state needs no locks. The completion, holding a `WeakPtr` to the source, caches the frame and calls `ImageFrameDecodeAtIndexHasFinished`, which triggers the repaint. This resolves the FIXMEs in `BitmapImageSource.cpp` and `ImageFrameAnimator.cpp` (decode ahead for animations), makes `decoding="async"` real and allows `decode()`. `PlatformStrategies` gains the `WorkQueue`; without one (a host that doesn't provide it), decoding stays synchronous, as now.
2. **File reads.** Krystal.UI's `FileLoaderStrategy` reads on an IO `Thread` and posts the response to Booey's event loop, which already receives responses as tasks, so Booey doesn't change.
3. **Later:** glyph rasterisation, `toDataURL`/`toBlob` encoding, SVG image rasterisation.

## Plan

1. The Krystal.Threading and Krystal.Threading.Tests projects; `MoveOnlyFunc` in Krystal.Core; `TaskInbox`, `JobSystem` (shared queue), `Thread` and `IsMainThread`, with their tests.
2. `WorkQueue` and `ManualWorkQueue` (in Krystal.Threading, so Booey's and Krystal.UI's tests share it); Booey's event loop drains a `TaskInbox`; the editor creates the `JobSystem`.
3. Asynchronous still-image decoding in Booey.
4. Decoding animation frames ahead.
5. `HTMLImageElement::decode()` and `SVGImageElement::decode()`.
6. `FileLoaderStrategy` reads on an IO thread.

## Open questions

- **Ownership:** should the `JobSystem` be created by `Application` and passed down (as proposed), or be an engine-wide singleton? Passing it down keeps tests and multiple engines in one process honest; a singleton is less plumbing.
- **Worker count:** cores − 1 leaves one core for the main thread. With an audio thread and the OS, some engines use cores − 2. Fine to start with cores − 1 and make it a setting.
- **IO:** a dedicated IO thread doing blocking reads (proposed, simple) or Windows IO completion ports (more reads in flight, more code). The `WorkQueue` and loader strategy APIs don't depend on the choice.
- **Fibers:** some engines (Naughty Dog's) run jobs on fibers so a job can wait mid-way without blocking its worker. It's powerful but hard to debug, so I'd not adopt it unless fork–join waiting proves too limiting.
