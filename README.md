# Quantum Script Extension CSV

Quantum Script extension
- Decoding one CSV line (record) into an array of strings:
`CSV.decode` splits on `,`, handles `"..."` quoted fields and `""` escapes,
ignores the trailing `\r` / `\n`.
- Encoding an array of values into one CSV line:
`CSV.encode` quotes fields that contain a space, a tab, `,`, `"` or a line break.
- For reading and writing tables exchanged with spreadsheets, databases and
command line tools, one line at a time.

```javascript
Script.requireExtension("CSV");

CSV;
CSV.decode(str);
CSV.encode(value);
```

Built on `quantum-script`, part of the XYO C++ SDK; also available in
fabricare build scripts and magnet.

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - build, load from a script, hosts, register in a C++ host
- [Script API](docs/script-api.md) - the CSV dialect, every function, edge cases, recipes
- [C++ API](docs/cpp-api.md) - registration, DLL entry point, build defines
- [API reference](docs/reference.md)

A Claude Code skill for this extension is in
[.claude/skills/quantum-script--csv](.claude/skills/quantum-script--csv/SKILL.md).

## License

Copyright (c) 2016-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.
