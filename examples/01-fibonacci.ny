//! Fibonacci (recursive way)
func fibonacci(n: u32): u32
	-> if n < 2u then n else fibonacci(n - 1u) + fibonacci(n - 2u);

func main {
	console << fibonacci(10u) << "\n";
}
