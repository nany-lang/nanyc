func main
{
	var circle = new class {
		var radius = 2.0;
		var diameter -> { get: radius * 2.0, set: radius = value / 2.0 };

		var circumference -> {
			get: 2.0 * std.math.pi * radius,
			set: diameter = value / std.math.pi };
	};

	console << "radius = " << circle.radius << " circumference: " << circle.circumference" << "\n\n";

	circle.circumference = 42.0;
	console << "new circumference: " << circle.circumference << "\n");
	console << "new radius = " << circle.radius << "\n";
}
