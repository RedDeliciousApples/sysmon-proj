# C++ for Java Developers — sysmon edition

A quick reference for the C++ features used in this codebase, with short
examples. Each section points at where the feature appears in sysmon.

---

## 1. The mental model shift

| Java | C++ |
|---|---|
| Everything is a reference to a heap object | Variables **are** objects, living on the stack by default |
| Garbage collector cleans up | **Destructors** clean up, deterministically, at scope exit |
| One compilation unit per class | Headers (`.h`) declare, sources (`.cpp`) define; the **linker** stitches them together |
| `import` loads classes | `#include` **pastes text** into your file before compilation |
| `null` for "no value" | `std::optional<T>` for "no value" |
| Exceptions unchecked | Exceptions exist but are used sparingly; UB is the real danger |

The single biggest difference: in C++, copying is the default. Assigning a
`std::string` to another variable **copies the string**, not the reference.

---

## 2. Compilation model: headers, sources, the linker

Java: compile everything at once, imports resolve automatically.
C++: each `.cpp` is compiled **independently** into an `.o` file, then the
linker connects them. That's why headers exist — they're the contract between
translation units.

**Header (`loadavg.h`) — declarations only:**
```cpp
#pragma once            // include this file at most once per .cpp

struct LoadAverage {
    long double one_min;
    long double five_min;
    long double fifteen_min;
};

LoadAverage get_load_avg();   // declaration: "this function exists somewhere"
```

**Source (`loadavg.cpp`) — the definition:**
```cpp
#include "loadavg.h"    // paste the header's text here

LoadAverage get_load_avg() {   // definition: the actual code
    // ...
}
```

Rules of thumb:
- `#pragma once` at the top of every header (Java has no equivalent need).
- Headers should be **self-contained**: include everything they use.
- `#include <foo>` = system/library header; `#include "foo.h"` = your header.
- If you change a header, every `.cpp` that includes it recompiles — that's
  what the `.d` dependency files in our Makefile track.

---

## 3. Value semantics, references, pointers

```cpp
std::string a = "hello";
std::string b = a;              // COPIES the string (Java: b points to same object)
b += " world";
// a is still "hello"
```

**References** (`&`) — like a Java reference, but bound once, never null:
```cpp
void print(const std::string& s);   // no copy, read-only (very common)
void bump(int& x) { x++; }          // mutable alias
```

**Pointers** (`*`) — a reference that can be null and can be reseated:
```cpp
FILE* f = fopen("/proc/uptime", "r");
if (f == nullptr) { /* fopen returns nullptr on failure, never throws */ }
```

Where you'll see them in sysmon:
- `const std::string&` parameters in `storage.cpp` — pass big objects without copying.
- `FILE*` in every collector — C-style I/O returns a pointer; `nullptr` means failure.
- `const auto& fs : storage_usage` in `getjson.cpp` — loop without copying each element.

**Rule:** pass small things (int, double) by value; pass everything else as
`const T&` unless you need to modify it.

---

## 4. RAII — the thing that replaces garbage collection

Resource Acquisition Is Initialization: acquire in the constructor, release in
the destructor. The destructor runs **the moment the variable goes out of
scope** — no GC pause, no leaks (if you follow the pattern).

```cpp
void example() {
    std::ifstream file("/proc/mounts");   // opens in constructor
    // ... use file ...
}                                         // destructor closes it HERE, always,
                                          // even if an exception is thrown above
```

This is why `storage.cpp` uses `std::ifstream` (auto-closes) while the
`FILE*` collectors must call `fclose` manually — and why forgetting `fclose`
in `uptime.cpp` was a real bug.

The standard library is full of RAII types you should prefer:
- `std::fstream` — files
- `std::lock_guard<std::mutex>` — unlocks the mutex at scope exit
- `std::thread` — joins-or-terminates per its rules (see §10)
- `std::vector`, `std::string` — free their own memory

---

## 5. const correctness

`const` means "I promise not to modify" — the compiler enforces it.

```cpp
const std::uint64_t delta = current.total() - previous.total();  // never reassigned

class MetricsSampler {
    std::optional<MetricsSnapshot> latest() const;
    //                       ^^^^^^ promises not to modify the object
};
```

`const` on a member function is like Java's idea of a pure accessor, but
actually enforced. You'll get a compile error if you break the promise.

---

## 6. std::optional — the null replacement

Java's `null` is a trap. C++17's `std::optional<T>` is a value that may or may
not be there, and the compiler makes you check.

```cpp
std::optional<double> calculate_cpu_percent(const CpuCounters& prev,
                                            const CpuCounters& curr);

std::optional<double> pct = calculate_cpu_percent(a, b);

if (pct) {                       // has a value?
    double x = *pct;             // dereference to get it
}
double y = pct.value_or(0.0);    // value, or a default
```

