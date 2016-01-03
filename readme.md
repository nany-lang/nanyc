Nany Compiler
=============

[![Join the chat at https://gitter.im/nany-lang/nany](https://badges.gitter.im/nany-lang/nany.svg)](https://gitter.im/nany-lang/nany?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://api.travis-ci.org/nany-lang/nany.png)](https://travis-ci.org/nany-lang/nany)

Nany is a high-productivity language, meant to be either compiled to native
code or bytecode, or to be run as script.


Contributing
------------

**Note**: this project uses [Zenhub](https://www.zenhub.io/) for ticket management and upvoting




Overview of the Nany Programming Language
-----------------------------------------

Here are some of the features of the Nany Language.

*note*: the current implementation of the Nany compiler may not implement all the
features required for compiling the following examples.

### Traditional Hello world

```nany
public func main
{
    console << “Hello world\n”;
}
```

### Fibonacci

```nany
//! Fibonacci (recursive way)
func fibonacci(n)
    -> if n < 2 then n else fibonacci(n - 1) + fibonacci(n - 2);

public func main
{
    console << fibonacci(10) << ‘\n’;
}
```

### Variables & References

```nany
public func main: i32
{
    var x = “hello world”;
    var y = z; // make a copy of x
    ref z = x; // x and z are actually the same variable

    // some standard types, even if not needed
    var languageName: string = “nany.io”;
    var version: f64 = 1.0;
    var introduced: i32 = 2015;
    var isAwesome: bool = true;
    return 0;
}
```

### Working with I/O, reading a file

```nany
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
```

### Working with I/O, reading a file line by line

```nany
public func main
{
    // print to the console the content of the file line-by-line
    console << (each in std.io.file.lines("tmp:///some-file.txt")) << ‘\n’
    on fail(e) do
        console.error << “failed to read ‘” << e.filename << “: ” << e.reason << ‘\n’;
}
```


### Scripting, interacting with environment variables

```nany
public func main
{
    // hard-coded env variables can directly be manipulated via the special
    // object ‘$’, like in shell
    // retrieve the value of the env variable “HOME”
    console << “HOME = ” << $HOME << “\n”;
    // set the value of the env variable “MY_ENV_VAR”
    $MY_ENV_VAR = “some text”;

    // the object $ behaves like a dictionary (string -> string)
    $[“ANOTHER_ENV_VAR”] = “some text”;
    console << “HTTP_PROXY available: ” << $.contains(“HTTP_PROXY”) << “\n”;

    // printing all env variables
    console << “--\nenv variables:\n”;
    for v in std.env.variables do
        console << v.name << “ = ” << v.value << ‘\n’;
}
```


### Parallel computing

```nany
// This example computes the total number of audio files available in each folder
// provided from the command line.
// Note the presence of the & operator which will dispatch each call into a new job.
// Since the operator += is allowed when creating a variable (which will be initialized
// with the default value), the each construct can be used in combination with += to
// obtain a really concise and elegant syntax for a complicated task.

func howManyAudioFilesAtUrl(url)
{
	switch std.io.type(url) do
    {
        case ntFolder:
        {
            // the given url is a folder
            // retrieving a virtual list of all files in this folder, matching our criteria
            var allfiles = (inode in std.io.directory(url).entries) | inode.type == ntFile
                and inode.name == :regex{ .*\.{mp3,wav,ogg,wma} };

            // retrieving how many files this folder has
            return allfiles.size;
        }
    }
	else
		return 0;
}


public func main(args)
{
    // The magic happens here !
    // Using the value for variable howManyFiles triggers a synchronization,
    // i.e. the statement will not run until all jobs are finished and the final value computed
    var howManyFiles += & howManyAudioFilesAtUrl((each in args));

    console << "total number of files : " << howManyFiles << “\n”;
}
```


### Named parameters

```nany
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
```


### Properties

```nany
class Circle
{
    var radius = 42.6;
    var diameter -> {get: radius * 2, set: radius = value / 2};
    var circumference -> {get: 2 * std.math.pi * radius, set: diameter = value / std.math.pi};
}
```
