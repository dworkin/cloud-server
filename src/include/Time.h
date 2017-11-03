# define Time		object "/lib/Time"
# define GMTime		object "/lib/GMTime"

# define SECSPERMIN	60
# define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
# define SECSPERDAY	(SECSPERMIN * MINSPERHOUR * HOURSPERDAY)
# define MINSPERHOUR	60
# define HOURSPERDAY	24
# define DAYSPERWEEK	7
# define DAYSPERYEAR	365
# define ISLEAP(y)	((y) % 4 == 0 && (y) != 2100)
# define LEAPCOUNT(y)	((y) / 4 - (y) / 100 + (y) / 400)
# define JAN1		0
# define FEB1		(JAN1 + 31)
# define MAR1		(FEB1 + 29)
# define APR1		(MAR1 + 31)
# define MAY1		(APR1 + 30)
# define JUN1		(MAY1 + 31)
# define JUL1		(JUN1 + 30)
# define AUG1		(JUL1 + 31)
# define SEP1		(AUG1 + 31)
# define OCT1		(SEP1 + 30)
# define NOV1		(OCT1 + 31)
# define DEC1		(NOV1 + 30)
# define LEAPDAY	(MAR1 - 1)
# define EPOCH_YEAR	1970
# define EPOCH_WEEKDAY	4	/* Thursday */
