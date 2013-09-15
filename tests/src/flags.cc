#include <cstdlib>
#include <libcobaltcore/flags.h>

enum TestFlag {
	A = (1 << 0),
	B = (1 << 1),
	C = (1 << 2),
	D = (1 << 3),
	E = (1 << 4),
};

typedef CoFlags<TestFlag> TestFlags;

int main() {
	TestFlags a = (B | C);
	a |= A;
	a &= ~(C | E);
	a |= D | C;
	TestFlags b = (A | B | C | D);
	
	printf ("a: 0x%X\n", a);
	printf ("b: 0x%X\n", b);
	return a == b ? EXIT_SUCCESS : EXIT_FAILURE;
}