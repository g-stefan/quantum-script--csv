// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

Script.requireExtension("Console");
Script.requireExtension("CSV");

var input = "1999,Chevy,\"Venture \"\"Extended Edition, Very Large\"\"\",,5000.00";
var result = CSV.encode(CSV.decode(input));
if(input == result) {
	Console.writeLn("-> test 01 ok");
} else {
	Console.writeLn("-> test 01 fail");
};

function check(name, value, expected) {
	if (value != expected) {
		Console.writeLn("-> " + name + " fail");
		throw "test fail: " + name;
	};
	Console.writeLn("-> " + name + " ok");
};

// line terminator is not part of the last field
check("test 02", CSV.decode("a,b\r\n")[1], "b");
check("test 03", CSV.decode("a,b\n")[1], "b");
check("test 04", CSV.decode("a,b\r")[1], "b");
check("test 05", CSV.decode("a,\r\n")[1], "");
check("test 06", CSV.decode("\r\n").length, 1);

// line breaks inside values are quoted and kept
check("test 07", CSV.encode(["a\nb", "c"]), "\"a\nb\",c");
check("test 08", CSV.encode(["a\r\nb"]), "\"a\r\nb\"");
check("test 09", CSV.decode(CSV.encode(["x", "line1\r\nline2\n"]))[1], "line1\r\nline2\n");
check("test 10", CSV.decode(CSV.encode(["x", "line1\r\nline2\n"]) + "\r\n")[1], "line1\r\nline2\n");
