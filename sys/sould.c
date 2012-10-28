private mapping	verbs;

/*
 * NAME:	create()
 * DESCRIPTION:	initialize soul data
 */
static create() {
   string      *verbarr;
   int		i;

   verbarr = ({
      "accuse", "agree", "apologize", "applaud", "bark", "beam",
      "beep", "beg", "bing", "bite", "bleed", "blink", "blush",
      "bop", "bounce", "bow", "breathe", "burn", "burp", "cackle",
      "calm", "caper", "caress", "challenge", "cheer", "choke",
      "chuckle", "clap", "comfort", "confess", "cough", "cower",
      "cringe", "croak", "cry", "cuddle", "curtsey", "dance",
      "daydream", "drool", "elbow", "expect", "explode", "eye",
      "faint", "fart", "fiddle", "fish", "flex", "flip", "flop",
      "fondle", "foo", "forgive", "froth", "frown", "fume",
      "gasp", "gesticulate", "gibber", "giggle", "glare", "greet",
      "grimace", "grin", "grind", "groan", "grope", "grovel",
      "growl", "grumble", "gurgle", "hang", "headbutt", "hiccup",
      "hold", "hop", "hug", "hum", "idle", "ignore", "juggle",
      "jump", "kick", "kiss", "knee", "laugh", "leap", "leer",
      "lick", "loom", "love", "massage", "moan", "mutter",
      "nestle", "nibble", "nod", "nudge", "nuzzle", "panic",
      "pat", "peer", "pet", "pinch", "plead", "point", "poke",
      "ponder", "pout", "puke", "punch", "purr", "push", "puzzle",
      "quirk", "quiver", "raise", "roll", "rub", "ruffle",
      "scratch", "scream", "shake", "shiver", "shrug", "shudder",
      "sigh", "sing", "slap", "sleep", "smile", "smirk", "snap",
      "snarl", "sneer", "sneeze", "snicker", "sniff", "snigger",
      "snore", "snort", "snuggle", "sob", "spank", "spit",
      "squeeze", "stagger", "stamp", "stand", "stare", "stifle",
      "strangle", "stroke", "strut", "stumble", "sulk", "swim",
      "tackle", "tap", "taunt", "tease", "thank", "think",
      "throw", "tickle", "tremble", "trip", "tweak", "twiddle",
      "twitch", "waggle", "waltz", "wave", "whicker", "whimper",
      "whistle", "wiggle", "wimper", "wince", "wink", "wipe",
      "wobble", "wonder", "worship", "yawn", "zip"
   });

   verbs = ([ ]);
   for (i = 0; i < sizeof(verbarr); i ++) {
      string	verb;
      verb = verbarr[i];
      if (verb[strlen(verb)-1 ..] == "s" ||
	  verb[strlen(verb)-1 ..] == "x" ||
	  verb[strlen(verb)-2 ..] == "ch" ||
	  verb[strlen(verb)-2 ..] == "sh") {
	 verbs[verb] = verb + "es";
      } else {
	 verbs[verb] = verb + "s";
      }
   }
}

/*
 * NAME:	query_verb()
 * DESCRIPTION:	return conjugated form of verb, if we handle it
 */
string query_verb(string verb) {
   return verbs[verb];
}
