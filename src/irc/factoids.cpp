#include <libcobaltcore/xml.h>
#include "irc.h"
#include "connection.h"
#include "user.h"
static CoXMLDocument* G_FactsXML = null;

static bool initFactsXML() {
	if (G_FactsXML == null) {
		G_FactsXML = CoXMLDocument::load ("facts.xml");
		
		if (errno)
			G_FactsXML = CoXMLDocument::newDocumentWithRoot ("facts");
	}
	
	return (G_FactsXML != null);
}

static CoXMLNode* getFactoid (CoStringRef subject) {
	assert (G_FactsXML != null);
	return G_FactsXML->root()->getOneNodeByAttribute ("subject", subject);
}

IRC_COMMAND (learn) {
	if (parms.size() < 3) {
		IRC_REPLY ("I should learn what?");
		IRC_REPLY ("Usage: %1 <subject> <:text>", parms[0]);
		return;
	}
	
	CoString subject = parms[1];
	CoString text = message.substr (message.posof (2) + 1);
	
	if (subject.length() > 20) {
		IRC_REPLY ("That subject is too long.")
		return;
	}
	
	for (char c : subject) {
		if ((c < 'a' || c > 'z') &&
			(c < 'A' || c > 'Z') &&
			(c < '0' || c > '9') &&
			c != '_')
		{
			IRC_REPLY ("Subject must only contain alpha-numeric characters or underscores")
			return;
		}
	}
	
	if (!initFactsXML())
		IRC_REPLY ("Couldn't open facts.xml: %1", CoXMLDocument::parseError());
	
	// If there already is a factoid by this name, delete it now
	CoXMLNode* node;
	while ((node = G_FactsXML->root()->findSubNode (subject)) != null)
		delete node;
	
	node = new CoXMLNode ("fact", G_FactsXML->root());
	node->setAttribute ("subject", subject);
	node->addSubNode ("author", invoker->userhost());
	node->addSubNode ("time", CoString::fromNumber (CoTime::now().seconds()));
	CoXMLNode* textNode = node->addSubNode ("text", text);
	textNode->setCDATA (true);
	
	if (!G_FactsXML->save ("facts.xml"))
		IRC_REPLY ("WARNING: Failed to save facts.xml with the new data: %1\n", strerror (errno));
	
	IRC_REPLY ("I now know about '%1'", subject);
}

IRC_COMMAND (about) {
	if (parms.size() < 2) {
		IRC_REPLY ("Usage: %1 <subject>", parms[0]);
		return;
	}
	
	if (!initFactsXML())
		IRC_REPLY ("Couldn't open facts.xml: %1", CoXMLDocument::parseError());
	
	CoString subject = parms[1];
	CoXMLNode* node = getFactoid (subject);
	CoXMLNode* textNode = node ? node->findSubNode ("text") : null;
	
	if (!textNode) {
		IRC_REPLY ("I don't know anything about %1.", subject);
		return;
	}
	
	IRC_REPLY ("%1 is %2", subject, textNode->contents());
}

IRC_COMMAND (factoid_meta) {
	if (parms.size() < 2) {
		IRC_REPLY ("Usage: %1 <subject>", parms[0]);
		return;
	}
	
	if (!initFactsXML())
		IRC_REPLY ("Couldn't open facts.xml: %1", CoXMLDocument::parseError());
	
	CoString subject = parms[1];
	CoXMLNode* node = getFactoid (subject);
	
	if (!node) {
		IRC_REPLY ("No such factoid");
		return;
	}
	
	CoXMLNode* timeNode = node->findSubNode ("time"),
		*authorNode = node->findSubNode ("author");
	
	if (!timeNode || !authorNode) {
		IRC_REPLY ("Bad factoid: no time and/or author nodes");
		return;
	}
	
	CoTime ts = CoTime (timeNode->contents().toLong());
	IRC_REPLY ("Factoid '%1' was set by %2 on %3", subject, authorNode->contents(), CoDate (ts));
}

IRC_COMMAND (factoid_del) {
	if (parms.size() < 2 || !invoker->isAdmin())
		return;
	
	initFactsXML();
	if (!G_FactsXML)
		return;
	
	CoString subject = parms[1];
	CoXMLNode* node = getFactoid (subject);
	
	if (node) {
		delete node;
		G_FactsXML->save ("facts.xml");
		IRC_REPLY ("Factoid '%1' deleted", subject);
	} else
		IRC_REPLY ("No such factoid exists")
}