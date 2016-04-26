
/* Declarations */
%{

#include "auxlib.h"
#include "lyutils.h"

#define YY_USER_ACTION  { scanner_useraction (); }


%}

/* Definitions */
%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn


/* reserved words, identifiers, int const, char const, string const */
LETTER          [A-Za-z_]
DIGIT           [0-9]
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
INTCON			({DIGIT}+)
STRINGCON		(\"([^\\"\n]|\\[\\’"0nt])*\")
CHARCON			(’([^\\’\n]|\\[\\’"0nt])’)
INVAL_IDENT		({DIGIT}({LETTER}|{DIGIT})*)

/* Rules */
%%

"#".*           { scanner_include();  }
[ \t]+          { }
\n              { scanner_newline();  }

"[]"			{ return yylval_token(TOK_ARRAY); }
"("				{ return yylval_token('(');       }
")"				{ return yylval_token(')');       }
"["				{ return yylval_token('[');       }
"]"				{ return yylval_token(']');       }
"{"				{ return yylval_token('{');       }
"}"				{ return yylval_token('}');       }
";"				{ return yylval_token(';');       }
","				{ return yylval_token(',');       }
"."				{ return yylval_token('.');       }
"="				{ return yylval_token('=');       }
"=="			{ return yylval_token(TOK_EQ);    }
"!="			{ return yylval_token(TOK_NE);    }
"<"				{ return yylval_token(TOK_LT);    }
"<="			{ return yylval_token(TOK_LE);    }
">"				{ return yylval_token(TOK_GT);    }
">="			{ return yylval_token(TOK_GE);    }
"+"				{ return yylval_token('+');       }
"-"				{ return yylval_token('-');       }
"*"				{ return yylval_token('*');       }
"/"				{ return yylval_token('/');       }
"%"				{ return yylval_token('%');       }
"!"				{ return yylval_token('!');       }


void			{ return yylval_token(TOK_VOID);    }
bool			{ return yylval_token(TOK_BOOL);    }
char			{ return yylval_token(TOK_CHAR);    }
int				{ return yylval_token(TOK_INT);     }
string			{ return yylval_token(TOK_STRING);  }
struct			{ return yylval_token(TOK_STRUCT);  }
if				{ return yylval_token(TOK_IF);      }
else			{ return yylval_token(TOK_ELSE);    }
while			{ return yylval_token(TOK_WHILE);   }
return			{ return yylval_token(TOK_RETURN);  }
false			{ return yylval_token(TOK_FALSE);   }
true			{ return yylval_token(TOK_TRUE);    }
null			{ return yylval_token(TOK_NULL);    }
ord				{ return yylval_token(TOK_ORD);     }
chr				{ return yylval_token(TOK_CHR);     }
new				{ return yylval_token(TOK_NEW);     }

{INVAL_IDENT}	{ scanner_badtoken(yytext); }
.				{ scanner_badchar(*yytext); }

%%