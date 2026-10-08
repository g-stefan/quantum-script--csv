# API reference

## Script

Available after `Script.requireExtension("CSV")`.

| Symbol | Returns | Notes |
|--------|---------|-------|
| `CSV.decode(str)` | Array of Strings | one CSV record; trailing `\r` / `\n` ignored; `,` separates, `"..."` quotes (anywhere in a field), `""` inside quotes is `"`; never fails, at least one field |
| `CSV.encode(value)` | String | `value` must be an Array; elements via `toString`, quoted when they contain space, tab, `,`, `"`, `\r` or `\n` (`"` doubled); joined with `,`, no line terminator |

### Edge cases

| Expression | Result |
|------------|--------|
| `CSV.decode("")` | `[""]` |
| `CSV.decode(",")` | `["", ""]` |
| `CSV.decode("a,,c,")` | `["a", "", "c", ""]` |
| `CSV.decode(" a , b ")` | `[" a ", " b "]` (not trimmed) |
| `CSV.decode("\"a,b\",c")` | `["a,b", "c"]` |
| `CSV.decode("\"\"\"\"")` | `["\""]` |
| `CSV.decode("\"unterminated,x")` | `["unterminated,x"]` |
| `CSV.decode("ab\"c,d\"e,f")` | `["abc,de", "f"]` |
| `CSV.decode("a\"\"b")` | `["ab"]` |
| `CSV.decode("a,b\r\n")`, `CSV.decode("a,b\n")`, `CSV.decode("a,b\r")` | `["a", "b"]` |
| `CSV.decode("\r\n")` | `[""]` |
| `CSV.decode("\"x\r\n\"\r\n")` | `["x\r\n"]` |
| `CSV.decode("a\nb")` | `["a\nb"]` (line break before the end is data) |
| `CSV.decode("a;b")` | `["a;b"]` |
| `CSV.decode(123)` | `["123"]` |
| `CSV.decode()` | `["undefined"]` |
| `CSV.encode([])` | `""` |
| `CSV.encode([""])` | `""` |
| `CSV.encode(["", ""])` | `","` |
| `CSV.encode(["a b"])` | `"\"a b\""` |
| `CSV.encode(["say \"hi\""])` | `"\"say \"\"hi\"\"\""` |
| `CSV.encode(["x\ny"])` | `"\"x\ny\""` (line break quoted) |
| `CSV.encode([1, 2.5, true, null])` | `"1,2.5,true,null"` |
| `CSV.encode([5000.00])` | `"5000"` |
| `CSV.encode([undefined])`, holes | `"undefined"` |
| `CSV.encode([[1, 2]])` | `"\"1,2\""` |
| `CSV.encode([{a: 1}])` | `"Object"` |

### Errors

| Message | Cause |
|---------|-------|
| `invalid parameter` | `CSV.encode` called with something that is not an Array (string, number, `null`, `undefined`, object) |
| `Unable to open "CSV"` | the extension library was not found and no internal one is registered (for example in the static `quantum-script`) |

`CSV.decode` never throws.

## Hosts

| Host | `CSV` |
|------|-------|
| `quantum-script`, dynamic SDK | yes (DLL) |
| `quantum-script`, static SDK | no |
| fabricare build scripts | yes, preloaded; plus `csvDecode(text)` (split on `\n`, decode each line, `null` for nil; no multi-line records) |
| magnet | yes, internal |

## C++

Namespace `XYO::QuantumScript::Extension::CSV`, umbrella header
`<XYO/QuantumScript.Extension/CSV.hpp>`.

### Library (`CSV/Library.hpp`)

| Symbol | Notes |
|--------|-------|
| `void registerInternalExtension(Executive *executive)` | register `"CSV"` as an internal extension |
| `void initExecutive(Executive *executive, void *extensionId)` | extension init, run by the engine |
| `extern "C" void quantumScriptExtension(Executive *, void *)` | DLL entry point (not in static builds) |

### Metadata

| Symbol | Notes |
|--------|-------|
| `Version::version()`, `Version::build()`, `Version::versionWithBuild()`, `Version::datetime()` | from `version.json` |
| `Copyright::copyright()`, `Copyright::publisher()`, `Copyright::company()`, `Copyright::contact()` | |
| `License::license()`, `License::shortLicense()` | MIT text |

`Version`, `Copyright` and `License` exist in every XYO library: qualify them
(`Extension::CSV::Version::versionWithBuild()`).

### Build configuration

| Name | Meaning |
|------|---------|
| `quantum-script--csv` | fabricare project, `dll-or-lib` |
| `quantum-script--csv.static` | fabricare project, static library with static CRT, for static tools on a dynamic platform |
| `test.01` | fabricare test project, runs `test/test.01.js` |
| `XYO_QUANTUMSCRIPT_EXTENSION_CSV_EXPORT` | export / import macro |
| `XYO_QUANTUMSCRIPT_EXTENSION_CSV_INTERNAL` | defined while building the DLL (from `QUANTUM_SCRIPT__CSV_INTERNAL`) |
| `XYO_QUANTUMSCRIPT_EXTENSION_CSV_LIBRARY` | static build: empty export macro, no DLL entry point |
| `XYO_QUANTUMSCRIPT_DEBUG_RUNTIME` | trace lines `- csv-decode` / `- csv-encode` |
