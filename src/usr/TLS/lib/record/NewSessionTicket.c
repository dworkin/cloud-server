# include "Record.h"
# include "Extension.h"

inherit Data;


private int ticketLifetime;		/* ticket lifetime */
private int ticketAgeAdd;		/* ticket added age */
private string ticketNonce;		/* ticket nonce */
private string ticket;			/* actual ticket */
private Extension *extensions;		/* extensions */

/*
 * initialize NewSessionTicket
 */
static void create(int ticketLifetime, int ticketAgeAdd, string ticketNonce,
		   string ticket, Extensions *extensions)
{
    ::create(HANDSHAKE_NEW_SESSION_TICKET);
    ::ticketLifetime = ticketLifetime;
    ::ticketAgeAdd = ticketAgeAdd;
    ::ticketNonce = ticketNonce;
    ::ticket = ticket;
    ::extensions = extensions;
}

/*
 * export as a blob
 */
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
