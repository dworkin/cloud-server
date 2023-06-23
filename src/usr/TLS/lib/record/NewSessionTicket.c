# include "Record.h"
# include "Extension.h"

inherit Data;


private int ticketLifetime;
private int ticketAgeAdd;
private string ticketNonce;
private string ticket;
private Extension *extensions;

static void create(int ticketLifetime, int ticketAgeAdd, string ticketNonce,
		   string ticket, Extensions *extensions)
{
    ::create(HANDSHAKE_NEW_SESSION_TICKET);
    ::ticketLifetime = ticketLifetime;
    ::ticketAgeAdd = ticketAgeAdd;
    ::ticketNonce = ticketNonce,
    ::ticket = ticket;
    ::extensions = extensions;
}

string transport()
{
    return int4Save(ticketLifetime) +
	   int4Save(ticketAgeAdd) +
	   len1Save(ticketNonce) +
	   len2Save(ticket) +
	   extSave(extensions);
}


int ticketLifetime()		{ return ticketLifetime; }
int ticketAgeAdd()		{ return ticketAgeAdd; }
string ticketNonce()		{ return ticketNonce; }
string ticket()			{ return ticket; }
Extension *extensions()		{ return extensions; }
