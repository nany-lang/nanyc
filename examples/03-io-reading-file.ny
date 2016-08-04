func main(args)
{
	for filename in args do
	{
		// read the whole content of the file
		var content = std.io.file.read(filename);
		console << filename << ":\n" << content << '\n';
	}
}
