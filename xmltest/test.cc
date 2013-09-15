#include "../libcobaltcore/main.h"
#include "../libcobaltcore/config.h"
#include "../libcobaltcore/format.h"

static const CoString G_ConfigFile = "cobalt.xml";

CONFIG (Int, test1, 5)
CONFIG (Float, test2, 3.51f)
CONFIG (String, test3, "This is a test!")
CONFIG (StringList, test4, CoStringList())
CONFIG (IntList, test5, CoList<int>())

int main() {
	if (!CoConfig::load (G_ConfigFile))
		fprint (stderr, "couldn't open cobalt.xml: %1\n", strerror (errno));
	
	CoString argh;
	for (int i = 0; i < rand() % 12; ++i)
		argh += rand() % ('z' - 'a') + 'a';
	
	test1 += 1;
	test2 *= 2.5f;
	test3 += " z";
	test4 << argh;
	test5 << rand() % 255;
	
	CoConfig::save (G_ConfigFile);
}