In sysmon: `MetricsSnapshot::cpu_percent` is `std::optional<double>` because
the **first sample has no previous counters to diff against** — there is no
meaningful CPU percentage yet. `latest()` returns
`std::optional<MetricsSnapshot>` because the sampler may not have collected
anything yet. Java would use `null` for both; C++ makes the "maybe" explicit
in the type system.

---

## 7. auto and range-based for

```cpp
auto x = 42;                        // int — compiler deduces the type
auto snapshot = collect_once();     // MetricsSnapshot

for (const auto& fs : storage_usage) {   // like Java's for (var fs : list)
    // fs is a const FilesystemUsage&
}
```

`auto` is not "dynamic typing" — the type is fixed at compile time, you just
don't write it. Use it when the type is obvious or monstrous; write the type
when it aids readability.

---

## 8. structs, classes, constructors, member init lists

A `struct` and a `class` are identical in C++ except default visibility
(struct: public, class: private). Plain data bundles are usually `struct`.

```cpp
struct LoadAverage {
    long double one_min;
    long double five_min;
    long double fifteen_min;
};   // no constructor needed — value-initialized to zeros with `LoadAverage x{};`
```

Classes with invariants use constructors and **member initializer lists**
(the `:` syntax — preferred over assignment in the body):

```cpp
class MetricsSampler {
public:
    explicit MetricsSampler(std::chrono::seconds interval)
        : interval_(interval)          // initialize members here
    {
        if (interval_ <= std::chrono::seconds::zero()) {
            throw std::invalid_argument("Sampler interval must be positive");
        }
    }

private:
    std::chrono::seconds interval_;    // trailing underscore = member convention
};
```

Notes vs Java:
- `explicit` stops the compiler from silently converting `3s` into a
  `MetricsSampler` via an implicit one-argument constructor.
- There is no `new` needed: `MetricsSampler s{5s};` creates it on the stack.
  `new`/`delete` exist but you should almost never need them (RAII types own
  their memory).
- Members with defaults: `std::atomic<bool> stopping_{false};` — the `{false}`
  initializes it without a constructor.

---

## 9. static — two unrelated meanings (trap!)

**Meaning 1 — file-local functions** (like Java package-private, but stricter:
visible only in this `.cpp`):
```cpp
// storage.cpp
static bool is_skip_filesystem(const std::string& fs_type) { ... }
```
No other file can call or even link to it, even if it declares the same
signature. Use `static` for helpers that belong to one `.cpp`.

**Meaning 2 — static class members** (same as Java):
```cpp
class Foo {
    static int count;      // one per class, not per object
};
```

sysmon uses meaning 1 throughout the collectors.

---

## 10. Concurrency: thread, mutex, atomic, condition_variable

Same concepts as Java's `Thread`/`synchronized`/`volatile`-done-right, but
explicit and library-based.

**Thread** — runs a member function via a pointer-to-member:
```cpp
worker_ = std::thread(&MetricsSampler::run, this);   // start
worker_.join();                                      // wait for it to finish
```
A `std::thread` must be `join()`ed or `detach()`ed before its destructor runs,
or the program terminates. That's why `~MetricsSampler()` calls `stop()`.

**Mutex + lock_guard** — `synchronized` block, but RAII:
```cpp
{
    std::lock_guard<std::mutex> lock(snapshot_mutex_);  // locks in constructor
    latest_snapshot_ = snapshot;
}                                                        // unlocks here
```
Never call `lock()`/`unlock()` manually if a `lock_guard` will do — it can't
forget to unlock, even during an exception.

**Atomic** — lock-free flag for simple types:
```cpp
std::atomic<bool> stopping_{false};
stopping_ = true;                        // atomic store
while (!stopping_) { ... }               // atomic load
```
Like an `AtomicBoolean`, but with plain syntax.

**Condition variable** — how we'll fix `stop()` blocking for up to `interval_`:
```cpp
std::condition_variable cv_;
std::unique_lock<std::mutex> lock(snapshot_mutex_);
cv_.wait_for(lock, interval_, [this]{ return stopping_.load(); });
// wakes early when stop() calls cv_.notify_all(), otherwise times out
```
Java equivalent: `wait()`/`notify()`, but you always wait with a timeout or
predicate to avoid missed signals.

**The one Java habit to unlearn:** every object has a built-in monitor in
Java. In C++, you must pair each shared datum with an explicit mutex — there
is no `synchronized` keyword.

---

## 11. std::chrono — time without unit bugs

Java's `System.currentTimeMillis()` returns one unit. `std::chrono` makes the
unit part of the type, so mixing them up is a compile error.

```cpp
using namespace std::chrono;

seconds interval = seconds(5);
std::this_thread::sleep_for(interval);          // sleeps exactly 5s

auto now = system_clock::now().time_since_epoch();
std::int64_t unix_secs = duration_cast<seconds>(now).count();
```

`duration_cast<seconds>` truncates to whole seconds; `.count()` extracts the
raw number. In sysmon this produces `collected_at_unix_seconds`.

---

## 12. Templates in 60 seconds

