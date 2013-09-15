#include <libcobaltcore/string.h>

int main() {
	const CoString a = "Cars go quite fast but snails go slow",
		b = a.substr (33, 34),
		c = a.substr (1, 1),
		d = a.substr (11, 12),
		e = a.substr (24, 28);
	
	b.dump();
	c.dump();
	d.dump();
	e.dump();
	return (b + c + d + e == "slatenails") ? EXIT_SUCCESS : EXIT_FAILURE;
}