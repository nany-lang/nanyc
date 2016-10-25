func load(memory) {
	console << "loading content from memory content: \(memory) \n";
}

func load(file) {
	console << "loading content from file: \(url) \n";
}

func main {
	load(memory: “hello world”);
	load(file: "/tmp/myfile.txt");
}
