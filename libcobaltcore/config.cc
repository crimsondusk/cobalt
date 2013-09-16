#include "main.h"
#include "config.h"
#include "xml.h"
#include "format.h"

// =============================================================================
// -----------------------------------------------------------------------------
namespace CoConfig {
	ConfigData              g_configData[COBALT_MAX_CONFIG];
	int                     g_cfgDataCursor = 0;
	static CoXMLDocument*   G_XMLDocument = null;
	
	// =============================================================================
	// -----------------------------------------------------------------------------
	static void updateXMLNode (CoStringRef name, void* ptr, Type type) {
		CoXMLNode* node = G_XMLDocument->navigateTo (name.split ("_"), true);
		if (!node)
			node = new CoXMLNode (name, G_XMLDocument->root());
		
		switch (type) {
		case IntType:
			node->setContents (CoString::fromNumber (*(reinterpret_cast<int*> (ptr))));
			break;
		
		case StringType:
			node->setContents (*(reinterpret_cast<CoString*> (ptr)));
			break;
		
		case FloatType:
			node->setContents (CoString::fromNumber (*(reinterpret_cast<float*> (ptr))));
			break;
		
		case BoolType:
			node->setContents (*(reinterpret_cast<bool*> (ptr)) ? "true" : "false");
			break;
		
		case StringListType:
			for (CoXMLNode* subnode : node->nodes())
				delete subnode;
			
			for (const CoString& item : *(reinterpret_cast<StringList*> (ptr))) {
				CoXMLNode* subnode = new CoXMLNode ("item", node);
				subnode->setContents (item);
			}
			break;
		
		case IntListType:
			for (int item : *(reinterpret_cast<IntList*> (ptr))) {
				CoXMLNode* subnode = new CoXMLNode ("item", node);
				subnode->setContents (CoString::fromNumber (item));
			}
			break;
		
		case StringMapType:
			for (auto pair : *(reinterpret_cast<StringMap*> (ptr))) {
				CoXMLNode* subnode = new CoXMLNode (pair.first, node);
				subnode->setContents (pair.second);
			}
			break;
		}
	}
	
	// =============================================================================
	// -----------------------------------------------------------------------------
	static void setConfigValue (void* ptr, Type type, CoXMLNode* node) {
		switch (type) {
		case IntType:
			*(reinterpret_cast<int*> (ptr)) = node->contents().toLong();
			break;
		
		case StringType:
			*(reinterpret_cast<CoString*> (ptr)) = node->contents();
			break;
		
		case FloatType:
			*(reinterpret_cast<float*> (ptr)) = node->contents().toFloat();
			break;
		
		case BoolType:
			{
				CoString val = node->contents();
				bool& var = *(reinterpret_cast<bool*> (ptr));
				
				if (val == "true" || val == "1" || val == "on" || val == "yes")
					var = true;
				else
					var = false;
			}
			break;
		
		case StringListType:
			{
				CoStringList& var = *(reinterpret_cast<CoStringList*> (ptr));
				
				for (const CoXMLNode* subnode : node->nodes())
					var << subnode->contents();
			}
			break;
		
		case IntListType:
			{
				CoList<int>& var = *(reinterpret_cast<CoList<int>*> (ptr));
				
				for (const CoXMLNode* subnode : node->nodes())
					var << subnode->contents().toLong();
			}
			break;
		
		case StringMapType:
			{
				StringMap& var = *(reinterpret_cast<StringMap*> (ptr));
				
				for (const CoXMLNode* subnode : node->nodes())
					var[subnode->name()] = subnode->contents();
			}
			break;
		}
	}
	
	// =============================================================================
	// Load the configuration from file
	// -----------------------------------------------------------------------------
	bool load (CoStringRef fname) {
		print ("config::load: Loading configuration file from %1\n", fname);
		
		CoXMLDocument* doc = CoXMLDocument::load (fname);
		if (!doc)
			return false;
		
		for (alias i : g_configData) {
			if (i.name == null)
				break;
			
			CoXMLNode* node = doc->navigateTo (CoString (i.name).split ("_"));
			
			if (node)
				setConfigValue (i.ptr, i.type, node);
		}
		
		G_XMLDocument = doc;
		return true;
	}
	
	// =============================================================================
	// Save the configuration to disk
	// -----------------------------------------------------------------------------
	bool save (CoStringRef fname) {
		if (G_XMLDocument == null)
			G_XMLDocument = CoXMLDocument::newDocumentWithRoot ("config");
		
		print ("Saving configuration to %1...\n", fname);
		
		for (alias i : g_configData) {
			if (i.name == null)
				break;
			
			updateXMLNode (i.name, i.ptr, i.type);
		}
		
		return G_XMLDocument->save (fname);
	}
	
	// =============================================================================
	// Reset configuration defaults.
	// -----------------------------------------------------------------------------
	/*
	void reset() {
		for (alias i : g_configData) {
			if (i.name == null)
				break;
			
			setConfigValue (i.ptr, i.type, i.def);
		}
	}
	*/
	
	CoXMLDocument* xml() {
		return G_XMLDocument;
	}
}