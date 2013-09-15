#include <libcobaltcore/string.h>

int main() {
	const CoString a = "This is a test",
		b = "This is",
		c = "This iz";
	
	return (a.startsWith (b) && !a.startsWith (c)) ? EXIT_SUCCESS : EXIT_FAILURE;
}