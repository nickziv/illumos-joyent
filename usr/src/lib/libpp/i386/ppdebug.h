/*
* preprocessor library debug maps
*/
struct map
{
	char*	nam;
	long	val;
};
static struct map pplexmap[] =
{
	"PROTO", PROTO,
	"RES1", RES1,
	"RES1a", RES1a,
	"RES1e", RES1e,
	"RES1f", RES1f,
	"RES1h", RES1h,
	"RES1l", RES1l,
	"RES1n", RES1n,
	"RES1o", RES1o,
	"RES1t", RES1t,
	"RES1x", RES1x,
	"RES1y", RES1y,
	"COM1", COM1,
	"COM2", COM2,
	"COM3", COM3,
	"COM4", COM4,
	"COM5", COM5,
	"COM6", COM6,
	"COM7", COM7,
	"NID", NID,
	"LIT", LIT,
	"LIT1", LIT1,
	"LIT2", LIT2,
	"BAD1", BAD1,
	"BAD2", BAD2,
	"DOT", DOT,
	"DOT2", DOT2,
	"WS1", WS1,
	"QUICK", QUICK,
	"QTOK", QTOK,
	"QNUM", QNUM,
	"QEXP", QEXP,
	"QCOM", QCOM,
	"QID", QID,
	"MAC0", MAC0,
	"MACN", MACN,
	"HIT0", HIT0,
	"HITN", HITN,
	"LIT0", LIT0,
	"SHARP1", SHARP1,
	"TOKEN", TOKEN,
	"OCT1", OCT1,
	"OCT2", OCT2,
	"OCT3", OCT3,
	"NOT1", NOT1,
	"PCT1", PCT1,
	"AND1", AND1,
	"STAR1", STAR1,
	"PLUS1", PLUS1,
	"MINUS1", MINUS1,
	"ARROW1", ARROW1,
	"COLON1", COLON1,
	"LT1", LT1,
	"LSH1", LSH1,
	"EQ1", EQ1,
	"RSH1", RSH1,
	"GT1", GT1,
	"CIRC1", CIRC1,
	"OR1", OR1,
	"DEC1", DEC1,
	"DEC2", DEC2,
	"HEX1", HEX1,
	"HEX2", HEX2,
	"HEX3", HEX3,
	"HEX4", HEX4,
	"HEX5", HEX5,
	"HEX6", HEX6,
	"HEX7", HEX7,
	"HEX8", HEX8,
	"DBL1", DBL1,
	"DBL2", DBL2,
	"DBL3", DBL3,
	"DBL4", DBL4,
	"DBL5", DBL5,
	"DOT1", DOT1,
	"HDR1", HDR1,
	"BIN1", BIN1,
	"TERMINAL", TERMINAL,
	"S_CHRB", S_CHRB,
	"S_COMMENT", S_COMMENT,
	"S_EOB", S_EOB,
	"S_LITBEG", S_LITBEG,
	"S_LITEND", S_LITEND,
	"S_LITESC", S_LITESC,
	"S_MACRO", S_MACRO,
	"S_NL", S_NL,
	"S_QUAL", S_QUAL,
	"S_SHARP", S_SHARP,
	"S_VS", S_VS,
	"S_CHR", S_CHR,
	"S_HUH", S_HUH,
	"S_TOK", S_TOK,
	"S_TOKB", S_TOKB,
	"S_WS", S_WS,
	"S_RESERVED", S_RESERVED,
};
static struct map ppstatemap[] =
{
	"ADD", ADD,
	"COLLECTING", COLLECTING,
	"COMPATIBILITY", COMPATIBILITY,
	"COMPILE", COMPILE,
	"CONDITIONAL", CONDITIONAL,
	"DEFINITION", DEFINITION,
	"DIRECTIVE", DIRECTIVE,
	"DISABLE", DISABLE,
	"EOF2NL", EOF2NL,
	"ESCAPE", ESCAPE,
	"FILEPOP", FILEPOP,
	"HEADER", HEADER,
	"HIDDEN", HIDDEN,
	"JOINING", JOINING,
	"NEWLINE", NEWLINE,
	"NOEXPAND", NOEXPAND,
	"NOSPACE", NOSPACE,
	"NOTEXT", NOTEXT,
	"NOVERTICAL", NOVERTICAL,
	"PASSEOF", PASSEOF,
	"PASSTHROUGH", PASSTHROUGH,
	"QUOTE", QUOTE,
	"SKIPCONTROL", SKIPCONTROL,
	"SKIPMACRO", SKIPMACRO,
	"SPACEOUT", SPACEOUT,
	"SQUOTE", SQUOTE,
	"STANDALONE", STANDALONE,
	"STRICT", STRICT,
	"STRIP", STRIP,
	"SYNCLINE", SYNCLINE,
	"TRANSITION", TRANSITION,
	"WARN", WARN,
};
static struct map ppmodemap[] =
{
	"ALLMULTIPLE", ALLMULTIPLE,
	"BUILTIN", BUILTIN,
	"CATLITERAL", CATLITERAL,
	"DUMP", DUMP,
	"EXPOSE", EXPOSE,
	"EXTERNALIZE", EXTERNALIZE,
	"FILEDEPS", FILEDEPS,
	"GENDEPS", GENDEPS,
	"HEADERDEPS", HEADERDEPS,
	"HOSTED", HOSTED,
	"HOSTEDTRANSITION", HOSTEDTRANSITION,
	"INACTIVE", INACTIVE,
	"INIT", INIT,
	"LOADING", LOADING,
	"MARKC", MARKC,
	"MARKHOSTED", MARKHOSTED,
	"MARKMACRO", MARKMACRO,
	"PEDANTIC", PEDANTIC,
	"READONLY", READONLY,
	"RELAX", RELAX,
};
static struct map ppoptionmap[] =
{
	"ELSEIF", ELSEIF,
	"FINAL", FINAL,
	"HEADEREXPAND", HEADEREXPAND,
	"HEADEREXPANDALL", HEADEREXPANDALL,
	"IGNORELINE", IGNORELINE,
	"INITIAL", INITIAL,
	"KEEPNOTEXT", KEEPNOTEXT,
	"KEYARGS", KEYARGS,
	"MODERN", MODERN,
	"NATIVE", NATIVE,
	"NOHASH", NOHASH,
	"NOISE", NOISE,
	"NOISEFILTER", NOISEFILTER,
	"NOPROTO", NOPROTO,
	"PLUSCOMMENT", PLUSCOMMENT,
	"PLUSPLUS", PLUSPLUS,
	"PLUSSPLICE", PLUSSPLICE,
	"PRAGMAEXPAND", PRAGMAEXPAND,
	"PREDEFINED", PREDEFINED,
	"PREDEFINITIONS", PREDEFINITIONS,
	"PREFIX", PREFIX,
	"PRESERVE", PRESERVE,
	"PROTOTYPED", PROTOTYPED,
	"REGUARD", REGUARD,
	"SPLICECAT", SPLICECAT,
	"SPLICESPACE", SPLICESPACE,
	"STRINGSPAN", STRINGSPAN,
	"STRINGSPLIT", STRINGSPLIT,
	"TRUNCATE", TRUNCATE,
	"ZEOF", ZEOF,
};
static struct map ppinmap[] =
{
	"BUFFER", IN_BUFFER,
	"COPY", IN_COPY,
	"EXPAND", IN_EXPAND,
	"FILE", IN_FILE,
	"INIT", IN_INIT,
	"MACRO", IN_MACRO,
	"MULTILINE", IN_MULTILINE,
	"QUOTE", IN_QUOTE,
	"RESCAN", IN_RESCAN,
	"SQUOTE", IN_SQUOTE,
	"STRING", IN_STRING,
};