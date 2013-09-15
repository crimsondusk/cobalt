#include "main.h"
#include "config.h"
#include "xml.h"
#include "format.h"

// =============================================================================
// -----------------------------------------------------------------------------
namespace CoConfig {
	ConfigData g_configData[COBALT_MAX_CONFIG];
	int g_cfgDataCursor = 0;
	
	// =============================================================================
	// -----------------------------------------------------------------------------
	static CoXMLNode* makeXMLNode (CoStringRef name, void* ptr, Type type, CoXMLNode* root) {
		CoXMLNode* node = new CoXMLNode (name, root);
		
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
			for (const CoString& item : *(reinterpret_cast<CoStringList*> (ptr))) {
				CoXMLNode* subnode = new CoXMLNode ("item", node);
				subnode->setContents (item);
			}
			break;
		
		case IntListType:
			for (int item : *(reinterpret_cast<CoList<int>*> (ptr))) {
				CoXMLNode* subnode = new CoXMLNode ("item", node);
				subnode->setContents (CoString::fromNumber (item));
			}
			break;
		}
		
		return node;
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
			
			CoXMLNode* node = doc->root()->findSubNode (i.name);
			
			if (node)
				setConfigValue (i.ptr, i.type, node);
		}
		
		delete doc;
		return true;
	}
	
	// =============================================================================
	// Save the configuration to disk
	// -----------------------------------------------------------------------------
	bool save (CoStringRef fname) {
		CoXMLDocument* doc = CoXMLDocument::newDocumentWithRoot ("config");
		print ("Saving configuration to %1...\n", fname);
		
		for (alias i : g_configData) {
			if (i.name == null)
				break;
			
			print ("write %1\n", i.name);
			makeXMLNode (i.name, i.ptr, i.type, doc->root());
		}
		
		bool r = doc->save (fname);
		delete doc;
		return r;
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
}