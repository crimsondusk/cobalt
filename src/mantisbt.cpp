#include "mantisbt.h"
#include "soap/soapH.h"
#include "soap/MantisConnectBinding.nsmap"
#include "utility.h"
#include "irc/irc.h"

CONFIG (String, tracker_url, "zandronum.com/tracker")
CONFIG (String, tracker_account, "")
CONFIG (String, tracker_password, "")

#define SOAP_STRING(N) const_cast<char*> (str (N).chars())
#define SOAP_ENDPOINT SOAP_STRING (endpoint())
#define SOAP_ACTION(N) const_cast<char*> (soapaction( #N ).chars())
#define TRACKER_ACCOUNT const_cast<char*> (tracker_account.chars())
#define TRACKER_PASSWORD const_cast<char*> (tracker_password.chars())

// Get an IRC color by status
static str ticketStatusColor (int val) {
	switch (val) {
#define CASE(A, B)  case A: return IRC::getColor (IRC::Black, IRC::B);
		CASE (10, Red)          // new
		CASE (20, LightPurple)  // feedback
		CASE (30, Orange)       // acknowledged
		CASE (40, Yellow)       // confirmed
		CASE (50, LightBlue)    // assigned
		CASE (60, SkyBlue)      // review
		CASE (70, Cyan)         // testing
		CASE (80, Green)        // resolved
		CASE (90, LightGray)    // closed
	}

	return "";
}

static str endpoint() {
	return fmt ("http://%1/api/soap/mantisconnect.php", tracker_url);
}

static str soapaction (str name) {
	return fmt ("http://%1/api/soap/mantisconnect.php/%2", tracker_url, name);
}

str issueStringRep (ns1__IssueData* data) {
	return fmt ("[%1] Ticket #%2 (%3): reporter: \002%4\017, "
		"assigned to: \002%5\017, status: \002%6%7\017, resolution: \002%8\017",
		str (data->project->name),
		str (data->id),
		str (data->summary),
		str (data->reporter->name ? data->reporter->name : "???"),
		(data->handler) ? str (data->handler->name) : "None",
		ticketStatusColor (atoi (data->status->id)),
		str (data->status->name),
		str (data->resolution->name));
}

bool ticketinfo (str idstr, str& val) {
	print ("Requesting info on %1\n", idstr);
	struct soap* soap = soap_new();
	
	// what a type name, really
	struct ns1__mc_USCOREissue_USCOREgetResponse resp;
	
	bool result = false;
	
	// Request the issue data:
	if (soap_call_ns1__mc_USCOREissue_USCOREget (soap,
			SOAP_ENDPOINT,
			SOAP_ACTION (mc_issue_get),
			TRACKER_ACCOUNT,
			TRACKER_PASSWORD,
			SOAP_STRING (idstr),
			&resp) == SOAP_OK)
	{
		val = issueStringRep (resp.return_);
		result = true;
	} else {
		const char** err = soap_faultstring (soap);
		val = fmt ("Error retrieving info for ticket %1: %2", idstr, str (*err));
		result = false;
	}
	
	soap_end (soap);
	soap_free (soap);
	return result;
}

str fullticketinfo (str idstr) {
	struct soap* soap = soap_new();
	struct ns1__mc_USCOREissue_USCOREgetResponse resp;
	str val;
	
	// Request the issue data:
	if (soap_call_ns1__mc_USCOREissue_USCOREget (soap,
			SOAP_ENDPOINT,
			SOAP_ACTION (mc_issue_get),
			TRACKER_ACCOUNT,
			TRACKER_PASSWORD,
			SOAP_STRING (idstr),
			&resp) == SOAP_OK) {
		ns1__IssueData* ticket = resp.return_;
		val += fmt ("[%1] Ticket #%2: %3\n",
			str (ticket->project->name),
			str (ticket->id),
			str (ticket->summary));
		
		val += fmt ("Reporter: %1, Assigned To: %2\n",
			str (ticket->reporter->name ? ticket->reporter->name : "None"),
			str (ticket->handler ? ticket->handler->name : "None"));
		
		val += fmt ("Priority: %1, Severity: %2, Reproducibility: %3\n",
			str (ticket->priority->name),
			str (ticket->severity->name),
			str (ticket->reproducibility->name));
		
		val += fmt ("Status: %1%2\017, resolution: %3\n",
			ticketStatusColor (atoi (ticket->status->id)),
			str (ticket->status->name),
			str (ticket->resolution->name));
		
		val += fmt ("Product Version: %1, Target Version: %2, Fixed in: %3\n",
			str (ticket->version ? ticket->version : "none"),
			str (ticket->target_USCOREversion ? ticket->target_USCOREversion : "none"),
			str (ticket->fixed_USCOREin_USCOREversion ? ticket->fixed_USCOREin_USCOREversion : "none"));
		
		RelationshipDataArray* rels = ticket->relationships;
		
		if (rels) {
			val += "Relationships:\n";
			
			for (int i = 0; i < rels->__size; ++i) {
				ns1__RelationshipData* rel = rels->__ptr[i];
				str id = rels->__ptr[i]->target_USCOREid;
				str relname;
				str type = str (rel->type->name);
				bool success = ticketinfo (id, relname);
				
				if (success)
					val += fmt ("%1. %2: %3\n", i + 1, type, relname);
			}
		}
	} else {
		const char** err = soap_faultstring (soap);
		val = fmt ("Error retrieving info for ticket %1: %2", idstr, str (*err));
	}
	
	soap_end (soap);
	soap_free (soap);
	return val;
}