// Public domain
// http://unlicense.org/
Script.requireExtension("Console");
Script.requireExtension("CSV");

var input = "1999,Chevy,\"Venture \"\"Extended Edition, Very Large\"\"\",,5000.00";
var result = CSV.encode(CSV.decode(input));
if(input == result) {
	Console.writeLn("-> test 0001 ok");
} else {
	Console.writeLn("-> test 0001 fail");
};

