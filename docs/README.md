# Quantum Script Extension CSV — Documentation

`quantum-script--csv` is the **CSV line codec extension of Quantum Script**.
Loaded with `Script.requireExtension("CSV")`, it adds a `CSV` object to
scripts with two functions:

```javascript
CSV.decode(str);      // one CSV line   -> Array of Strings
CSV.encode(value);    // Array of values -> one CSV line (String)
```

CSV (comma-separated values) is the plain text table format that
spreadsheets, databases and command line tools exchange: one record per
line, fields separated by `,`, a field that contains `,`, `"` or a line
break written between double quotes with `"` doubled inside. This
extension converts **one record** at a time; splitting a file into
records and joining records into a file is left to the script.

- **Decode one record.** `CSV.decode('1999,Chevy,"Venture ""Extended"""')`
  returns `["1999", "Chevy", "Venture \"Extended\""]`. Separators inside
  quotes are kept, `""` inside quotes becomes `"`, the quotes themselves
  are removed. Every field is a `String`; nothing is trimmed or converted.
- **Encode one record.** `CSV.encode(["a b", "x,y", "say \"hi\""])` returns
  `"a b","x,y","say ""hi"""`. Fields are converted to text and quoted only
  when they contain a space, a tab, `,`, `"` or a line break.
- **Lenient decoding.** `decode` never fails: an unterminated quote runs to
  the end of the line, a quote in the middle of a field opens a quoted
  section, any other character is copied as it is.
- **Line terminators and line breaks.** `decode` ignores the `\r` / `\n`
  at the end of its input, so LF and CRLF lines give the same fields;
  `encode` quotes values containing `\r` or `\n`, and `decode` keeps them.
- **Comma only.** The separator is always `,`; there is no header
  handling and no type detection.

```
scripts: quantum-script .js, fabricare build scripts, magnet, embedding hosts, ...
quantum-script--csv       <-- this extension: CSV.decode / CSV.encode
quantum-script            (Executive, Variable, Context)
quantum-script--console   (link dependency)
xyo-system, xyo-encoding, xyo-multithreading, xyo-data-structures, xyo-managed-memory, xyo-platform
```

## Why it exists

| Need | What `CSV` gives |
|------|------------------|
| Read tables exported by spreadsheets, databases, reports | `CSV.decode(line)` per line, quoted fields and `""` handled |
| Read the CSV output of command line tools (`tea releases list --output csv`, ...) | fabricare's `csvDecode(text)` is built on `CSV.decode` |
| Write tables other software can open | `CSV.encode(row)` per line, quotes only where needed |
| Keep commas, quotes and spaces inside values | quoting and `""` escaping are done for you |
| Small and dependency free | about 100 lines of C++, no parser library, one engine per thread |

## Concepts at a glance

| Need | Use | Notes |
|------|-----|-------|
| Load the extension | `Script.requireExtension("CSV");` | |
| Split one line into fields | `CSV.decode("a,\"b,c\",d")` | `["a", "b,c", "d"]` |
| Join fields into one line | `CSV.encode(["a", "b,c", "d"])` | `a,"b,c",d` |
| Read a file | split on `"\n"`, join lines while a quote is open, `CSV.decode` each record | see [Script API, recipes](script-api.md#recipes) |
| Write a file | `CSV.encode(row) + "\r\n"` for each row | |
| Numbers | `Convert.toNumber(field)` after decode | decode returns only strings |
| Fixed decimals | `CSV.encode([Convert.numberFormat(5000, 2)])` | `encode([5000.00])` writes `5000` |
| Header row to objects | use row 0 as keys | see recipes |
| Invalid `encode` argument | must be an Array | anything else throws `Error: invalid parameter` |

## Contents

| Document | What it covers |
|----------|----------------|
| [Getting started](getting-started.md) | Build and install, load the extension from a script, which hosts have it (quantum-script, fabricare, magnet), register it in a C++ host |
| [Script API](script-api.md) | The CSV dialect, every function, argument conversion, edge cases, limitations, recipes |
| [C++ API](cpp-api.md) | `registerInternalExtension`, `initExecutive`, the DLL entry point, build defines, how the two functions work |
| [API reference](reference.md) | Every script and C++ symbol on one page |

Quantum Script itself (the language, `Script.requireExtension`, embedding,
writing extensions) is documented in the `quantum-script` repository,
`docs/`.

## Source map

```
source/XYO/QuantumScript.Extension/CSV.hpp            umbrella header, include this from C++
source/XYO/QuantumScript.Extension/CSV.Amalgam.cpp    the whole extension in one translation unit
source/XYO/QuantumScript.Extension/CSV/
    Dependency.hpp                                    <XYO/QuantumScript.hpp>, export macro
    Library[.hpp/.cpp]                                initExecutive, registerInternalExtension,
                                                      decode / encode, DLL entry point
    Copyright / License / Version                     library metadata
    Library.rc, *.rh                                  Windows version resource
test/test.01.cpp                                      C++ host registering Console and CSV as internal
test/test.01.js                                       round trip of a quoted record
```

## AI assistant skill

A Claude Code skill describing how to use this extension lives in
[`.claude/skills/quantum-script--csv/`](../.claude/skills/quantum-script--csv/SKILL.md).
It is picked up automatically inside this repository; copy the folder to
`~/.claude/skills/` to have it available in the projects that use `CSV`
(Quantum Script tools, fabricare scripts, magnet scripts).
