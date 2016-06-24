func main
{
	var radius = 42.6;
	var diameter -> { get: radius * 2, set: radius = value / 2 };

	var circumference -> {
		get: 2 * std.math.pi * radius,
		set: diameter = value / std.math.pi };

	console << "circle: radius = \(radius), circumference: \(circumference)\n";

	circumference = 21;
	console << "new radius: \(radius)\n";
}
