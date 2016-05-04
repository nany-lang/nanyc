//! Fibonacci (recursive way)
func fibonacci(n: u64): u64
	-> if n < 2u then n else fibonacci(n - 1u) + fibonacci(n - 2u);


public func main
{
	console << fibonacci(10u) << "\n";
}
