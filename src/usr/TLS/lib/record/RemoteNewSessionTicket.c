# include <String.h>
# include "Record.h"

inherit NewSessionTicket;


static void create(String blob, int offset, int end)
{
    int ticketLifetime;
    int ticketAgeAdd;
    string ticketNonce;
    string ticket;

    ticketLifetime = int4Restore(blob, offset);
    offset += 4;
    ticketAgeAdd = int4Restore(blob, offset);
    offset += 4;
    ({ ticketNonce, offset }) = len1Restore(blob, offset);
    ({ ticket, offset }) = len2Restore(blob, offset);

    ::create(ticketLifetime, ticketAgeAdd, ticketNonce, ticket,
	     extRestore(blob, offset, end));
}
