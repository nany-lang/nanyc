func main {
	// for each line, print it to the standard output
	for line in std.io.file.open(ro: "/tmp/myfile.txt") do
		console << line << '\n';
	// the same than above, but with line numbers
	var lineNumber = 0u;
	for line in std.io.file.open(ro: "/tmp/myfile.txt") do
		console << (++lineNumber) << ": " << line << "\n";
}
