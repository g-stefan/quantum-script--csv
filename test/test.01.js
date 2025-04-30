// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
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

