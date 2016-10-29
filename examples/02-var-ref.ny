func main: i32 {
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
