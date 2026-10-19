# Coding standards

This project follows the **[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)**
(consulted 2026-10-19), with the deliberate deviations listed in §4. Where this document and the
Google guide disagree, **this document wins** — every deviation is there on purpose, with its reason.

The rules have a machine-readable form, and that form is normative:

| Half | File | Checked by |
|---|---|---|
| Formatting | [`.clang-format`](../.clang-format) | `tools\check-format.bat` |
| Naming | [`.clang-tidy`](../.clang-tidy) | `tools\check-naming.bat` |
| Editor defaults | [`.editorconfig`](../.editorconfig) | editors |

Read this document to know *what* the rules are and *why*; read the config files to know precisely
what the tools enforce. If the two ever disagree, the config is wrong and should be fixed, not the
document.

**Scope.** `Engine/`, `Editor/`, `Game/` and `UnitTests/`. Out of scope: `Thirdparty/` (vendored,
not ours to reformat), CMake files, GLSL shaders (`*.vert`, `*.frag`), and assets under
`Resources/`.

---

## 1. Naming

| Kind | Rule | Example |
|---|---|---|
| Types — class, struct, union, enum, alias | `PascalCase` | `ECSManager`, `OrthoView`, `GizmoMode` |
| Functions and methods, **including accessors** | `PascalCase()` | `CreateEntity()`, `GetZoom()`, `UpdateUI()` |
| Variables, parameters, locals | `snake_case` | `viewport_active`, `delta_time` |
| Namespaces | `snake_case` | `rse`, `editor_bounds` |
| Public data members (and therefore struct members) | `snake_case` | `center`, `half_extent` |
| Private and protected data members | `m_snake_case` | `m_selected_entity`, `m_shader_id` |
| Static data members | as the variable rule, prefix kept | `m_settings`, `s_keys_down` |
| Constants and `constexpr` at namespace or class scope | `kPascalCase` | `kOrthoMinZoom`, `kOutlineHalo` |
| Enumerators | `kPascalCase` | `GizmoMode::kTranslate`, `SystemPhase::kPreScene` |
| Macro definitions | `ALL_CAPS` | `NOMINMAX`, `RENDERER_MAX_SPRITES` |
| Global variables | `g_snake_case` | `g_frame_count` (none exist today) |
| File names | `snake_case` | `ortho_view.h`, `editor_camera.cpp` |
| Directories | `snake_case`, below the module root | `Engine/Engine/render/`, `Editor/Source/panels/` |
| Test suites | `PascalCase`, suffixed `Test` | `TEST(OrthoViewTest, …)` |

### 1.1 Acronyms

An acronym keeps the casing it already has; the tools enforce the first letter, not the middle. So
`updateUI()` becomes `UpdateUI()`, not `UpdateUi()`, and `GetDPI()` stays `GetDPI()`. The one place
acronyms are lowercased is **file names**, which are entirely lowercase, where the acronym is treated
as one word: `opengl_renderer.h`, `imgui_layer.h`, `renderable_2d.h`.

### 1.2 Hungarian notation

There is none. An enum called `eComponents` is `Components`; `Camera_Movement` is `CameraMovement`.
Types say what they are; they do not repeat it in the first letter.

### 1.3 What the naming check cannot see

Test **suite** names are arguments to the `TEST(...)` macro, not identifiers, so the check does not
enforce them — reviewers do. File and directory names are likewise enforced by review (and by the
compiler, which insists that an `#include` match a real file).

---

## 2. Formatting and layout

`.clang-format` is a Google base with these settings, which are the ones worth knowing:

| Setting | Value | Note |
|---|---|---|
| `ColumnLimit` | **120** | Google says 80; we chose 120 (§4) |
| `IndentWidth` | 2 | Google's default |
| `UseTab` | `Never` | the tree used to be tab-indented |
| `BreakBeforeBraces` | `Attach` | K&R, not Allman |
| `PointerAlignment` | `Left` | `const Foo* p`, not `const Foo *p` |
| `NamespaceIndentation` | `None` | |
| `Standard` | `c++20` | the project builds as C++20 |
| `SortIncludes` | `CaseSensitive`, regrouped | with categories for this repo's include roots |

### 2.1 Files and includes

- Headers are `.h`; sources are `.cpp`. (The tree used to be split between `.hpp` and `.h`.)
- Every header starts with `#pragma once`. No include guards.
- Includes are grouped: the matching header first, then the C++ standard library, then third-party
  (`<glm/...>`, `<SDL3/...>`, `<entt/...>`), then our own headers.
- Our own headers are included as `<dir/name.h>` across directories and as `"name.h"` within one. The
  include roots are `Engine/Engine`, `Editor/Source` and `Engine/` itself, which is what puts the
  debug helper under `Engine/tests` on the path, so `#include <render/ortho_view.h>`,
  `#include <panels/inspector_panel.h>` and `#include <tests/test.h>` all resolve.

### 2.2 Line endings

The repository is CRLF throughout, and `.editorconfig` records that. Keep it that way.

---

## 3. Language features

