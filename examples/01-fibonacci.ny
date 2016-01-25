//! Fibonacci (recursive way)
func fibonacci(n: u64): u64
	-> if n < 2 then n else fibonacci(n - 1) + fibonacci(n - 2);


public func main
{
	console << fibonacci(10u) << ‘\n’;
}
