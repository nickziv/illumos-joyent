provider lex {
	/* Tracks the lexer's state transitions */
	probe state(int);
	/* Fires when a token-match occures; arg0 is matched token */
	probe match_token(char*);
	/* Fires when a regex-match occures; arg0 is line of matched rgx */
	probe match_regex(int, int);
	/* The current character */
	probe ch(char);
	/* Fires when the lexer backtracks to the fallback state */
	probe fallback_st(int);
	/* Fires when the lexer backtracks to the fallback char */
	probe fallback_ch(char);
	/* Fires when we enter a compressed state */
	probe compr_st();
	/* Fires when the lexer stops to complete a match */
	probe stop();
	/* Fires when the user uses the BEGIN macro */
	probe begin(int, void*);
	/* Fires when the user uses the REJECT macro */
	probe reject();
};
