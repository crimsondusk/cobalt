#include <libcobaltcore/string.h>

int main() {
	const CoString a = "This is a test",
		b = " a test",
		c = " a testinator",
		d = " a tes",
		e = " a tez";
	
	return
		 a.endsWith (b) &&
		!a.endsWith (c) &&
		!a.endsWith (d) &&
		!a.endsWith (e) ? EXIT_SUCCESS : EXIT_FAILURE;
}