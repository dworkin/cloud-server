# define Continuation		object "/lib/Continuation"
# define ChainedContinuation	object "/lib/ChainedContinuation"
# define DelayedContinuation	object "/lib/DelayedContinuation"
# define IterativeContinuation	object "/lib/IterativeContinuation"
# define DistContinuation	object "/lib/DistContinuation"

# define CONT_OBJS	0	/* object array, object, or chained flag */
# define CONT_DELAY	1	/* delay */
# define CONT_ORIGIN	2	/* originating object */
# define CONT_FUNC	3	/* function to call */
# define CONT_ARGS	4	/* arguments */
# define CONT_VAL	5	/* previous return value */
# define CONT_SIZE	6	/* size of continuation */
