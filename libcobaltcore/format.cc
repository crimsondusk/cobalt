#include <stdio.h>
#include "variant.h"
#include "misc.h"
#include "format.h"

// =============================================================================
// -----------------------------------------------------------------------------
void DoPrint (FILE* fp, std::initializer_list<CoVariant> args) {
	CoString msg = DoFormat (args);
	fprintf (fp, "%s", msg.chars());
}

// =============================================================================
// -----------------------------------------------------------------------------
CoString DoFormat (std::initializer_list<CoVariant> args) {
	assert (args.size() >= 1);
	assert (args.begin()->valueType() == CoVariant::StringType);
	
	bool perc = false;
	uint percnum = 0;
	int percdigits = 0;
	str fmtd;
	str percstr;
	CoVariant::StringFlags flags = CoVariant::NoFlags;
	str fmtstr = args.begin()->asString();
	
	for (const char* c = &fmtstr[0];; c++) {
		if (perc) {
			if (*c >= '0' && *c <= '9') {
				// Argument digit
				percnum += (*c - '0') * exponent (10, percdigits);
				percdigits++;
				percstr += *c;
				continue;
			} elif (percdigits > 0) {
				// End of argument, fill in the value
				if (args.size() < percnum + 1) {
					fprint (stderr, "Too few arguments to format string `%1`\n", fmtstr);
					fmtd += percstr;
				} else
					fmtd += (args.begin() + percnum)->stringRep (flags);
				
				perc = false;
			} else {
				// '%' followed by a non-digit. If we got %%, add both percentage signs
				// and jump to the start of the loop to prevent the latter % from being
				// intepreted as an argument symbol.
				
				perc = false;
				fmtd += "%";
				
				if (*c == '%') {
					fmtd += "%";
					continue;
				}
			}
		}
		
		if (*c == '\0')
			break;
		
		if (*c == '%') {
			if (perc) {
				// Another case of %%
				perc = false;
				fmtd += "%%";
			} else {
				// We got '%', begin intepreting argument
				perc = true;
				percdigits = 0;
				percnum = 0;
				percstr = "%";
			}
			
			continue;
		}
		
		fmtd += *c;
	}
	
	return fmtd;
}