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
