# define THING_PFLAGS		0	/* parser flags */
# define THING_NUM		1	/* number or amount */
# define THING_OWNER		2	/* owner's thing */
# define THING_ADJECTIVES	3	/* blue thing */
# define THING_NOUN		4	/* thing */
# define THING_ASPECT		5	/* thing of evil */
# define THING_EXCEPT		6	/* things except for the blue ones */

# define PARSER_SINGULAR	0x01	/* a thing */
# define PARSER_PLURAL		0x02	/* things */
# define PARSER_SPECIFIC	0x04	/* THE thing */
# define PARSER_COUNT		0x08	/* 10 things */
# define PARSER_CARDINAL	0x10	/* the 3rd thing */
# define PARSER_TITLE		0x20	/* thing the foo */
# define PARSER_SINGLE		0x40	/* one */
# define PARSER_MULTIPLE	0x80	/* more than one */


# define MATCH_NONE		0	/* no match */
# define MATCH_AND		1	/* x and y */
# define MATCH_OR		2	/* x or y */
# define MATCH_ERROR		3	/* followed by error string */
