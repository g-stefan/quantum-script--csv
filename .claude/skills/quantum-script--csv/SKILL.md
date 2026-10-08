---
name: quantum-script--csv
description: >-
  How to use the Quantum Script CSV extension (quantum-script--csv), the
  one-line CSV codec loaded with Script.requireExtension("CSV"):
  CSV.decode(str) (one record -> Array of Strings; "," separator only,
  "..." quoting anywhere in a field, "" inside quotes is ", never fails,
  never trims, unterminated quote runs to end of input, trailing \r / \n
  line terminator ignored so LF and CRLF lines decode the same, line breaks
  before the end are kept as data, "" decodes to [""]) and
  CSV.encode(array) (elements via toString, quoted only when they contain
  space / tab / , / " / \r / \n, " doubled, joined with ",", no line
  terminator added, numbers lose formatting: 5000.00 -> 5000,
  undefined / holes -> "undefined", non-Array argument throws
  Error "invalid parameter"); reading CSV files with multi-line quoted
  values (split "\n", join lines while the " count is odd, skip empty
  lines) and writing them (join records with "\r\n"), header row
  to objects, Convert.toNumber / Convert.numberFormat for numbers;
  fabricare's csvDecode(text) helper; which hosts have it (quantum-script
  dynamic SDK, fabricare build scripts, magnet; NOT the static
  quantum-script); the C++ side (registerInternalExtension, initExecutive,
  quantumScriptExtension entry point, quantum-script--csv.static,
  XYO_QUANTUMSCRIPT_EXTENSION_CSV_LIBRARY). Use when writing or reviewing
  Quantum Script or fabricare code that reads or writes CSV, C++ code that
  includes <XYO/QuantumScript.Extension/CSV.hpp>, a fabricare.json
  depending on "quantum-script--csv", or when working inside the
  quantum-script--csv repository.
---

# quantum-script--csv

CSV extension of Quantum Script (see the `quantum-script` skill for the
language and its differences from JavaScript; its rules apply — no
hoisting, `typeof(x)` needs parentheses, statements end with `;`).
Purpose: **convert one CSV record to an array of strings and back**, so
scripts can read spreadsheet / database / tool exports and write tables
other software opens. Splitting a file into records and joining records is
the script's job.

Full documentation: `docs/` in the quantum-script--csv repository
(`X:\Storage\XYO\Gitea\CPP\quantum-script--csv\docs` on this machine):
README (purpose), getting-started (build, load, hosts, fabricare
`csvDecode`, C++ registration), **script-api** (dialect, exact rules, edge
cases, round trip, recipes), cpp-api, reference. When in doubt read
`source/XYO/QuantumScript.Extension/CSV/Library.cpp` (~140 lines).

## Script API

```javascript
Script.requireExtension("CSV");

CSV.decode("a,\"b,c\",\"say \"\"hi\"\"\"");   // ["a", "b,c", "say \"hi\""]   always Strings, never fails
CSV.encode(["a", "b c", "x,y", 42]);          // a,"b c","x,y",42              one line, no "\r\n" added
```

## Hard rules

1. **One record per call.** `decode` drops every `\r` / `\n` at the end
   of its input (`"a,b\r\n"` → `["a", "b"]`), so lines from LF and CRLF
   files decode the same; `\r` / `\n` before the end are data. Split
   files yourself and skip empty lines (`"a\n".split("\n")` ends with
   `""`, which decodes to `[""]`).
2. **Line breaks in values are quoted by `encode`** (`["a\nb"]` →
   `"a<LF>b"` in quotes) and kept by `decode`. A file with such values has
   records spanning several lines: when reading, join lines while the
   number of `"` in the record so far is odd (see the recipe). Do not
   `replace("\r", "")` the whole text first — it would also change
   `\r\n` inside values.
3. **Decoded fields are always Strings, untrimmed.** `" a "` stays
   `" a "`; `"5000.00"` stays a string — use `Convert.toNumber(f)`. Empty
   fields are `""`, never `undefined` / `null`.
4. **`encode` converts with `toString`.** `5000.00` → `5000` (use
   `Convert.numberFormat(x, 2)` for fixed decimals), `true` → `true`,
   `null` → `null`, `undefined` and array holes → `undefined`, objects →
   `Object`, nested arrays → `"1,2"`. Convert values explicitly when that
   matters.
5. **`encode` needs an Array.** A string, number, `null`, `undefined` or
   object throws `Error`, `e.message == "invalid parameter"`.
6. **Quoting on encode**: only when the text contains space, tab, `,`,
   `"`, `\r` or `\n` (then `"` → `""`). `[""]` and `[]` both give `""` (indistinguishable);
   `["", ""]` gives `,`.
7. **Decode is lenient, not validating.** An unterminated quote runs to
   end of input (`"a,b` → `["a,b"]`); a quote mid-field opens a quoted
   section (`ab"c,d"e` → `["abc,de"]`); outside quotes `a""b` → `["ab"]`.
   There is no error to detect malformed CSV — check `row.length` against
   the expected column count.
8. **Comma only.** No `;` or tab separator, no options. `CSV.decode("a;b")`
   is `["a;b"]`. Unquoted `;` files can be split with `line.split(";")`.
