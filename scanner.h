#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "common.h"
#include "str.h"

class Scanner {
private:
	str buffer;
	ulong cursor;

public:
	str token;
	
	Scanner(str input);
	bool Next (bool raw = false);
	bool MustNext (str stuff = "");
	bool MustLiteral ();
	bool MustString ();
	str PeekNext (bool raw = false);
};

#endif // __SCANNER_H__