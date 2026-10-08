# Script API

Everything the extension defines after `Script.requireExtension("CSV")`.
`CSV` is a plain object holding two native functions; it has no
constructor, no options and no state.

A Quantum Script string is a sequence of bytes (UTF-8 by convention). The
only special characters are the ASCII `,`, `"`, and `\r` / `\n` at the end
of a line, so UTF-8 text passes through both functions unchanged.

## The dialect

Both functions work on **one record**:

| Rule | `CSV.decode` | `CSV.encode` |
|------|--------------|--------------|
| Field separator | `,` only (no `;`, no tab) | `,` |
| Quoting | `"..."` anywhere in a field opens / closes a quoted section | a field is quoted when it contains a space, a tab, `,`, `"`, `\r` or `\n` |
| Quote inside quotes | `""` → `"` | `"` → `""` |
| Spaces | kept as they are, never trimmed | kept, and cause quoting |
| Line terminator | trailing `\r` / `\n` at the end of the input are ignored | none added |
| Line breaks in values | kept (inside quotes, or anywhere before the end) | kept, the field is quoted |
| Field types | every field is a `String` | every value is converted to text |
| Header row | not special | not special |

This is RFC 4180 for one record. A record whose quoted values contain line
breaks spans several lines of a file: read such files with the
[record reader recipe](#recipes), not with a plain `split("\n")`. Not
supported: other separators, trimming, type detection.

## `CSV.decode(str)`

Splits one CSV record into its fields and returns them as an **Array of
Strings**. It never fails and never returns `undefined`: there is always at
least one field.

```javascript
CSV.decode("a,b,c");                    // ["a", "b", "c"]
CSV.decode("a,b,c\r\n");                // ["a", "b", "c"]   line terminator ignored
CSV.decode("");                         // [""]          one empty field
CSV.decode("\r\n");                     // [""]
CSV.decode(",");                        // ["", ""]
CSV.decode("a,,c,");                    // ["a", "", "c", ""]
CSV.decode(" a , b ");                  // [" a ", " b "]  not trimmed
CSV.decode("\"a,b\",c");                // ["a,b", "c"]
CSV.decode("\"He said \"\"hi\"\"\",x"); // ["He said \"hi\"", "x"]
CSV.decode("\"\"\"\"");                 // ["\""]
CSV.decode("1,\"line1\r\nline2\"\r\n"); // ["1", "line1\r\nline2"]
CSV.decode("1999,Chevy,\"Venture \"\"Extended Edition, Very Large\"\"\",,5000.00");
	// ["1999", "Chevy", "Venture \"Extended Edition, Very Large\"", "", "5000.00"]
```

How the record is read:

- Every `\r` and `\n` at the **end** of the input is dropped first (`\n`,
  `\r\n`, `\r`, or several of them), so lines read from LF or CRLF files
  decode the same way.
- Then character by character: `,` outside quotes ends the current field
  and starts a new one.
- `"` opens a quoted section — **anywhere in the field**, not only at its
  start. Inside it, `,` is ordinary, `""` gives one `"`, and a single `"`
  closes the section. The quote characters are not part of the value.
- Any other character (including spaces, tabs, and `\r` / `\n` before the
  end) is copied.
- At the end the current field is added, even if empty.

Lenient cases that follow from these rules:

| Input | Result | Why |
|-------|--------|-----|
| `"unterminated,x` | `["unterminated,x"]` | an open quote runs to the end of the record |
| `ab"c,d"e,f` | `["abc,de", "f"]` | a quote in the middle of a field opens a quoted section |
| `"x" y,z` | `["x y", "z"]` | text after the closing quote is appended |
| `a""b` | `["ab"]` | outside quotes, `""` is an empty quoted section, not a `"` |
| `a\nb,c` | `["a\nb", "c"]` | a line break before the end is data |
| `"x\r\n"\r\n` | `["x\r\n"]` | only the terminator after the closing quote is dropped |
| `a;b;c` | `["a;b;c"]` | `;` is not a separator |

Argument conversion: the argument is converted with `toString` first, so
`CSV.decode(123)` is `["123"]` and `CSV.decode()` is `["undefined"]`. Pass a
string.

## `CSV.encode(value)`

Joins the elements of an **Array** into one CSV record and returns it as a
`String`. Each element is converted to text with `toString`, then:

- if the text contains a space, a tab, `,`, `"`, `\r` or `\n`, every `"` is
  doubled and the text is put between `"`;
- otherwise it is written as it is (an empty string is written as nothing).

Elements are separated by `,`; no line terminator is added at the end.

```javascript
CSV.encode(["a", "b", "c"]);            // a,b,c
CSV.encode([]);                         // ""   (empty record)
CSV.encode([""]);                       // ""   (same as [] — cannot be told apart)
CSV.encode(["", ""]);                   // ,
CSV.encode(["a b"]);                    // "a b"
CSV.encode(["a,b"]);                    // "a,b"
CSV.encode(["a\tb"]);                   // "a<tab>b" between quotes
CSV.encode(["say \"hi\""]);             // "say ""hi"""
CSV.encode(["line1\nline2", "x"]);      // "line1<LF>line2",x
```

Element conversion:

| Element | Written as |
|---------|------------|
| String | itself (quoted if needed) |
| number | decimal text: `5000.00` → `5000`, `2.5` → `2.5`, `-3` → `-3` |
| `true` / `false` / `null` | `true` / `false` / `null` |
| `undefined`, array holes | `undefined` (`a = []; a[2] = "x";` → `undefined,undefined,x`) |
| nested array | elements joined with `,`, then quoted: `[[1, 2]]` → `"1,2"` |
| object | `Object` |

The argument itself **must be an Array**: a string, number, `null`,
`undefined` or object throws an `Error` with message `invalid parameter`:

```javascript
try {
	CSV.encode("a,b");
} catch (e) {
	Console.writeLn(e.message);          // invalid parameter
};
```

## Round trip

For an array of strings, `CSV.decode(CSV.encode(row))` gives `row` back,
line breaks inside values included, and so does
`CSV.decode(CSV.encode(row) + "\r\n")`:

```javascript
var row = ["a b", "c\"d", "e,f", "", ",", "line1\r\nline2"];
CSV.decode(CSV.encode(row));            // same as row
```

Exceptions:

- `CSV.encode([""])` and `CSV.encode([])` are both `""`, which decodes to
  `[""]`.
- Non-string elements come back as strings (`[42]` → `["42"]`).

`CSV.encode(CSV.decode(line))` normalizes quoting: unnecessary quotes are
removed and needed ones added. `"abc","d e"` becomes `abc,"d e"`. For
canonical input it gives back the same line (this is what
`test/test.01.js` checks).

## Recipes

Read a CSV file into an array of rows. Lines are joined while a quoted value
is open (odd number of `"` so far), so values with line breaks are read
back whole; empty lines are skipped; LF and CRLF files both work:

```javascript
Script.requireExtension("Shell");
Script.requireExtension("CSV");

function csvDecodeText(text) {
	var table = [];
	var record = null;
	for (var line of text.split("\n")) {
		if (Script.isNil(record)) {
			record = line;
		} else {
			record += "\n" + line;
		};
		if ((record.split("\"").length - 1) % 2) {
			continue;
		};
		if (record != "" && record != "\r") {
			table.push(CSV.decode(record));
		};
		record = null;
	};
	if (!Script.isNil(record)) {
		table.push(CSV.decode(record));
	};
	return table;
};

function csvReadFile(fileName) {
	var text = Shell.fileGetContents(fileName);
	if (Script.isNil(text)) {
		return null;
	};
	return csvDecodeText(text);
};
```

Do not remove all `\r` from the text before splitting: that would also turn
`\r\n` inside quoted values into `\n`. `CSV.decode` already ignores the
`\r` at the end of each record. If the data never contains quoted line
breaks, a plain loop over `text.split("\n")` with `CSV.decode` per
non-empty line is enough.

Write an array of rows to a CSV file (CRLF, as spreadsheets expect):

```javascript
function csvWriteFile(fileName, table) {
	var text = "";
	for (var row of table) {
		text += CSV.encode(row) + "\r\n";
	};
	return Shell.filePutContents(fileName, text);
};
```

Turn rows with a header into objects:

```javascript
var table = csvReadFile("people.csv");
var header = table[0];
var people = [];
for (var i = 1; i < table.length; ++i) {
	var item = {};
	for (var k = 0; k < header.length; ++k) {
		item[header[k]] = table[i][k];
	};
	people.push(item);
};
```

Numbers: decoded fields are strings, and encoded numbers lose their
formatting:

```javascript
var total = Convert.toNumber(CSV.decode("1999,5000.00")[1]) + 1;   // 5001
CSV.encode([Convert.numberFormat(5000, 2)]);                       // 5000.00
```

Parse the CSV output of a command (fabricare has a simpler version of this
as `csvDecode`):

```javascript
Script.requireExtension("ProcessInteractive");
var rows = csvDecodeText(ProcessInteractive.run("tool --output csv"));
```

Functions must be defined before they are called: Quantum Script has no
hoisting.

Semicolon or tab separated files are not supported by `decode`: a file
that never quotes its values can be split with
`line.split(";")`; otherwise convert it to commas with another tool first.
