#ifndef LIBCOBALT_XML_H
#define LIBCOBALT_XML_H

#include <climits>
#include "main.h"

class CoXMLNode;

// =============================================================================
// -----------------------------------------------------------------------------
class CoXMLDocument {
public:
	CoXMLDocument (CoXMLNode* root = null);
	~CoXMLDocument();
	
	CoXMLNode* findNodeByName (CoStringRef name) const;
	bool       save (CoStringRef fname) const;
	void       setRootNode (CoXMLNode* root);
	
	inline CoXMLNode* root() const {
		return m_root;
	}

	static CoXMLDocument*   newDocumentWithRoot (CoStringRef rootName);
	static CoXMLDocument*   load (CoStringRef fname);
	static CoStringRef      parseError();
	static CoString         encode (CoStringRef in);
	static CoString         decode (CoStringRef in);
	
protected:
	void setHeader (const CoMap<CoString, CoString>& header);
	
private:
	void writeNode (FILE* fp, const CoXMLNode* node) const;
	
	CoXMLNode*                  m_root;
	CoMap<CoString, CoString>   m_header;
};

// =============================================================================
// -----------------------------------------------------------------------------
class CoXMLNode {
	PROPERTY (CoString, name, setName)
	
public:
	CoXMLNode (CoStringRef name, CoXMLNode* parent);
	~CoXMLNode();
	
	CoString     attribute (CoStringRef name) const;
	CoStringRef  contents() const;
	void         dropNode (CoXMLNode* node);
	CoXMLNode*   findSubNode (CoStringRef fname, bool recursive = false);
	bool         hasAttribute (CoStringRef name);
	void         setAttribute (CoStringRef name, CoStringRef data);
	void         setCDATA (bool v);
	void         setContents (CoStringRef data);
	
	inline const CoMap<CoString, CoString>& attributes() const {
		return m_attrs;
	}
	
	inline const CoList<CoXMLNode*>& nodes() const {
		return m_nodes;
	}
	
	inline bool isCDATA() const {
		return m_isCData;
	}
	
	inline bool isSelfEnclosing() const {
		return m_isSelfEnclosing;
	}
	
	static CoXMLNode* newSelfEnclosingNode (CoStringRef name, CoXMLNode* parent);
	
protected:
	CoString                    m_contents;
	CoList<CoXMLNode*>          m_nodes;
	CoMap<CoString, CoString>   m_attrs;
	bool                        m_isCData,
	                            m_isSelfEnclosing;
	friend class CoXMLDocument;
};

#endif // LIBCOBALT_XML_H