Java generics are erased at runtime; C++ templates generate a **new compiled
function/class per type used**. You mostly *consume* templates here:

```cpp
std::vector<FilesystemUsage> filesystems;      // like ArrayList<FilesystemUsage>
std::optional<double> pct;                     // optional that holds a double
std::lock_guard<std::mutex> lock(m);           // RAII lock parameterized by mutex type
nlohmann::json j;
j["cpu"] = 12.5;                               // json is a template-heavy library
double cpu = j["cpu"].get<double>();           // .get<T>() is a template call
```

You rarely need to *write* templates for this project. If you see
`template<typename T>` in a header, read it as "this works for any T that
supports the operations used in the body."

---

## 13. Exceptions — same syntax, different culture

```cpp
throw std::runtime_error("Failed to open /proc/meminfo");

try {
    MetricsSnapshot s = collect_once();
} catch (const std::exception& e) {     // catch by const reference, always
    std::cerr << "Error: " << e.what() << '\n';
}
```

Differences from Java:
- No `throws` clauses; nothing forces callers to handle.
- Catch by `const&` — catching by value slices derived exception objects.
- `e.what()` is the equivalent of `getMessage()`.
- Culture: exceptions for **genuinely exceptional** failures (file vanished),
  `std::optional` for **expected** "not available yet" states (first CPU sample).
- The sampler's `run()` loop catches everything from collectors so one bad
  read doesn't kill the monitoring thread — that's the error policy.

---

## 14. Undefined behavior — the new failure mode

Java fails loudly (exceptions, `NullPointerException`). C++ has a category of
**undefined behavior**: code that compiles, sometimes works, and is allowed to
do *anything* — including silently corrupt data.

Common sources in this codebase:

```cpp
// 1. Uninitialized variables — reading them is UB (the mem.cpp bug)
long double total;                 // contains GARBAGE, not 0
fscanf(f, "MemTotal: %Lf", &total); // if this fails, total stays garbage
double pct = 100.0L * (1.0L - available / total);  // NaN or worse

// 2. Dereferencing null (the get_uptime() bug)
FILE* f = fopen("/proc/uptime", "r");   // returns nullptr on failure
fscanf(f, "%Lf", &x);                   // UB if f is null — no NPE, just demons

// 3. Out-of-bounds indexing — no ArrayIndexOutOfBoundsException
std::vector<int> v(3);
v[10] = 1;                              // UB; v.at(10) throws instead
```

Defenses: initialize with `{}` (`CpuCounters counters{};`), check every
`fopen`/`fscanf` return, prefer `.at()` over `[]` when unsure, and run the
sanitizer build (`-fsanitize=address,undefined`) — it turns UB into loud
errors at runtime.

---

## 15. Namespaces

Like Java packages, but per-file/per-block rather than per-directory:

```cpp
namespace sysmon {
    void helper();
}
sysmon::helper();

using namespace std::chrono;   // import names into scope (like Java's static import)
```

`nlohmann::json` is the library's namespace. Avoid `using namespace std;` in
headers — it pollutes every file that includes them.

---

## 16. Cheat sheet: Java → C++ translation

| Java | C++ (used in sysmon) |
|---|---|
| `ArrayList<T>` | `std::vector<T>` |
| `HashMap<K,V>` | `std::unordered_map<K,V>` |
| `String` | `std::string` |
| `null` check | `if (opt) ...` on `std::optional` |
| `synchronized` block | `std::lock_guard<std::mutex>` |
| `AtomicBoolean` | `std::atomic<bool>` |
| `Thread t = new Thread(r)` | `std::thread t(&F, obj)` |
| `t.join()` | `t.join()` (mandatory before destructor) |
| `System.currentTimeMillis()` | `duration_cast<seconds>(system_clock::now().time_since_epoch()).count()` |
| `interface` | abstract class with pure virtual: `virtual void f() = 0;` |
| `@Override` | `override` keyword |
| `package` | `namespace` |
| GC | RAII + destructors |
| `NullPointerException` | UB — prevent with `optional`/null checks |
| `import` | `#include` (text paste, not module load) |
| `final` param | `const` param |
| `static` method | `static` member or free function in a namespace |

---

## 17. Where these appear in sysmon — reading order

1. `loadavg.{h,cpp}` — smallest example of the header/source split
2. `math_utils.{h,cpp}` — free functions, `const`, `<cmath>`
3. `cpu.{h,cpp}` — structs with methods, `std::optional`, `fscanf` done right
4. `storage.cpp` — `std::ifstream` RAII, `static` helpers, `std::vector`, range-for
5. `metrics_snapshot.h` — plain data struct, `optional`, `vector` members
6. `metrics_sampler.{h,cpp}` — classes, init lists, thread/mutex/atomic, exceptions
7. `getjson.cpp` — templates as a consumer (`nlohmann::json`), `auto&` loops
8. `http_server.cpp` — C-style sockets, `std::ostringstream`, pointers to structs
