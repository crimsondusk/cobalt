#ifndef LIBCOBALTCORE_XML_SCANNER_H
#define LIBCOBALTCORE_XML_SCANNER_H

#include "main.h"

class CoXMLScanner {
public:
	enum Token {
		HeaderStart,
		HeaderEnd,
		TagCloser,
		TagSelfCloser,
		TagStart,
		TagEnd,
		CData,
		Equals,
		Symbol,
		String
	};
	
	CoXMLScanner (const char* data);
	
	bool next();
	bool next (Token tok);
	
	inline Token tokenType() const {
		return m_tokenType;
	}
	
	inline CoStringRef token() const {
		return m_token;
	}
	
	inline bool insideTag() const {
		return m_insideTag;
	}
	
private:
	bool checkString (const char* c, bool peek = false);
	
	const char*   m_data;
	const char*   m_ptr;
	CoString      m_token;
	Token         m_tokenType;
	bool          m_insideTag;
};

#endif // LIBCOBALTCORE_XML_SCANNER_H