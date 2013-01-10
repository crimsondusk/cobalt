#include "common.h"
#include "wad.h"

wad::wad (const char* filepath) {
	data = NULL;
	
	wadname = basename (filepath);
	
	FILE* fp = fopen (filepath, "r");
	if (!fp) {
		errormsg.format ("Couldn't open %s for reading", filepath);
		return;
	}
	
	char sig[5];
	if (fread (sig, 1, 4, fp) < 4) {
		errormsg = "Couldn't read WAD signature!";
		fclose (fp);
		return;
	}
	sig[4] = '\0';
	
	if (strcmp (sig, "IWAD") != 0 && strcmp (sig, "PWAD") != 0) {
		errormsg = "This is not a WAD file!";
		fclose (fp);
		return;
	}
	
	assert (sizeof (long) == 4);
	if (fread (&numlumps, 4, 1, fp) < 1) {
		errormsg = "Couldn't read the amount of lumps";
		fclose (fp);
		return;
	}
	
	long dirofs;
	if (fread (&dirofs, 4, 1, fp) < 1) {
		errormsg = "Couldn't read directory offset";
		fclose (fp);
		return;
	}
	
	// Read the data
	datalen = dirofs - 12;
	data = new byte[datalen];
	if (fread (data, 1, datalen, fp) < datalen) {
		errormsg = "Couldn't read WAD data";
		fclose (fp);
		delete[] data;
		data = NULL;
		return;
	}
	
	// Read the offset
	assert (ftell (fp) == dirofs);
	lumpinfo info;
	while (fread (&info, sizeof info, 1, fp)) {
		info.label[8] = '\0';
		directory << info;
	}
	
	printf ("%s successfully read.\n", wadname.chars());
	fclose (fp);
	init = true;
}

wad::~wad () {
	if (data)
		delete[] data;
}