public func main(args)
{
	for filename in args do
	{
		// read the whole content of the file
		var content = std.io.file.read(filename) on fail(e) do
		{
			console.error << “failed to open ‘” << e.filename << “: ” << e.reason << ‘\n’;
			continue;
		};
		console << filename << “:\n” << content << ‘\n’;
	}
}
