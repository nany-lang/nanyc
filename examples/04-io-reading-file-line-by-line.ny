public func main
{
	// print to the console the content of the file line-by-line
	console << (each in std.io.file.lines("tmp:///some-file.txt")) << ‘\n’
		on fail(e) do
		console.error << “failed to read ‘” << e.filename << “: ” << e.reason << ‘\n’;
}
