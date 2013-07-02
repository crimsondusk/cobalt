#include "irc.h"
#include "irc_user.h"
#include "irc_channel.h"

// ==============================================================================================
//
// IRCUser methods
//
// ==============================================================================================

// ==============================================================================================
// Determine status level of this user.
IRCChannel::Status IRCUser::chanStatus( IRCChannel* chan )
{
	return chan->effectiveStatus( this );
}

// Check if this user is our administrator
void IRCUser::checkAdmin()
{
	if( user().len() == 0 || host().len() == 0 )
		return;
	
	str uhost = userhost();
	bool admin = false;
	
	for( uint i = 0; i < g_AdminMasks.size() && !admin; i++ )
		if( mask( uhost, g_AdminMasks[i] ) )
			admin = true;
	
	if( admin )
		*this |= Admin;
	else
		*this &= ~Admin;
}

str IRCUser::userhost() const
{
	return fmt( "%1!%2@%3", nick(), user(), host());
}

long IRCUser::operator|( long f ) const
{
	return flags() | f;
}

IRCUser IRCUser::operator|=( long f )
{
	setFlags( flags() | f );
	return *this;
}

long IRCUser::operator&( long f ) const
{
	return flags() & f;
}

IRCUser IRCUser::operator&=( long f )
{
	setFlags( flags() & f );
	return *this;
}

long IRCUser::operator^( long f ) const
{
	return flags() ^ f;
}

IRCUser IRCUser::operator^=( long f )
{
	setFlags( flags() ^ f );
	return *this;
}

IRCUser::operator variant()
{
	str rep = fmt( "%1 (%2@%3)", nick(), user(), host() );
	return Variant( rep );
}

void IRCUser::addKnownChannel( IRCChannel* chan )
{
	m_channels << chan;
}

void IRCUser::delKnownChannel( IRCChannel* chan )
{
	m_channels.remove( chan );
}

bool IRCUser::isAdmin() const
{
	return flags() & Admin;
}