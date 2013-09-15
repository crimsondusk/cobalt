#include "main.h"
#include "xml.h"
#include "format.h"
#include "xml_scanner.h"

static CoString             G_XMLError;
static CoVector<CoXMLNode*> G_Stack;
static int                  G_SaveStack;

// =============================================================================
// -----------------------------------------------------------------------------
static CoXMLNode* topStackNode() {
	if (G_Stack.size() == 0)
		return null;
	
	return G_Stack[G_Stack.size() - 1];
}

// =============================================================================
// -----------------------------------------------------------------------------
#define XML_ERROR(...) { \
	G_XMLError = fmt (__VA_ARGS__); \
	delete[] buf; \
	delete root; \
	return null; }

#define XML_MUST_GET(N) \
	if (!scan.next (N)) XML_ERROR ("Expected " #N)

// =============================================================================
// -----------------------------------------------------------------------------
CoXMLDocument::CoXMLDocument (CoXMLNode* root) :
	m_root (root)
{
	m_header["version"] = "1.0";
}

// =============================================================================
// -----------------------------------------------------------------------------
CoXMLDocument::~CoXMLDocument() {
	delete m_root;
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoXMLDocument::setHeader (const CoMap<CoString, CoString>& header) {
	m_header = header;
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoXMLDocument::setRootNode (CoXMLNode* root) {
	m_root = root;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoXMLDocument* CoXMLDocument::newDocumentWithRoot (CoStringRef rootName) {
	return new CoXMLDocument (new CoXMLNode (rootName, null));
}

// =============================================================================
// -----------------------------------------------------------------------------
CoXMLDocument* CoXMLDocument::load (CoStringRef fname) {
	FILE*                     fp;
	long                      fsize;
	char*                     buf;
	CoXMLNode*                root = null;
	CoMap<CoString, CoString> header;
	
	if ((fp = fopen (fname, "r")) == null) {
		G_XMLError = fmt ("couldn't open %1 for reading", fname);
		return null;
	}
	
	fseek (fp, 0l, SEEK_END);
	fsize = ftell (fp);
	rewind (fp);
	
	buf = new char[fsize];
	if ((long) fread (buf, 1, fsize, fp) < fsize) {
		G_XMLError = fmt ("I/O error while opening %1", fname);
		fclose (fp);
		return null;
	}
	
	fclose (fp);
	
	CoXMLScanner scan (buf);
	
	XML_MUST_GET (CoXMLScanner::HeaderStart)
	
	while (scan.next (CoXMLScanner::Symbol)) {
		CoString attrname = scan.token();
		XML_MUST_GET (CoXMLScanner::Equals)
		XML_MUST_GET (CoXMLScanner::String)
		header[attrname] = scan.token();
	}
	
	XML_MUST_GET (CoXMLScanner::HeaderEnd)
	
	/*
	if (header.find ("version") == header.end())
		XML_ERROR ("No version defined in header!");
	*/
	
	while (scan.next()) {
		switch (scan.tokenType()) {
		case CoXMLScanner::TagStart:
			{
				XML_MUST_GET (CoXMLScanner::Symbol)
				CoXMLNode* node = new CoXMLNode (scan.token(), topStackNode());
				
				if (G_Stack.size() == 0) {
					if (root != null) {
						// XML forbids having multiple roots
						delete node;
						XML_ERROR ("Multiple root nodes")
					}
					
					root = node;
				}
				
				G_Stack << node;
				
				while (scan.next (CoXMLScanner::Symbol)) {
					CoString attrname = scan.token();
					XML_MUST_GET (CoXMLScanner::Equals)
					XML_MUST_GET (CoXMLScanner::String)
					node->m_attrs[attrname] = scan.token();
				}
				
				if (scan.next (CoXMLScanner::TagSelfCloser)) {
					if (node == root)
						XML_ERROR ("Root must not close self")
					
					CoXMLNode* popee;
					if (!G_Stack.pop (popee) || popee != node)
						XML_ERROR ("WTF is going on with the stack?!")
					
					node->m_isSelfEnclosing = true;
				} else
					XML_MUST_GET (CoXMLScanner::TagEnd)
			}
			break;
		
		case CoXMLScanner::TagCloser:
			{
				XML_MUST_GET (CoXMLScanner::Symbol)
				
				CoXMLNode* popee;
				if (!G_Stack.pop (popee) || popee->m_name != scan.token())
					XML_ERROR ("Misplaced closing tag")
				
				XML_MUST_GET (CoXMLScanner::TagEnd)
			}
			break;
		
		case CoXMLScanner::CData:
		case CoXMLScanner::Symbol:
			{
				if (G_Stack.size() == 0)
					XML_ERROR ("Misplaced CDATA/symbol/string")
				
				CoXMLNode* node = G_Stack[G_Stack.size() - 1];
				node->m_isCData = (scan.tokenType() == CoXMLScanner::CData);
				
				if (node->m_isCData)
					node->m_contents = scan.token();
				else
					node->m_contents = decode (scan.token());
			}
			break;
		
		case CoXMLScanner::String:
		case CoXMLScanner::HeaderStart:
		case CoXMLScanner::HeaderEnd:
		case CoXMLScanner::Equals:
		case CoXMLScanner::TagSelfCloser:
		case CoXMLScanner::TagEnd:
			XML_ERROR ("Unexpected token '%1'", scan.token());
		}
	}
	
	CoXMLDocument* doc = new CoXMLDocument (root);
	doc->setHeader (header);
	return doc;
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoXMLDocument::save (CoStringRef fname) const {
	FILE* fp;
	
	if ((fp = fopen (fname, "w")) == null)
		return false;
	
	fprint (fp, "<?xml");
	for (const auto& i : m_header)
		fprint (fp, " %1=\"%2\"", CoString (i.first), CoString (i.second));
	
	fprint (fp, " ?>\n");
	
	G_SaveStack = 0;
	writeNode (fp, m_root);
	return true;
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoXMLDocument::writeNode (FILE* fp, const CoXMLNode* node) const {
	for (int i = 0; i < G_SaveStack; ++i)
		fprint (fp, "\t");
	
	if (node->isSelfEnclosing()) {
		fprint (fp, "<%1 />\n", node->name());
		return;
	}
	
	fprint (fp, "<%1", node->name());
	
	for (const auto& attr : node->attributes())
		fprint (fp, " %1=\"%2\"", encode (attr.first), encode (attr.second));
	
	fprint (fp, ">");
	
	if (node->nodes().size() > 0) {
		// Write nodes
		fprint (fp, "\n");
		for (const CoXMLNode* subnode : node->nodes()) {
			G_SaveStack++;
			writeNode (fp, subnode);
			G_SaveStack--;
		}
		
		for (int i = 0; i < G_SaveStack; ++i)
			fprint (fp, "\t");
	} else {
		// Write content
		if (node->isCDATA())
			fprint (fp, "<![CDATA[%1]]>", node->contents());
		else
			fprint (fp, encode (node->contents()));
	}
	
	fprint (fp, "</%1>\n", node->name());
}

// =============================================================================
// -----------------------------------------------------------------------------
CoString CoXMLDocument::encode (CoStringRef in) {
	CoString out (in);
	out.replace ("&", "&amp;");
	out.replace ("<", "&lt;");
	out.replace (">", "&gt;");
	out.replace ("\"", "&quot;");
	return out;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoString CoXMLDocument::decode (CoStringRef in) {
	CoString out (in);
	out.replace ("&amp;", "&");
	out.replace ("&lt;", "<");
	out.replace ("&gt;", ">");
	out.replace ("&quot;", "\"");
	return out;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoXMLNode* CoXMLDocument::findNodeByName (CoStringRef name) const {
	return m_root->findSubNode (name, true);
}

// =============================================================================
// -----------------------------------------------------------------------------
CoStringRef CoXMLDocument::parseError() {
	return G_XMLError;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoXMLNode::CoXMLNode (CoStringRef name, CoXMLNode* parent) :
	m_name    (name),
	m_isCData (false)
{
	if (parent != null)
		parent->m_nodes << this;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoXMLNode::~CoXMLNode() {
	for (CoXMLNode* node : m_nodes)
		delete node;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoString CoXMLNode::attribute (CoStringRef name) const {
	auto it = m_attrs.find (name);
	
	if (it == m_attrs.end())
		return CoString();
	
	return it->second;
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoXMLNode::setCDATA (bool v) {
	m_isCData = v;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoStringRef CoXMLNode::contents() const {
	return m_contents;
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoXMLNode::setContents (CoStringRef data) {
	m_contents = data;
	m_isCData = false;
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoXMLNode::dropNode (CoXMLNode* node) {
	m_nodes.remove (node);
}

// =============================================================================
// -----------------------------------------------------------------------------
bool CoXMLNode::hasAttribute (CoStringRef name) {
	return m_attrs.find (name) != m_attrs.end();
}

// =============================================================================
// -----------------------------------------------------------------------------
void CoXMLNode::setAttribute (CoStringRef name, CoStringRef data) {
	m_attrs[name] = data;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoXMLNode* CoXMLNode::newSelfEnclosingNode (CoStringRef name, CoXMLNode* parent) {
	CoXMLNode* node = new CoXMLNode (name, parent);
	node->m_isSelfEnclosing = true;
	return node;
}

// =============================================================================
// -----------------------------------------------------------------------------
CoXMLNode* CoXMLNode::findSubNode (CoStringRef fname, bool recursive) {
	for (CoXMLNode* node : m_nodes) {
		if (node->name() == fname)
			return node;
		
		CoXMLNode* target;
		if (recursive && (target = node->findSubNode (fname)) != null)
			return target;
	}
	
	return null;
}