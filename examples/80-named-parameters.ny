func load(memory)
{
	console << “loading content from memory content: \(memory) \n”;
}

func load(file)
{
	console << “loading content from url: \(url) \n”;
}


public func main
{
	load(memory: “hello world”);
	load(file: :url{http://nany.io/index.html});
}