9. **Round trip**: `CSV.decode(CSV.encode(row))` (also with `+ "\r\n"`)
   equals `row` for string arrays, line breaks included (except `[""]` /
   `[]`).
   `CSV.encode(CSV.decode(line))` normalizes quoting (`"abc"` → `abc`).
10. **Hosts**: dynamic `quantum-script` (loads `quantum-script--csv.dll`),
    **fabricare build scripts** (internal, already required) and **magnet**
    (internal) have it. The **static** `quantum-script`
    (`win64-msvc-2026.static`) does not: `Unable to open "CSV"`.
11. **fabricare `csvDecode(text)`**: global helper, `null` for nil, splits
    on `"\n"` and decodes each line; `\r` of CRLF lines is ignored by
    `decode` (fabricare built with an older extension kept it), a trailing
    newline adds a `[""]` row, and multi-line quoted values are split
    across rows. Filter empty rows; use the record reader recipe when
    values can contain line breaks.
12. One engine per thread: each thread requires `CSV` itself; the
    functions are stateless.

## Recipes

```javascript
Script.requireExtension("Shell");
Script.requireExtension("CSV");

function csvDecodeText(text) {                       // records may span lines
	var table = [];
	var record = null;
	for (var line of text.split("\n")) {
		if (Script.isNil(record)) {
			record = line;
		} else {
			record += "\n" + line;
		};
		if ((record.split("\"").length - 1) % 2) {      // quote still open
			continue;
		};
		if (record != "" && record != "\r") {            // skip empty lines
			table.push(CSV.decode(record));          // trailing \r ignored
		};
		record = null;
	};
	if (!Script.isNil(record)) {
		table.push(CSV.decode(record));
	};
	return table;
};

function csvReadFile(fileName) {                     // array of rows, null if missing
	var text = Shell.fileGetContents(fileName);
	if (Script.isNil(text)) {
		return null;
	};
	return csvDecodeText(text);
};

function csvWriteFile(fileName, table) {             // CRLF, as spreadsheets expect
	var text = "";
	for (var row of table) {
		text += CSV.encode(row) + "\r\n";
	};
	return Shell.filePutContents(fileName, text);
};

var table = csvReadFile("people.csv");               // header row to objects
var people = [];
for (var i = 1; i < table.length; ++i) {
	var item = {};
	for (var k = 0; k < table[0].length; ++k) {
		item[table[0][k]] = table[i][k];
	};
	people.push(item);
};

var price = Convert.toNumber(CSV.decode("apple,2.50")[1]);   // 2.5
CSV.encode(["apple", Convert.numberFormat(price, 2)]);       // apple,2.50
```

## C++

```cpp
#include <XYO/QuantumScript.Extension/Console.hpp>
#include <XYO/QuantumScript.Extension/CSV.hpp>
using namespace XYO::QuantumScript;

void initExecutive(Executive *executive) {                 // host init callback
	Extension::Console::registerInternalExtension(executive);
	Extension::CSV::registerInternalExtension(executive);     // scripts still requireExtension("CSV")
};
```

- fabricare.json dependency: `"quantum-script--csv"` (`dll-or-lib`: DLL on
  dynamic platforms, static lib on `*.static` platforms); it pulls in
  `quantum-script` and `quantum-script--console`. Statically linked tools
  on a dynamic platform (fabricare) use `"quantum-script--csv.static"`,
  which defines `XYO_QUANTUMSCRIPT_EXTENSION_CSV_LIBRARY` for dependents.
- DLL entry point `extern "C" quantumScriptExtension(Executive *, void *)`
  only with `XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY` and without
  `XYO_QUANTUMSCRIPT_EXTENSION_CSV_LIBRARY`. Static hosts must register the
  extension as internal.
- No separate C++ CSV API: the parser lives in the static `decode` /
  `encode` functions of `CSV/Library.cpp`.

## Working in this repository

- Build: `fabricare make`, `fabricare test` (runs `test/test.01`, which
  registers Console and CSV as internal; `test/test.01.js` checks the
  round trip, terminator handling and line-break quoting, and throws on
  failure; run `make` first), `fabricare install` (see the `fabricare` skill).
  `quantum-script` and `quantum-script--console` must be installed first.
  With the SDK installed a script can be run directly:
  `quantum-script test/test.01.js` (uses the installed DLL).
- Native functions are
  `static TPointer<Variable> name(VariableFunction *, Variable *this_, VariableArray *arguments)`
  in `CSV/Library.cpp`, registered in `initExecutive` with
  `executive->setFunction2("CSV.name(args)", name)`.
- Changing decode / encode rules changes fabricare (`csvDecode`, Gitea
  release scripts) and magnet scripts; update `README.md`,
  `docs/script-api.md`, `docs/reference.md`, `test/test.01.js` and this
  skill together.
- Code style: tabs (width 8), `.clang-format`, CRLF, statements and blocks
  end with `};`, camelCase. SPDX header: MIT for `source/` and `docs/`,
  Unlicense for `test/` and `.claude/` (see `.reuse/dep5`).
