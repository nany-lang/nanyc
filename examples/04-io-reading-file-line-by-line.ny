func main
{
	// for each line, print it to the standard output
	console << (each in std.io.file("/tmp/myfile.txt")) << "\n";

	// the same than above, but with line numbers
	var lineNumber = 0;
	for line in std.io.file("/tmp/myfile.txt") do
		console << (++lineNumber) << ": " << line << "\n";
}
