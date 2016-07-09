# Overview of the Nany Programming Language

Here are some of the features of the Nany Language.

*note*: the current status of the Nany compiler may not be able to compile
all the following examples. Some feature may still be missing but we're
working on it !



## The traditional Hello world

```nany
func main
{
	console << "Hello world!\n";
}
```



## Fibonacci

```nany
//! Fibonacci (recursive way)
func fibonacci(n: u32): u32
	-> if n < 2u then n else fibonacci(n - 1u) + fibonacci(n - 2u);


func main
{
	console << fibonacci(10u) << "\n";
}
```



## Variables & References

```nany
func main: i32
{
	var x = "hello world";
	var y = z; // make a copy of x
	ref z = x; // x and z are actually the same variable

	// some standard types, even if types can be omitted
	var languageName: string = "nany.io";
	var version: f32 = 1.0;
	var introduced: i32 = 2015;
	var isAwesome: bool = true;
	return 0;
}
```



## Working with I/O, reading a file

```nany
func main(args)
{
	for filename in args do
	{
		// read the whole content of the file
		var content = std.io.read(filename);
		console << filename << ":\n" << content << "\n";
	}
}
```

- [ ] feature not fully implemented yet



## Working with I/O, reading a file line by line

```nany
func main
{
	// for each line, print it to the standard output
	console << (each in std.io.file("/tmp/myfile.txt")) << "\n";

	// the same than above, but with line numbers
	var lineNumber = 0;
	for line in std.io.file("/tmp/myfile.txt") do
		console << (++lineNumber) << ": " << line << "\n";
}
```
- [ ] feature not fully implemented yet



## Scripting, interacting with environment variables

```nany
func main
{
	// hard-coded env variables can directly be manipulated via the special
	// object ‘$’, like in shell
	// retrieve the value of the env variable “HOME”
	console << "HOME = " << $HOME << "\n";
	// set the value of the env variable “MY_ENV_VAR”
	$MY_ENV_VAR = "some text";

	// the object $ behaves like a dictionary (string -> string)
	$["ANOTHER_ENV_VAR"] = "some text";
	console << "HTTP_PROXY available: " << std.env.exists("HTTP_PROXY") << “\n”;

	// printing all env variables
	console << "--\nenv variables:\n";
	for v in std.env.variables do
		console << v.name << " = " << v.value << "\n";
}
```
- [ ] feature not fully implemented yet



## Parallel computing

```nany
// This example computes the total number of audio files available in each folder
// provided from the command line.
// Note the presence of the & operator which will dispatch each call into a new job.
// Since the operator += is allowed when creating a variable (which will be initialized
// with the default value), the each construct can be used in combination with += to
// obtain a really concise and elegant syntax for a complicated task.

func howManyAudioFilesAtUrl(url): u64
{
	switch std.io.type(url) do
	{
		case ntFolder:
		{
			// the given url is a folder
			// retrieving a virtual list of all files in this folder, matching our criteria
			var files = (inode in std.io.folder(url) :recursive) | inode.type == ntFile
				and inode.name == :regex{ .*\.{mp3,wav,ogg,wma} };

			// retrieving how many files this folder has
			return files.size;
		}
	}
	else
		return 0;
}


func main(args)
{
	// The magic happens here !
	// Using the value for variable howManyFiles triggers a synchronization,
	// i.e. the statement will not run until all jobs are finished and the final value computed
	var howManyFiles += & howManyAudioFilesAtUrl((each in args));

	console << "total number of files : " << howManyFiles << "\n";
}
```
- [ ] feature not fully implemented yet



## Named parameters

```nany
func load(memory)
{
	console << "loading content from memory content: \(memory) \n";
}

func load(file)
{
	console << "loading content from file: \(url) \n";
}

func main
{
	load(memory: “hello world”);
	load(file: "/tmp/myfile.txt");
}
```



## String interpolation

```nany
func main
{
	var x = 10;
	var y = 42;

	// x = y = 10 + 42 = 52
	console << "x + y = \(x) + \(y) = \(x + y)\n";
}
```



## Properties

```nany
func main
{
	var circle = new class {
		var radius = 2.0;
		var diameter -> { get: radius * 2.0, set: radius = value / 2.0 };

		var circumference -> {
			get: 2.0 * std.math.pi * radius,
			set: diameter = value / std.math.pi };
	};

	print("radius = \(circle.radius), circumference: \(circle.circumference)\n\n");

	circle.circumference = 42.0;
	print("new circumference: \(circle.circumference)\n");
	print("new radius = \(circle.radius)\n");
}
```



## Resources

 * [Feature Matrix](http://nany.io/en/feature-matrix/)
 * [Documentation](http://nany.io/en/docs/latest/)
