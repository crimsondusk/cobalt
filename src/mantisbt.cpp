#include <util/format.h>
#include "mantisbt.h"
#include "names.h"
#include "soap/soapH.h"
#include "soap/MantisConnectBinding.nsmap"
#include "config.h"
#include "utility.h"

#define SOAP_STRING(N) const_cast<char*>( str( N ).chars() )
#define SOAP_ENDPOINT SOAP_STRING( endpoint() )
#define SOAP_ACTION(N) const_cast<char*>( soapaction( #N ).chars() )
#define TRACKER_ACCOUNT const_cast<char*>( cfg( Name::TrackerAccount ).chars())
#define TRACKER_PASSWORD const_cast<char*>( cfg( Name::TrackerPassword ).chars())

// Get an IRC color by status
static str ticketStatusColor( int val )
{
	return valueMap<const char*>( val, "", 9,
		10, "\00301,04",  // new
		20, "\00301,13",  // feedback
		30, "\00301,07",  // acknowledged
		40, "\00301,08",  // confirmed
		50, "\00301,10",  // assigned
		60, "\00301,11",  // review
		70, "\00301,12",  // testing
		80, "\00301,09",  // resolved
		90, "\00301,15"); // closed
}

static str endpoint()
{
	return fmt( "%1/api/soap/mantisconnect.php", cfg( Name::TrackerURL ));
}

static str soapaction( str name )
{
	return fmt( "%1/api/soap/mantisconnect.php/%2", cfg( Name::TrackerURL ), name );
}

str issueStringRep( ns1__IssueData* data )
{
	return fmt( "[%1] Ticket #%2 (%3): reporter: \002%4\017, "
		"assigned to: \002%5\017, status: \002%6%7\017, resolution: \002%8\017",
		str( data->project->name ),
		str( data->id ),
		str( data->summary ),
		str( data->reporter->name ? data->reporter->name : "???" ),
		( data->handler ) ? str( data->handler->name ) : "None",
		ticketStatusColor( atoi ( data->status->id )),
		str( data->status->name ),
		str( data->resolution->name ));
}

bool ticketinfo( str idstr, str& val )
{
	// Remind me to nuke CMake someday so I can use gSOAP++.
	struct soap* soap = soap_new();
	
	// what a type name, really :p
	struct ns1__mc_USCOREissue_USCOREgetResponse resp;
	
	bool result = false;
	
	// Request the issue data:
	if( soap_call_ns1__mc_USCOREissue_USCOREget (soap,
		SOAP_ENDPOINT,
		SOAP_ACTION( mc_issue_get ),
		TRACKER_ACCOUNT,
		TRACKER_PASSWORD,
		SOAP_STRING( idstr ),
		&resp ) == SOAP_OK )
	{
		val = issueStringRep( resp.return_ );
		result = true;
	}
	else
	{
		const char** err = soap_faultstring( soap );
		val = fmt( "Error retrieving info for ticket %1: %2", idstr, str( *err ) );
		result = false;
	}
	
	soap_end( soap ); 
	soap_free( soap );
	
	return result;
}

str fullticketinfo( str idstr )
{
	struct soap* soap = soap_new();
	struct ns1__mc_USCOREissue_USCOREgetResponse resp;
	
	str val;
	
	// Request the issue data:
	if( soap_call_ns1__mc_USCOREissue_USCOREget (soap,
		SOAP_ENDPOINT,
		SOAP_ACTION( mc_issue_get ),
		TRACKER_ACCOUNT,
		TRACKER_PASSWORD,
		SOAP_STRING( idstr ),
		&resp ) == SOAP_OK )
	{
		ns1__IssueData* ticket = resp.return_;
		val += fmt( "[%1] Ticket #%2: %3\n",
			str( ticket->project->name ),
			str( ticket->id ),
			str( ticket->summary ));
		
		val += fmt( "Reporter: %1, Assigned To: %2\n",
			str( ticket->reporter->name ? ticket->reporter->name : "None" ),
			str( ticket->handler ? ticket->handler->name : "None" ));
		
		val += fmt( "Priority: %1, Severity: %2, Reproducibility: %3\n",
			str( ticket->priority->name ),
			str( ticket->severity->name ),
			str( ticket->reproducibility->name ));
		
		val += fmt( "Status: %1%2\017, resolution: %3\n",
			ticketStatusColor( atoi ( ticket->status->id )),
			str( ticket->status->name ),
			str( ticket->resolution->name ));
		
		val += fmt( "Product Version: %1, Target Version: %2, Fixed in: %3\n",
			str( ticket->version ? ticket->version : "none" ),
			str( ticket->target_USCOREversion ? ticket->target_USCOREversion : "none" ),
			str( ticket->fixed_USCOREin_USCOREversion ? ticket->fixed_USCOREin_USCOREversion : "none" ));
		
		RelationshipDataArray* rels = ticket->relationships;
		if( rels )
		{
			val += "Relationships:\n";
			
			for( int i = 0; i < rels->__size; ++i )
			{
				ns1__RelationshipData* rel = rels->__ptr[i];
				str id = rels->__ptr[i]->target_USCOREid;
				str relname;
				str type = str( rel->type->name );
				bool success = ticketinfo( id, relname );
				
				if( success )
					val += fmt( "%1. %2: %3\n", i + 1, type, relname );
			}
		}
	}
	else
	{
		const char** err = soap_faultstring( soap );
		val = fmt( "Error retrieving info for ticket %1: %2", idstr, str( *err ) );
	}
	
	soap_end( soap ); 
	soap_free( soap );
	
	return val;
}

/*
str ticketsearch( str keywords )
{
	struct soap* soap = soap_new();
	
	ns1__mc_USCOREproject_USCOREget_USCOREissue_USCOREheadersResponse resp;
	str val;
	
	if( soap_call_ns1__mc_USCOREproject_USCOREget_USCOREissue_USCOREheaders (soap,
		SOAP_ENDPOINT,
		SOAP_ACTION( mc_filter_get_issue_headers ),
		TRACKER_ACCOUNT,
		TRACKER_PASSWORD,
		SOAP_STRING( "1 "),
		SOAP_STRING( "0" ),
		SOAP_STRING( "5000" ),
		&resp) == SOAP_OK )
	{
		IssueHeaderDataArray* arr = resp.return_;
		print( "%1 issues\n", arr->__size );
		for( int i = 0; i < arr->__size; ++i )
		{
			ns1__IssueHeaderData* head = arr->__ptr[i];
			print( "%1\n", str( head->id ), str( head->summary ));
		}
	}
	else
		soap_print_fault( soap, stderr );
	
	soap_end( soap ); 
	soap_free( soap );
	
	return val;
}
*/