C++20, and the Google guide's rules on features stand as written. In particular: no exceptions in
*interfaces* that promise not to throw, `nullptr` rather than `NULL` or `0`, no C-style casts, no
`using namespace` in headers, `const` on everything that is not mutated.

---

## 4. Deviations from the Google guide

Every one of these is a decision, not an oversight.

| Deviation | Reason |
|---|---|
| **120-column limit** | Chosen for this project; it also matches how the code is already written, so the reflow was small |
| **`.cpp` and `.h`**, not `.cc` and `.h` | Windows-first project; also unifies the old `.hpp`/`.h` split |
| **`#pragma once`**, not include guards | Already true of every header in the tree |
| **`m_snake_case` members**, not `snake_case_` | The `m_` prefix is already 100% consistent across the existing code; the `_` suffix would have replaced a working convention with a different one for no gain. Also keeps static members (`m_settings`, `s_keys_down`) consistent, which the `_` suffix cannot do (see below) |
| **Public/struct data members are unsuffixed** | Google would suffix public class members too. The suffix marks *encapsulation*, and a struct's members are its interface |
| **Static data members are unsuffixed** | `clang-tidy` classifies static data members as variables, so they follow the variable rule: `m_settings`, `s_keys_down`. This is also how the code already spells them |
| **Function-local `const`/`constexpr` follow the variable rule** | Google's `k` rule would rename a local `const float height` to `kHeight`. For short-lived locals that reads worse, so `k` marks constants that are part of an interface |
| **Accessors are `PascalCase`** | Google allows accessors to look like variables (`count()`). One rule is easier to apply and to check than two |
| **Project includes are `<dir/name.h>`** | See §2.1 — CMake puts the module root on the include path, so Google's repo-root-relative quoted form does not resolve |
| **Exceptions are permitted** | The engine uses them deliberately for load/save failure; removing them would be a design change, not a style change |
| **`_test.h` for unit tests** | Google's `_unittest.cc` spelled the way the rest of the tree names test files |
| **Framework hooks keep the framework's spelling** | cereal resolves its hooks by name (`serialize`, `save`/`load`, `*_minimal`, `load_and_construct`). Renaming one still compiles, but the type quietly drops out of serialization, so the mistake surfaces later and somewhere else |
| **`createWindow` stays `camelCase`** | `winuser.h` defines `CreateWindow` as a macro, so the PascalCase spelling cannot be declared at all |

The last two rows are the only names exempted in `.clang-tidy`, and they are exempted by name rather than
by rule, so nothing else escapes the check.

---

## 5. Not enforced by tooling

These are review-time matters. They are listed so that "the tools pass" is never mistaken for "the
code is good".

- **Include discipline** — include what you use, forward-declare instead of including.
- **`explicit`** on single-argument constructors.
- **`const`-correctness**, `[[nodiscard]]`, `std::string_view` over `const std::string&`.
- **Magic numbers** worth naming; **macros** that should be `constexpr`; dead code.
- **`struct` versus `class`** — spelled out here because the choice decides a naming rule (§1).
- **Comment quality**: explain *why*, not *what*.

---

## 6. Checking your work

The tools are pinned in one place — `tools\_llvm.bat` finds them — and the same scripts run locally and
in CI, so "it passes on my machine" and "it passes in CI" are the same statement.

```bat
tools\format.bat                 :: reformat every source file in place
tools\check-format.bat           :: fail if any file is not clang-format clean
tools\check-naming.bat x64-debug :: fail if any translation unit has a naming violation
```

`check-naming.bat` needs a **built** tree — configure it and then build it — because `clang-tidy`
reads `out/<preset>/compile_commands.json`, and the compile commands MSVC generates carry
`.obj.modmap` response files that only exist once something has actually been compiled. The other two
scripts need nothing at all.

Both checks are complete-or-nothing by design: a rename that is 95% done looks like a convention,
which is worse than one not started. That is also why the gates are **verified against deliberately
broken input** — an instrument that cannot fail proves nothing.

### 6.1 A note on versions

`clang-format` output changes between major versions, so CI installs the same release line we build with
locally, LLVM 19.1.x. If they ever drift, every file in the tree will appear to need reformatting at once.

`clang-tidy` cannot be pinned that way. A hosted runner ships a newer MSVC than a developer machine,
and the MSVC STL refuses to be parsed by an old Clang at all (`STL1000: expected Clang 20 or newer`),
so CI runs the newest LLVM that can read the runner's toolchain. The naming **rules** are what must
agree — and they are shared, since `.clang-tidy` and the scripts are. This is the one place where
"passes locally" and "passes in CI" are only approximately the same statement.

---

## 7. Mechanical commits and `git blame`

The commits that applied these rules mechanically (reformatting, renames) are listed in
[`.git-blame-ignore-revs`](../.git-blame-ignore-revs), so `git blame` still points at the commit that
made the *decision* rather than the one that moved the text. GitHub honours that file automatically;
locally, enable it once:

```bat
git config blame.ignoreRevsFile .git-blame-ignore-revs
```
