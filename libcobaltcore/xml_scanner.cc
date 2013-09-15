#include "xml_scanner.h"
#include "xml.h"

static const char* G_XMLTokens[] = {
	"<?xml",     // HeaderStart
	"?>",        // HeaderEnd
	"</",        // TagCloser
	"/>",        // TagSelfCloser
	"<",         // TagStart
	">",         // TagEnd
};

static const char* G_CDataStart   = "<![CDATA[";
static const char* G_CDataEnd     = "]]>";
static const char* G_CommentStart = "<!--";
static const char* G_CommentEnd   = "-->";

// =============================================================================
// -----------------------------------------------------------------------------
CoXMLScanner::CoXMLScanner (const char* data) :
	m_data      (data),
	m_ptr       (&m_data[0]),
	m_insideTag (false) {}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoXMLScanner::checkString (const char* c, bool peek) {
	const bool r = strncmp (m_ptr, c, strlen (c)) == 0;
	
	if (r && !peek)
		m_ptr += strlen (c);
	
	return r;
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoXMLScanner::next() {
	m_token = "";
	
	while (isspace (*m_ptr))
		m_ptr++;
	
	if (*m_ptr == '\0')
		return false;
	
	// Skip any comments
	while (checkString (G_CommentStart))
		while (!checkString (G_CommentEnd))
			m_ptr++;
	
	// Check and parse CDATA
	if (checkString (G_CDataStart)) {
		while (!checkString (G_CDataEnd))
			m_token += *m_ptr++;
		
		m_tokenType = CData;
		return true;
	}
	
	// Check "<", ">", "/>", ...
	for (int i = 0; i < (signed) (sizeof G_XMLTokens / sizeof *G_XMLTokens); ++i) {
		if (checkString (G_XMLTokens[i])) {
			m_token = G_XMLTokens[i];
			m_tokenType = (Token) i;
			
			// We need to keep track of when we're inside node tags so we can
			// stop on '=' signs for attributes when inside tags where '=' has
			// special meaning but not outside tags where it's just a glyph.
			if (i == TagStart || i == HeaderStart)
				m_insideTag = true;
			elif (i == TagEnd || TagSelfCloser || i == HeaderEnd)
				m_insideTag = false;
			
			return true;
		}
	}
	
	// Check and parse string
	if (*m_ptr == '\"') {
		m_ptr++;
		
		while (*m_ptr != '\"') {
			if (!*m_ptr)
				return false;
			
			if (checkString ("\\\"")) {
				m_token += "\"";
				continue;
			}
			
			m_token += *m_ptr++;
		}
		
		m_tokenType = String;
		m_ptr++; // skip the final quote
		return true;
	}
	
	m_tokenType = Symbol;
	while (m_ptr != '\0') {
		if (m_insideTag && isspace (*m_ptr))
			break;
		
		// Stop at '=' if inside tag
		if (m_insideTag && *m_ptr == '=') {
			if (m_token.length() > 0)
				break;
			else {
				m_tokenType = Equals;
				m_token = "=";
				m_ptr++;
				return true;
			}
		}
		
		bool stopHere = false;
		for (int i = 0; i < (signed) (sizeof G_XMLTokens / sizeof *G_XMLTokens); ++i) {
			if (checkString (G_XMLTokens[i], true)) {
				stopHere = true;
				break;
			}
		}
		
		if (stopHere)
			break;
		
		m_token += *m_ptr++;
	}
	
	return true;
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoXMLScanner::next (CoXMLScanner::Token tok) {
	const char* oldPos = m_ptr;
	
	if (!next())
		return false;
	
	if (tokenType() != tok) {
		m_ptr = oldPos;
		return false;
	}
	
	return true;
}