# Getting started

## 1. Build and install

The extension is built with [fabricare](https://github.com/g-stefan/fabricare),
the build tool used by all XYO C++ projects. `quantum-script` (and everything
below it: `xyo-system`, `xyo-encoding`, ...) and `quantum-script--console`
must be installed to the SDK first. From the repository root:

```bash
fabricare make       # build into output/
fabricare test       # build and run test/test.01 (run make first)
fabricare install    # copy output/{bin,include,lib} to ~/.fabricare/<platform>
fabricare clean      # remove output/ and temp/
```

`fabricare.json` declares three projects:

| Project | Kind | Purpose |
|---------|------|---------|
| `quantum-script--csv` | `dll-or-lib`: shared library in a dynamic build, static library in a static build | the extension |
| `quantum-script--csv.static` | static library with static CRT, category `make` | for statically linked tools built on a dynamic platform (fabricare); dependents get `XYO_QUANTUMSCRIPT_EXTENSION_CSV_LIBRARY` defined |
| `test.01` | executable, category `test` | runs `test/test.01.js` with the extension registered as internal |

After `fabricare install`, `quantum-script--csv.dll` (Windows) /
`libquantum-script--csv.so` (Linux) sits in the SDK `bin` folder next to
`quantum-script.exe`, which is where `Script.requireExtension("CSV")` finds
it.

## 2. Use it from a script

```javascript
Script.requireExtension("Console");
Script.requireExtension("CSV");

var fields = CSV.decode("1999,Chevy,\"Venture \"\"Extended Edition\"\"\",,5000.00");
Console.writeLn(fields.length);           // 5
Console.writeLn(fields[2]);               // Venture "Extended Edition"
Console.writeLn(fields[3] == "");         // true

var line = CSV.encode(["Ana", "New York", "a, b", 42]);
Console.writeLn(line);                    // Ana,"New York","a, b",42
```

Run it with:

```bash
quantum-script hello-csv.js
```

`Script.requireExtension("CSV")` looks for an external `quantum-script--csv`
library first (the file as named, then every include path folder: next to
the interpreter, next to the script), then for an internal extension
registered by the host. Loading twice does nothing. A missing extension
throws `Unable to open "CSV"`.

## 3. Which hosts have it

| Host | `CSV` available | How |
|------|-----------------|-----|
| `quantum-script` (dynamic SDK, `win64-msvc-2026/bin`) | yes | loads `quantum-script--csv.dll` |
| `quantum-script` (static SDK, `win64-msvc-2026.static/bin`) | **no** | a static executable cannot load extension DLLs: `Unable to open "CSV"` |
| `fabricare` build scripts | yes | registered as internal (`quantum-script--csv.static`) and required by fabricare at start-up |
| `magnet` scripts | yes | registered as internal by `quantum-script--magnet` |
| your own C++ host | if you register it | see section 4 |

### fabricare

In fabricare build scripts `CSV` is already loaded. fabricare also defines
a helper on top of it, `csvDecode(text)`, that decodes a whole text:

```javascript
// fabricare build script
var releases = csvDecode(ProcessInteractive.run("tea releases list --output csv --repo " + repo));
for (var row of releases) {
	// row is an Array of Strings
};
```

`csvDecode` returns `null` for `null` / `undefined`, splits the text on
`"\n"` and calls `CSV.decode` on every line. `CSV.decode` ignores the
`\r` of CRLF lines (fabricare builds that include an older extension left
it in the last field). A trailing newline gives a last row `[""]`, and a
quoted value containing a line break is split across rows; skip empty rows,
or use the record reader from [Script API, recipes](script-api.md#recipes)
when values can contain line breaks.

## 4. Register it in a C++ host

A host that embeds Quantum Script makes `CSV` available as an internal
extension by registering it in the init callback (this is what
`test/test.01.cpp` does):

```cpp
#include <XYO/QuantumScript.hpp>
#include <XYO/QuantumScript.Extension/Console.hpp>
#include <XYO/QuantumScript.Extension/CSV.hpp>

using namespace XYO::QuantumScript;

void initExecutive(Executive *executive) {
	Extension::Console::registerInternalExtension(executive);
	Extension::CSV::registerInternalExtension(executive);
};

int main(int cmdN, char *cmdS[]) {
	if (ExecutiveX::initExecutive(cmdN, cmdS, initExecutive)) {
		if (!ExecutiveX::executeString(
		        "Script.requireExtension(\"Console\");"
		        "Script.requireExtension(\"CSV\");"
		        "Console.writeLn(CSV.encode([\"a\", \"b c\"]));")) {
			printf("%s\n", (ExecutiveX::getError()).value());
			printf("%s", (ExecutiveX::getStackTrace()).value());
		};
		ExecutiveX::endProcessing();
	};
	return 0;
};
```

Registering only makes the extension *available*: scripts still call
`Script.requireExtension("CSV")`. With the DLL build of the engine an
external `quantum-script--csv.dll` found on the include path wins over the
internal one for `requireExtension`; use
`Script.requireInternalExtension("CSV")` to force the internal one.

In the host's `fabricare.json`:

```json
{
	"name": "my-host",
	"make": "exe",
	"sourcePath": "XYO/MyHost",
	"dependency": [
		"quantum-script--csv"
	]
}
```

`quantum-script--csv` depends on `quantum-script` and
`quantum-script--console`; fabricare resolves them transitively. A
statically linked tool built on a dynamic platform depends on
`quantum-script--csv.static` instead (as fabricare itself does).

## 5. Static builds

The project is `dll-or-lib`: on a static platform (for example
`win64-msvc-2026.static`, which sets `XYO_PLATFORM_COMPILE_STATIC`) the same
project builds a static library. The export macros become empty and the
`quantumScriptExtension` DLL entry point is left out (it is compiled only
with `XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY`). Defining
`XYO_QUANTUMSCRIPT_EXTENSION_CSV_LIBRARY` has the same effect when the
sources are compiled into another library; the `quantum-script--csv.static`
project passes this define to its dependents.

A static host must register the extension with `registerInternalExtension`
(section 4): external DLLs cannot be loaded into a host that does not use
the engine DLL.

## 6. Threads

Each thread that runs scripts has its own engine, so every thread loads the
extension itself with `Script.requireExtension("CSV")`. The functions keep
no state: they are safe to call from any thread that loaded the extension.
