#include "common.h"
#include "scanner.h"
#include "str.h"
#include "config.h"

inline bool IsCharWhitespace (char c) {
	return (c <= 32 || c == 127);
}

Scanner::Scanner(str input) {
	cursor = 0;
	buffer = input;
}

bool Scanner::Next (bool raw) { 
	str work;
	token = "";
	for (;;) {
		if (cursor >= buffer.len())
			break;
		
		char c = buffer[cursor++];
		
		// Possible breakpoint
		if (!raw && ((c >= 33 && c <= 47) ||
			(c >= 58 && c <= 64) ||
			(c >= 91 && c <= 96 && c != '_') ||
			(c >= 123 && c <= 126)))
		{
			
			// Slide past crap
			while (cursor < buffer.len() && IsCharWhitespace (c))
				cursor++;
			
			// Don't break out if we're at the beginning of the token.
			if (work.len() > 0)
				cursor--;
			else
				work += c;
			break;
		}
		
		if (IsCharWhitespace (c)) {
			// Don't break if we haven't gathered anything yet.
			if (work.len())
				break;
		} else {
			work += c;
		}
	}
	
	token = work;
	return (token.len() > 0);
}

bool Scanner::MustNext (str stuff) {
	str old_token = token;
	
	if (!Next()) {
		if (stuff.len())
			parseError ("expected `%s`", stuff.chars());
		else
			parseError ("unexpected end-of-command");
		return false;
	}
	
	if (stuff.len() && !!token.compare (stuff)) {
		parseError ("expected `%s` after `%s`", (char*)stuff, (char*)old_token);
		return false;
	}
	
	return true;
}

bool Scanner::MustString () {
	MustNext ("\"");
	
	token = "";
	
	while (cursor < buffer.len()) {
		char c = buffer[cursor++];
		
		if (c == '\"')
			return true;
		else
			token += c;
	}
	
	parseError ("unterminated string");
	return false;
}

bool Scanner::MustLiteral () {
	if (!Next() || token.isnumber())
		return false;
	return true;
}

str Scanner::PeekNext () {
	ulong oldcurs = cursor;
	str oldtoken = token;
	
	if (!Next())
		return "";
	
	str peeked = token;
	cursor = oldcurs;
	token = oldtoken;
	
	return peeked;
}