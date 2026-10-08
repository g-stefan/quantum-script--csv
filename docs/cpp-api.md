# C++ API

For hosts that embed Quantum Script and for maintainers of the extension.
Read the `quantum-script` repository's `docs/embedding.md` and
`docs/writing-extensions.md` first: native functions, `Variable` and
`TPointer` work the same way here.

## Headers and namespace

```cpp
#include <XYO/QuantumScript.Extension/CSV.hpp>   // Library.hpp

using namespace XYO::QuantumScript;
```

Namespace: `XYO::QuantumScript::Extension::CSV`. Export macro:
`XYO_QUANTUMSCRIPT_EXTENSION_CSV_EXPORT`:

| Define | Effect |
|--------|--------|
| `XYO_QUANTUMSCRIPT_EXTENSION_CSV_INTERNAL` (or `QUANTUM_SCRIPT__CSV_INTERNAL`, set by fabricare while building the DLL) | export macro = `XYO_PLATFORM_LIBRARY_EXPORT` |
| none | export macro = `XYO_PLATFORM_LIBRARY_IMPORT` (consumers of the DLL) |
| `XYO_QUANTUMSCRIPT_EXTENSION_CSV_LIBRARY` (set for dependents of `quantum-script--csv.static`) | export macro empty, no `quantumScriptExtension` entry point (sources compiled into another library) |
| `XYO_PLATFORM_COMPILE_STATIC` (static platforms) | `XYO_PLATFORM_LIBRARY_EXPORT` / `IMPORT` are empty |

The `quantumScriptExtension` entry point is compiled only when
`XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY` is defined and
`XYO_QUANTUMSCRIPT_EXTENSION_CSV_LIBRARY` is not.

## Registering the extension

```cpp
void Extension::CSV::registerInternalExtension(Executive *executive);
void Extension::CSV::initExecutive(Executive *executive, void *extensionId);
```

- `registerInternalExtension` registers `"CSV"` as an internal extension;
  call it from the host's init callback (see
  [Getting started](getting-started.md#4-register-it-in-a-c-host)).
- `initExecutive` is the extension's init function, run by the engine when a
  script first requires `CSV` in a thread. It sets the extension name, info
  (`"CSV"` plus the short license text), version and public flag, then:

  ```cpp
  executive->compileStringX("var CSV={};");
  executive->setFunction2("CSV.decode(str)", decode);
  executive->setFunction2("CSV.encode(value)", encode);
  ```

  Do not call it directly.
- The DLL build also exports
  `extern "C" void quantumScriptExtension(Executive *, void *)`, which
  forwards to `initExecutive`; it is what `Script.requireExtension` looks up
  in `quantum-script--csv.dll`.

## The native functions

Both are `static` in `CSV/Library.cpp`, with the usual signature
`TPointer<Variable> name(VariableFunction *function, Variable *this_, VariableArray *arguments)`.

| Script function | Implementation | Returns |
|-----------------|----------------|---------|
| `CSV.decode(str)` | `arguments->index(0)->toString()`; trailing `\r` / `\n` are excluded (`end` moved back); then one pass over the bytes up to `end`: `,` ends a field, `"` opens a quoted section in which `""` is `"` and `"` closes it, other bytes are appended; the last field is always added | `VariableArray` of `VariableString` |
| `CSV.encode(value)` | `TIsType<VariableArray>` check (else `throw Error("invalid parameter")`); for each index `getPropertyByIndex(k)->toString()`; quote when `indexOf` finds `" "`, `"\t"`, `","`, `"\r"`, `"\n"` or `"\""`, doubling `"` with `replace("\"", "\"\"")`; join with `,` | `VariableString` |

With `XYO_QUANTUMSCRIPT_DEBUG_RUNTIME` defined each function prints a trace
line (`- csv-decode`, `- csv-encode`).

There is no C++ CSV API outside the script functions: C++ code that needs
CSV parsing either runs a script or implements the same loop (it is about
40 lines, see `Library.cpp`).

## Notes for maintainers

- New functions: register them in `initExecutive` with
  `executive->setFunction2("CSV.name(args)", name)`, then update
  `README.md`, `docs/script-api.md`, `docs/reference.md`, the skill in
  `.claude/skills/quantum-script--csv/` and `test/test.01.js`.
- Behavior changes (for example trimming, another separator) change the
  output of existing scripts:
  fabricare's `csvDecode` (used by the Gitea release scripts) and magnet
  scripts depend on the current rules. Document them in
  `docs/script-api.md` and the skill.
- `test/test.01.js` checks that `CSV.encode(CSV.decode(line)) == line` for
  a record with an empty field and a quoted field containing `,` and `""`
  (test 01), that `\n`, `\r\n` and `\r` terminators are ignored by
  `decode` (tests 02-06), and that values with line breaks are quoted and
  survive a round trip (tests 07-10). A failed check throws, so
  `fabricare test` fails.
- The `quantum-script--csv.static` project must keep the same sources and
  version as `quantum-script--csv` (`linkVersion`); fabricare links it.
