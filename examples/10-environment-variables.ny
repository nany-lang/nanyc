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
