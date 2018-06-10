
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */


#include <stdint.h>
#include "parse_node.h"
#include "parse_malloc.h"
#include "ob_non_reserved_keywords.h"
#include "common/ob_privilege_type.h"
#define YYDEBUG 1



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NAME = 258,
     STRING = 259,
     INTNUM = 260,
     DATE_VALUE = 261,
     HINT_VALUE = 262,
     BOOL = 263,
     APPROXNUM = 264,
     NULLX = 265,
     UNKNOWN = 266,
     QUESTIONMARK = 267,
     SYSTEM_VARIABLE = 268,
     TEMP_VARIABLE = 269,
     EXCEPT = 270,
     UNION = 271,
     INTERSECT = 272,
     OR = 273,
     AND = 274,
     NOT = 275,
     COMP_NE = 276,
     COMP_GE = 277,
     COMP_GT = 278,
     COMP_EQ = 279,
     COMP_LT = 280,
     COMP_LE = 281,
     CNNOP = 282,
     LIKE = 283,
     BETWEEN = 284,
     IN = 285,
     IS = 286,
     MOD = 287,
     UMINUS = 288,
     PROCEDURE = 289,
     DECLARE = 290,
     ELSEIF = 291,
     OUT = 292,
     INOUT = 293,
     WHILE = 294,
     LOOP = 295,
     EXIT = 296,
     CONTINUE = 297,
     DO = 298,
     CALL = 299,
     ARRAY = 300,
     REVERSE = 301,
     CURSOR = 302,
     OPEN = 303,
     FETCH = 304,
     CLOSE = 305,
     NEXT = 306,
     PRIOR = 307,
     FIRST = 308,
     LAST = 309,
     ABSOLUTE = 310,
     RELATIVE = 311,
     ADD = 312,
     ANY = 313,
     ALL = 314,
     ALTER = 315,
     AS = 316,
     ASC = 317,
     BEGI = 318,
     BIGINT = 319,
     BINARY = 320,
     BOOLEAN = 321,
     BOTH = 322,
     BY = 323,
     CASCADE = 324,
     CASE = 325,
     CHARACTER = 326,
     CLUSTER = 327,
     COMMENT = 328,
     COMMIT = 329,
     CONSISTENT = 330,
     COLUMN = 331,
     COLUMNS = 332,
     CREATE = 333,
     CREATETIME = 334,
     CURRENT_USER = 335,
     CHANGE_OBI = 336,
     SWITCH_CLUSTER = 337,
     DATE = 338,
     DATETIME = 339,
     DEALLOCATE = 340,
     DECIMAL = 341,
     DEFAULT = 342,
     DELETE = 343,
     DESC = 344,
     DESCRIBE = 345,
     DISTINCT = 346,
     DOUBLE = 347,
     DROP = 348,
     DUAL = 349,
     TRUNCATE = 350,
     ELSE = 351,
     END = 352,
     END_P = 353,
     ERROR = 354,
     EXECUTE = 355,
     EXISTS = 356,
     EXPLAIN = 357,
     FLOAT = 358,
     FOR = 359,
     FROM = 360,
     FULL = 361,
     FROZEN = 362,
     FORCE = 363,
     GLOBAL = 364,
     GLOBAL_ALIAS = 365,
     GRANT = 366,
     GROUP = 367,
     HAVING = 368,
     HINT_BEGIN = 369,
     HINT_END = 370,
     HOTSPOT = 371,
     IDENTIFIED = 372,
     IF = 373,
     INNER = 374,
     INTEGER = 375,
     INSERT = 376,
     INTO = 377,
     JOIN = 378,
     SEMI_JOIN = 379,
     KEY = 380,
     LEADING = 381,
     LEFT = 382,
     LIMIT = 383,
     LOCAL = 384,
     LOCKED = 385,
     LOCKWJH = 386,
     MEDIUMINT = 387,
     MEMORY = 388,
     MODIFYTIME = 389,
     MASTER = 390,
     NUMERIC = 391,
     OFFSET = 392,
     ON = 393,
     ORDER = 394,
     OPTION = 395,
     OUTER = 396,
     PARAMETERS = 397,
     PASSWORD = 398,
     PRECISION = 399,
     PREPARE = 400,
     PRIMARY = 401,
     READ_STATIC = 402,
     REAL = 403,
     RENAME = 404,
     REPLACE = 405,
     RESTRICT = 406,
     PRIVILEGES = 407,
     REVOKE = 408,
     RIGHT = 409,
     ROLLBACK = 410,
     KILL = 411,
     READ_CONSISTENCY = 412,
     NO_GROUP = 413,
     LONG_TRANS = 414,
     SCHEMA = 415,
     SCOPE = 416,
     SELECT = 417,
     SESSION = 418,
     SESSION_ALIAS = 419,
     SET = 420,
     SHOW = 421,
     SMALLINT = 422,
     SNAPSHOT = 423,
     SPFILE = 424,
     START = 425,
     STATIC = 426,
     SYSTEM = 427,
     STRONG = 428,
     SET_MASTER_CLUSTER = 429,
     SET_SLAVE_CLUSTER = 430,
     SLAVE = 431,
     TABLE = 432,
     TABLES = 433,
     THEN = 434,
     TIME = 435,
     TIMESTAMP = 436,
     TINYINT = 437,
     TRAILING = 438,
     TRANSACTION = 439,
     TO = 440,
     UPDATE = 441,
     USER = 442,
     USING = 443,
     VALUES = 444,
     VARCHAR = 445,
     VARBINARY = 446,
     WHERE = 447,
     WHEN = 448,
     WITH = 449,
     WORK = 450,
     PROCESSLIST = 451,
     QUERY = 452,
     CONNECTION = 453,
     WEAK = 454,
     INDEX = 455,
     STORING = 456,
     BLOOMFILTER_JOIN = 457,
     MERGE_JOIN = 458,
     HASH_JOIN = 459,
     AUTO_INCREMENT = 460,
     CHUNKSERVER = 461,
     COMPRESS_METHOD = 462,
     CONSISTENT_MODE = 463,
     EXPIRE_INFO = 464,
     GRANTS = 465,
     JOIN_INFO = 466,
     MERGESERVER = 467,
     REPLICA_NUM = 468,
     ROOTSERVER = 469,
     ROW_COUNT = 470,
     SERVER = 471,
     SERVER_IP = 472,
     SERVER_PORT = 473,
     SERVER_TYPE = 474,
     STATUS = 475,
     TABLE_ID = 476,
     TABLET_BLOCK_SIZE = 477,
     TABLET_MAX_SIZE = 478,
     UNLOCKED = 479,
     UPDATESERVER = 480,
     USE_BLOOM_FILTER = 481,
     VARIABLES = 482,
     VERBOSE = 483,
     WARNINGS = 484
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{


  struct _ParseNode *node;
  const struct _NonReservedKeyword *non_reserved_keyword;
  int    ival;



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "sql_parser.lex.h"

#define YYLEX_PARAM result->yyscan_info_

extern void yyerror(YYLTYPE* yylloc, ParseResult* p, char* s,...);

extern ParseNode* merge_tree(void *malloc_pool, ObItemType node_tag, ParseNode* source_tree);

extern ParseNode* new_terminal_node(void *malloc_pool, ObItemType type);

extern ParseNode* new_non_terminal_node(void *malloc_pool, ObItemType node_tag, int num, ...);

extern char* copy_expr_string(ParseResult* p, int expr_start, int expr_end);

#define ISSPACE(c) ((c) == ' ' || (c) == '\n' || (c) == '\r' || (c) == '\t' || (c) == '\f' || (c) == '\v')

#define malloc_terminal_node(node, malloc_pool, type) \
do \
{ \
  if ((node = new_terminal_node(malloc_pool, type)) == NULL) \
  { \
    yyerror(NULL, result, "No more space for malloc"); \
    YYABORT; \
  } \
} while(0)

#define malloc_non_terminal_node(node, malloc_pool, node_tag, ...) \
do \
{ \
  if ((node = new_non_terminal_node(malloc_pool, node_tag, ##__VA_ARGS__)) == NULL) \
  { \
    yyerror(NULL, result, "No more space for malloc"); \
    YYABORT; \
  } \
} while(0)

#define merge_nodes(node, malloc_pool, node_tag, source_tree) \
do \
{ \
  if (source_tree == NULL) \
  { \
    node = NULL; \
  } \
  else if ((node = merge_tree(malloc_pool, node_tag, source_tree)) == NULL) \
  { \
    yyerror(NULL, result, "No more space for merging nodes"); \
    YYABORT; \
  } \
} while (0)

#define dup_expr_string(str_ptr, result, expr_start, expr_end) \
  do \
  { \
    str_ptr = NULL; \
    int start = expr_start; \
    while (start <= expr_end && ISSPACE(result->input_sql_[start - 1])) \
      start++; \
    if (start >= expr_start \
      && (str_ptr = copy_expr_string(result, start, expr_end)) == NULL) \
    { \
      yyerror(NULL, result, "No more space for copying expression string"); \
      YYABORT; \
    } \
  } while (0)




#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  204
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3804

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  241
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  221
/* YYNRULES -- Number of rules.  */
#define YYNRULES  670
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1264

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   484

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    36,     2,     2,
      40,    41,    34,    32,   240,    33,    42,    35,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   239,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    38,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    37,    39,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,    10,    12,    14,    16,    18,    20,
      22,    24,    26,    28,    30,    32,    34,    36,    38,    40,
      42,    44,    46,    48,    50,    52,    54,    56,    58,    60,
      62,    64,    66,    68,    70,    72,    74,    76,    78,    80,
      82,    84,    86,    88,    90,    92,    94,    96,    98,    99,
     101,   105,   107,   111,   115,   117,   119,   121,   123,   125,
     127,   129,   131,   133,   137,   139,   141,   145,   151,   153,
     155,   157,   159,   162,   164,   166,   169,   172,   176,   180,
     184,   188,   192,   196,   200,   202,   205,   208,   212,   216,
     220,   224,   228,   232,   236,   240,   244,   248,   252,   256,
     260,   264,   269,   273,   277,   280,   284,   289,   293,   298,
     302,   307,   313,   320,   324,   329,   333,   335,   339,   345,
     347,   348,   350,   353,   358,   361,   362,   367,   372,   377,
     383,   388,   395,   400,   404,   409,   411,   413,   415,   417,
     419,   421,   423,   429,   437,   439,   443,   447,   457,   461,
     464,   465,   469,   471,   475,   476,   478,   484,   486,   489,
     492,   496,   501,   507,   513,   519,   525,   532,   540,   544,
     548,   552,   558,   560,   562,   567,   574,   577,   586,   590,
     591,   593,   597,   599,   605,   609,   611,   613,   615,   617,
     619,   622,   625,   627,   630,   632,   635,   638,   640,   643,
     646,   649,   652,   654,   656,   658,   661,   667,   671,   672,
     676,   677,   679,   680,   684,   685,   689,   690,   693,   694,
     697,   699,   702,   704,   707,   709,   713,   714,   718,   722,
     726,   730,   734,   738,   742,   746,   750,   754,   756,   757,
     762,   763,   766,   768,   772,   778,   785,   786,   788,   792,
     794,   802,   807,   815,   816,   819,   821,   823,   827,   828,
     830,   834,   838,   844,   846,   850,   853,   855,   859,   863,
     865,   868,   872,   877,   883,   892,   894,   896,   906,   916,
     921,   926,   931,   932,   935,   939,   944,   949,   952,   955,
     960,   961,   965,   967,   971,   972,   974,   977,   979,   981,
     996,  1001,  1007,  1012,  1016,  1021,  1023,  1025,  1028,  1029,
    1031,  1035,  1037,  1039,  1041,  1043,  1045,  1047,  1049,  1051,
    1053,  1054,  1056,  1057,  1060,  1064,  1069,  1074,  1079,  1083,
    1087,  1091,  1092,  1096,  1098,  1099,  1103,  1105,  1109,  1112,
    1113,  1115,  1117,  1118,  1121,  1122,  1124,  1126,  1128,  1131,
    1135,  1137,  1139,  1143,  1145,  1149,  1151,  1155,  1158,  1162,
    1165,  1167,  1173,  1175,  1179,  1186,  1192,  1195,  1198,  1201,
    1203,  1205,  1206,  1210,  1212,  1214,  1216,  1218,  1220,  1221,
    1225,  1231,  1237,  1243,  1248,  1253,  1258,  1261,  1266,  1270,
    1274,  1278,  1282,  1286,  1290,  1294,  1298,  1303,  1306,  1307,
    1309,  1312,  1317,  1319,  1321,  1322,  1323,  1326,  1329,  1330,
    1332,  1333,  1335,  1339,  1341,  1345,  1350,  1352,  1354,  1358,
    1360,  1364,  1370,  1377,  1380,  1381,  1385,  1389,  1391,  1395,
    1400,  1402,  1404,  1406,  1407,  1411,  1412,  1415,  1419,  1422,
    1425,  1430,  1431,  1433,  1434,  1436,  1438,  1445,  1447,  1451,
    1454,  1456,  1458,  1461,  1463,  1465,  1467,  1470,  1472,  1474,
    1476,  1478,  1480,  1481,  1483,  1485,  1491,  1494,  1495,  1500,
    1502,  1504,  1506,  1508,  1510,  1513,  1517,  1519,  1523,  1527,
    1531,  1536,  1541,  1547,  1553,  1557,  1561,  1565,  1567,  1569,
    1571,  1573,  1577,  1579,  1583,  1587,  1590,  1591,  1593,  1597,
    1601,  1603,  1605,  1610,  1617,  1619,  1623,  1627,  1632,  1637,
    1643,  1645,  1646,  1648,  1650,  1651,  1655,  1659,  1663,  1666,
    1671,  1679,  1687,  1694,  1701,  1702,  1704,  1706,  1710,  1720,
    1723,  1724,  1728,  1732,  1736,  1737,  1739,  1741,  1743,  1745,
    1749,  1756,  1757,  1759,  1761,  1763,  1765,  1767,  1769,  1771,
    1773,  1775,  1777,  1779,  1781,  1783,  1785,  1787,  1789,  1791,
    1793,  1795,  1797,  1799,  1801,  1803,  1805,  1807,  1809,  1811,
    1813,  1815,  1817,  1819,  1821,  1824,  1831,  1837,  1841,  1843,
    1846,  1850,  1854,  1858,  1862,  1867,  1872,  1877,  1881,  1882,
    1884,  1887,  1889,  1891,  1893,  1895,  1897,  1899,  1901,  1904,
    1907,  1910,  1913,  1916,  1919,  1922,  1925,  1928,  1931,  1934,
    1937,  1940,  1943,  1946,  1948,  1950,  1951,  1953,  1956,  1958,
    1960,  1962,  1964,  1966,  1968,  1971,  1974,  1977,  1980,  1983,
    1986,  1989,  1992,  1995,  1998,  2001,  2004,  2007,  2010,  2013,
    2015,  2017,  2022,  2029,  2035,  2039,  2049,  2058,  2067,  2075,
    2077,  2080,  2085,  2088,  2096,  2099,  2101,  2106,  2107,  2110,
    2116,  2128,  2141,  2149,  2151,  2154,  2159,  2166,  2172,  2178,
    2182
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     242,     0,    -1,   243,   107,    -1,   243,   239,   244,    -1,
     244,    -1,   434,    -1,   459,    -1,   461,    -1,   460,    -1,
     273,    -1,   275,    -1,   276,    -1,   291,    -1,   284,    -1,
     285,    -1,   286,    -1,   287,    -1,   289,    -1,   290,    -1,
     322,    -1,   315,    -1,   292,    -1,   267,    -1,   264,    -1,
     263,    -1,   308,    -1,   312,    -1,   359,    -1,   362,    -1,
     399,    -1,   402,    -1,   410,    -1,   415,    -1,   421,    -1,
     413,    -1,   370,    -1,   375,    -1,   377,    -1,   379,    -1,
     382,    -1,   392,    -1,   397,    -1,   386,    -1,   387,    -1,
     388,    -1,   389,    -1,   363,    -1,   311,    -1,    -1,   250,
      -1,   245,   240,   250,    -1,   429,    -1,   430,    42,   429,
      -1,   430,    42,    34,    -1,     4,    -1,     6,    -1,     5,
      -1,     9,    -1,     8,    -1,    10,    -1,    12,    -1,    14,
      -1,    13,    -1,   173,    42,   429,    -1,   246,    -1,   247,
      -1,    40,   250,    41,    -1,    40,   245,   240,   250,    41,
      -1,   252,    -1,   258,    -1,   259,    -1,   323,    -1,   110,
     323,    -1,   257,    -1,   248,    -1,    32,   249,    -1,    33,
     249,    -1,   249,    32,   249,    -1,   249,    33,   249,    -1,
     249,    34,   249,    -1,   249,    35,   249,    -1,   249,    36,
     249,    -1,   249,    38,   249,    -1,   249,    37,   249,    -1,
     248,    -1,    32,   250,    -1,    33,   250,    -1,   250,    32,
     250,    -1,   250,    33,   250,    -1,   250,    34,   250,    -1,
     250,    35,   250,    -1,   250,    36,   250,    -1,   250,    38,
     250,    -1,   250,    37,   250,    -1,   250,    26,   250,    -1,
     250,    25,   250,    -1,   250,    24,   250,    -1,   250,    22,
     250,    -1,   250,    23,   250,    -1,   250,    21,   250,    -1,
     250,    28,   250,    -1,   250,    20,    28,   250,    -1,   250,
      19,   250,    -1,   250,    18,   250,    -1,    20,   250,    -1,
     250,    31,    10,    -1,   250,    31,    20,    10,    -1,   250,
      31,     8,    -1,   250,    31,    20,     8,    -1,   250,    31,
      11,    -1,   250,    31,    20,    11,    -1,   250,    29,   249,
      19,   249,    -1,   250,    20,    29,   249,    19,   249,    -1,
     250,    30,   251,    -1,   250,    20,    30,   251,    -1,   250,
      27,   250,    -1,   323,    -1,    40,   245,    41,    -1,    79,
     253,   254,   256,   106,    -1,   250,    -1,    -1,   255,    -1,
     254,   255,    -1,   202,   250,   188,   250,    -1,   105,   250,
      -1,    -1,    14,    40,     5,    41,    -1,    14,    40,    14,
      41,    -1,   431,    40,    34,    41,    -1,   431,    40,   262,
     250,    41,    -1,   431,    40,   245,    41,    -1,   431,    40,
     250,    70,   297,    41,    -1,   431,    40,   342,    41,    -1,
     431,    40,    41,    -1,   260,    40,   261,    41,    -1,   224,
      -1,   322,    -1,   315,    -1,   264,    -1,   263,    -1,    68,
      -1,   100,    -1,    97,   114,   355,   329,   316,    -1,   195,
     331,   355,   174,   265,   329,   316,    -1,   266,    -1,   265,
     240,   266,    -1,   429,    24,   250,    -1,    87,   209,   293,
     355,   147,   355,   268,   269,   271,    -1,    40,   319,    41,
      -1,   210,   270,    -1,    -1,    40,   319,    41,    -1,   272,
      -1,   271,   240,   272,    -1,    -1,   306,    -1,    44,   274,
      56,   113,   322,    -1,   432,    -1,    57,   274,    -1,    58,
     274,    -1,    58,   274,    60,    -1,    58,   274,   131,   412,
      -1,    58,   274,    60,   131,   412,    -1,    58,   274,    62,
     131,   412,    -1,    58,   274,    61,   131,   412,    -1,    58,
     274,    63,   131,   412,    -1,    58,   274,    64,     5,   131,
     412,    -1,    58,   274,   288,    65,     5,   131,   412,    -1,
      58,   274,    61,    -1,    58,   274,    62,    -1,    58,   274,
      63,    -1,    58,   274,   288,    65,     5,    -1,    60,    -1,
      61,    -1,    58,   274,    64,     5,    -1,    58,   274,   114,
       5,   194,     5,    -1,    59,   274,    -1,    87,   186,   293,
     355,    40,   294,    41,   305,    -1,   127,    20,   110,    -1,
      -1,   295,    -1,   294,   240,   295,    -1,   296,    -1,   155,
     134,    40,   319,    41,    -1,   429,   297,   303,    -1,   191,
      -1,   176,    -1,   141,    -1,   129,    -1,    73,    -1,    95,
     298,    -1,   145,   298,    -1,    75,    -1,   112,   299,    -1,
     157,    -1,   101,   300,    -1,   190,   301,    -1,    93,    -1,
      80,   302,    -1,    74,   302,    -1,   199,   302,    -1,   200,
     302,    -1,    88,    -1,   143,    -1,    92,    -1,   189,   301,
      -1,    40,     5,   240,     5,    41,    -1,    40,     5,    41,
      -1,    -1,    40,     5,    41,    -1,    -1,   153,    -1,    -1,
      40,     5,    41,    -1,    -1,    40,     5,    41,    -1,    -1,
     303,   304,    -1,    -1,    20,    10,    -1,    10,    -1,    96,
     247,    -1,   214,    -1,   155,   134,    -1,   306,    -1,   305,
     240,   306,    -1,    -1,   220,   307,     4,    -1,   218,   307,
       4,    -1,   232,   307,     5,    -1,   231,   307,     5,    -1,
     230,   307,     5,    -1,   222,   307,     5,    -1,   216,   307,
       4,    -1,   235,   307,     8,    -1,   217,   307,   180,    -1,
      82,   307,     4,    -1,    24,    -1,    -1,   102,   186,   309,
     310,    -1,    -1,   127,   110,    -1,   354,    -1,   310,   240,
     354,    -1,   104,   186,   309,   310,   425,    -1,   102,   209,
     309,   313,   147,   314,    -1,    -1,   355,    -1,   313,   240,
     355,    -1,   355,    -1,   317,   131,   355,   318,   198,   320,
     316,    -1,   317,   131,   355,   322,    -1,   317,   131,   355,
      40,   319,    41,   322,    -1,    -1,   202,   250,    -1,   159,
      -1,   130,    -1,    40,   319,    41,    -1,    -1,   429,    -1,
     319,   240,   429,    -1,    40,   321,    41,    -1,   320,   240,
      40,   321,    41,    -1,   250,    -1,   321,   240,   250,    -1,
     324,   316,    -1,   323,    -1,    40,   324,    41,    -1,    40,
     323,    41,    -1,   325,    -1,   328,   341,    -1,   326,   345,
     341,    -1,   326,   344,   330,   341,    -1,   171,   331,   350,
     352,   340,    -1,   171,   331,   350,   352,   114,   103,   329,
     340,    -1,   328,    -1,   323,    -1,   171,   331,   352,   131,
     412,   114,   353,   329,   341,    -1,   171,   331,   350,   352,
     114,   353,   329,   343,   349,    -1,   326,    16,   350,   326,
      -1,   326,    17,   350,   326,    -1,   326,    15,   350,   326,
      -1,    -1,   201,   250,    -1,   201,     7,   250,    -1,   137,
     339,   146,   339,    -1,   146,   339,   137,   339,    -1,   137,
     339,    -1,   146,   339,    -1,   137,   339,   240,   339,    -1,
      -1,   123,   332,   124,    -1,   333,    -1,   332,   240,   333,
      -1,    -1,   334,    -1,   333,   334,    -1,   156,    -1,   125,
      -1,   133,    40,   355,   240,   355,   240,   355,    42,   355,
     240,   355,    42,   355,    41,    -1,   166,    40,   338,    41,
      -1,   209,    40,   355,   355,    41,    -1,     3,    40,     3,
      41,    -1,    40,   335,    41,    -1,   132,    40,   336,    41,
      -1,   167,    -1,   168,    -1,   335,   240,    -1,    -1,   337,
      -1,   336,   240,   337,    -1,   211,    -1,   212,    -1,   213,
      -1,   208,    -1,   182,    -1,   180,    -1,   116,    -1,     5,
      -1,    12,    -1,    -1,   330,    -1,    -1,   113,   195,    -1,
     250,   114,   250,    -1,    76,   250,   114,   250,    -1,   135,
     250,   114,   250,    -1,   192,   250,   114,   250,    -1,    76,
     114,   250,    -1,   135,   114,   250,    -1,   192,   114,   250,
      -1,    -1,   121,    77,   245,    -1,   345,    -1,    -1,   148,
      77,   346,    -1,   347,    -1,   346,   240,   347,    -1,   250,
     348,    -1,    -1,    71,    -1,    98,    -1,    -1,   122,   250,
      -1,    -1,    68,    -1,   100,    -1,   250,    -1,   250,   432,
      -1,   250,    70,   432,    -1,    34,    -1,   351,    -1,   352,
     240,   351,    -1,   354,    -1,   353,   240,   354,    -1,   355,
      -1,   355,    70,   430,    -1,   355,   430,    -1,   323,    70,
     430,    -1,   323,   430,    -1,   356,    -1,    40,   356,    41,
      70,   430,    -1,   430,    -1,    40,   356,    41,    -1,   354,
     357,   132,   354,   147,   250,    -1,   354,   132,   354,   147,
     250,    -1,   115,   358,    -1,   136,   358,    -1,   163,   358,
      -1,   128,    -1,   150,    -1,    -1,   111,   361,   360,    -1,
     322,    -1,   263,    -1,   315,    -1,   264,    -1,   237,    -1,
      -1,   175,   187,   367,    -1,   175,   209,   147,   355,   367,
      -1,   175,    86,   114,   355,   367,    -1,   175,    86,    30,
     355,   367,    -1,   175,   186,   229,   367,    -1,   175,   225,
     229,   367,    -1,   175,   366,   236,   367,    -1,   175,   169,
      -1,   175,    87,   186,   355,    -1,    99,   355,   368,    -1,
      98,   355,   368,    -1,   175,   238,   364,    -1,   175,   258,
     238,    -1,   175,   219,   365,    -1,   175,   151,   367,    -1,
     175,   369,   205,    -1,   140,   186,   355,    -1,   137,     5,
     240,     5,    -1,   137,     5,    -1,    -1,   378,    -1,   113,
      89,    -1,   113,    89,    40,    41,    -1,   118,    -1,   172,
      -1,    -1,    -1,    28,     4,    -1,   201,   250,    -1,    -1,
       4,    -1,    -1,   115,    -1,    87,   196,   371,    -1,   372,
      -1,   371,   240,   372,    -1,   373,   126,    77,   374,    -1,
       4,    -1,     4,    -1,   102,   196,   376,    -1,   373,    -1,
     376,   240,   373,    -1,   174,   152,   378,    24,   374,    -1,
      69,   196,   373,   126,    77,   374,    -1,   113,   373,    -1,
      -1,   158,   196,   381,    -1,   373,   194,   373,    -1,   380,
      -1,   381,   240,   380,    -1,    69,   196,   373,   383,    -1,
     139,    -1,   233,    -1,   204,    -1,    -1,   203,    84,   177,
      -1,    -1,    72,   384,    -1,   179,   193,   385,    -1,    83,
     384,    -1,   164,   384,    -1,   165,   390,   391,     5,    -1,
      -1,   118,    -1,    -1,   206,    -1,   207,    -1,   120,   393,
     147,   396,   194,   376,    -1,   394,    -1,   393,   240,   394,
      -1,    68,   395,    -1,    69,    -1,    87,    -1,    87,   196,
      -1,    97,    -1,   102,    -1,   104,    -1,   120,   149,    -1,
     130,    -1,   195,    -1,   171,    -1,   159,    -1,   161,    -1,
      -1,    34,    -1,   430,    -1,   162,   393,   398,   114,   376,
      -1,   147,   396,    -1,    -1,   154,   400,   114,   401,    -1,
     432,    -1,   322,    -1,   315,    -1,   264,    -1,   263,    -1,
     174,   403,    -1,   174,   174,   405,    -1,   404,    -1,   403,
     240,   404,    -1,    14,   406,   250,    -1,   429,   406,   250,
      -1,   118,   429,   406,   250,    -1,   172,   429,   406,   250,
      -1,   119,    42,   429,   406,   250,    -1,   173,    42,   429,
     406,   250,    -1,    13,   406,   250,    -1,   257,   406,   250,
      -1,    14,   406,   408,    -1,   194,    -1,    24,    -1,    14,
      -1,   257,    -1,    40,   409,    41,    -1,   247,    -1,   409,
     240,   247,    -1,   109,   400,   411,    -1,   197,   412,    -1,
      -1,   407,    -1,   412,   240,   407,    -1,   414,   154,   400,
      -1,    94,    -1,   102,    -1,    69,   186,   355,   416,    -1,
      69,   186,   355,   158,   194,   355,    -1,   417,    -1,   416,
     240,   417,    -1,    66,   418,   296,    -1,   102,   418,   429,
     419,    -1,    69,   418,   429,   420,    -1,   158,   418,   429,
     194,   432,    -1,    85,    -1,    -1,    78,    -1,   160,    -1,
      -1,   174,    20,    10,    -1,   102,    20,    10,    -1,   174,
      96,   247,    -1,   102,    96,    -1,    69,   181,   174,   423,
      -1,    69,   181,   422,    90,   144,    24,     4,    -1,    69,
     181,   422,    91,   144,    24,     4,    -1,    69,   181,   183,
     144,    24,     4,    -1,    69,   181,   184,   185,    24,     4,
      -1,    -1,   117,    -1,   424,    -1,   423,   240,   424,    -1,
     429,    24,   247,   425,   426,   228,    24,   427,   428,    -1,
      82,     4,    -1,    -1,   170,    24,   142,    -1,   170,    24,
     178,    -1,   170,    24,    76,    -1,    -1,   223,    -1,   234,
      -1,   215,    -1,   221,    -1,    81,    24,     5,    -1,   226,
      24,     4,   227,    24,     5,    -1,    -1,     3,    -1,   433,
      -1,     3,    -1,   433,    -1,     3,    -1,     3,    -1,   433,
      -1,   214,    -1,   215,    -1,   216,    -1,   217,    -1,   218,
      -1,   219,    -1,   220,    -1,   221,    -1,   222,    -1,   223,
      -1,   224,    -1,   225,    -1,   226,    -1,   227,    -1,   228,
      -1,   229,    -1,   231,    -1,   230,    -1,   232,    -1,   233,
      -1,   234,    -1,   235,    -1,   236,    -1,   237,    -1,   238,
      -1,   435,   438,    -1,    87,    43,     3,    40,   436,    41,
      -1,    87,    43,     3,    40,    41,    -1,   436,   240,   437,
      -1,   437,    -1,    14,   297,    -1,    30,    14,   297,    -1,
      46,    14,   297,    -1,    47,    14,   297,    -1,    14,   297,
      54,    -1,    30,    14,   297,    54,    -1,    46,    14,   297,
      54,    -1,    47,    14,   297,    54,    -1,    72,   439,   106,
      -1,    -1,   440,    -1,   440,   441,    -1,   441,    -1,   447,
      -1,   451,    -1,   456,    -1,   455,    -1,   445,    -1,   446,
      -1,   327,   239,    -1,   315,   239,    -1,   264,   239,    -1,
     263,   239,    -1,   322,   239,    -1,   273,   239,    -1,   275,
     239,    -1,   291,   239,    -1,   277,   239,    -1,   278,   239,
      -1,   279,   239,    -1,   281,   239,    -1,   280,   239,    -1,
     282,   239,    -1,   283,   239,    -1,   458,    -1,   457,    -1,
      -1,   443,    -1,   443,   444,    -1,   444,    -1,   447,    -1,
     451,    -1,   456,    -1,   455,    -1,   446,    -1,   327,   239,
      -1,   315,   239,    -1,   264,   239,    -1,   263,   239,    -1,
     322,   239,    -1,   273,   239,    -1,   275,   239,    -1,   291,
     239,    -1,   277,   239,    -1,   278,   239,    -1,   279,   239,
      -1,   281,   239,    -1,   280,   239,    -1,   282,   239,    -1,
     283,   239,    -1,   458,    -1,   457,    -1,    44,   412,   297,
     239,    -1,    44,   412,   297,    96,   247,   239,    -1,    44,
     412,   297,    54,   239,    -1,   174,   403,   239,    -1,   127,
     250,   188,   442,   448,   450,   106,   127,   239,    -1,   127,
     250,   188,   442,   448,   106,   127,   239,    -1,   127,   250,
     188,   442,   450,   106,   127,   239,    -1,   127,   250,   188,
     442,   106,   127,   239,    -1,   449,    -1,   448,   449,    -1,
      45,   250,   188,   442,    -1,   105,   442,    -1,    79,   250,
     452,   454,   106,    79,   239,    -1,   452,   453,    -1,   453,
      -1,   202,   250,   188,   442,    -1,    -1,   105,   442,    -1,
      49,   442,   106,    49,   239,    -1,   113,    14,    30,   249,
     194,   249,    49,   442,   106,    49,   239,    -1,   113,    14,
      30,    55,   249,   194,   249,    49,   442,   106,    49,   239,
      -1,    48,   250,    52,   442,   106,    48,   239,    -1,   239,
      -1,    50,   239,    -1,    50,   202,   250,   239,    -1,    53,
       3,    40,   245,    41,   331,    -1,    53,     3,    40,    41,
     331,    -1,   102,    43,   127,   110,     3,    -1,   102,    43,
       3,    -1,   175,    43,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   261,   261,   270,   277,   284,   285,   286,   287,   289,
     290,   291,   292,   293,   295,   296,   297,   298,   299,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   330,   332,   342,
     346,   353,   355,   361,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   383,   385,   387,   389,   395,   403,
     407,   411,   415,   419,   427,   428,   432,   436,   437,   438,
     439,   440,   441,   442,   445,   446,   450,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   473,   477,   481,   485,   489,   493,   497,
     501,   505,   509,   513,   517,   521,   528,   532,   537,   545,
     546,   550,   552,   557,   564,   565,   570,   574,   582,   596,
     624,   699,   722,   726,   748,   756,   763,   764,   765,   766,
     770,   774,   788,   802,   812,   816,   823,   837,   853,   860,
     866,   872,   879,   883,   888,   894,   911,   922,   935,   951,
     957,   971,   986,  1001,  1017,  1033,  1049,  1066,  1082,  1103,
    1122,  1139,  1152,  1157,  1172,  1189,  1209,  1229,  1246,  1249,
    1253,  1257,  1264,  1268,  1277,  1286,  1288,  1290,  1292,  1294,
    1296,  1308,  1317,  1319,  1321,  1323,  1328,  1335,  1337,  1344,
    1351,  1358,  1365,  1367,  1369,  1375,  1387,  1389,  1392,  1396,
    1397,  1401,  1402,  1406,  1407,  1411,  1412,  1416,  1419,  1423,
    1428,  1433,  1435,  1437,  1442,  1446,  1451,  1457,  1462,  1467,
    1472,  1477,  1482,  1487,  1492,  1497,  1503,  1511,  1512,  1523,
    1533,  1534,  1539,  1543,  1555,  1572,  1582,  1584,  1588,  1595,
    1606,  1620,  1631,  1649,  1650,  1657,  1662,  1670,  1675,  1679,
    1680,  1687,  1691,  1697,  1698,  1712,  1722,  1727,  1728,  1732,
    1736,  1741,  1751,  1772,  1794,  1820,  1821,  1829,  1861,  1887,
    1909,  1931,  1957,  1958,  1962,  1969,  1977,  1985,  1989,  1993,
    2005,  2008,  2022,  2026,  2031,  2037,  2041,  2048,  2052,  2056,
    2060,  2066,  2072,  2079,  2084,  2089,  2093,  2100,  2105,  2112,
    2116,  2123,  2127,  2132,  2140,  2144,  2148,  2152,  2158,  2160,
    2166,  2167,  2173,  2174,  2182,  2189,  2196,  2203,  2210,  2221,
    2232,  2247,  2248,  2255,  2256,  2260,  2267,  2269,  2274,  2282,
    2283,  2285,  2291,  2292,  2300,  2303,  2307,  2314,  2319,  2327,
    2335,  2345,  2349,  2356,  2358,  2363,  2367,  2371,  2375,  2379,
    2383,  2387,  2396,  2404,  2408,  2412,  2421,  2427,  2433,  2439,
    2446,  2447,  2457,  2465,  2466,  2467,  2468,  2472,  2473,  2484,
    2486,  2488,  2490,  2492,  2494,  2496,  2501,  2503,  2505,  2507,
    2509,  2513,  2526,  2530,  2534,  2541,  2547,  2556,  2566,  2570,
    2572,  2574,  2579,  2580,  2581,  2586,  2587,  2589,  2595,  2596,
    2601,  2602,  2612,  2618,  2622,  2628,  2634,  2640,  2652,  2658,
    2662,  2674,  2678,  2684,  2689,  2700,  2706,  2712,  2716,  2728,
    2734,  2739,  2753,  2758,  2762,  2767,  2771,  2777,  2789,  2801,
    2813,  2820,  2824,  2832,  2836,  2841,  2855,  2866,  2870,  2876,
    2882,  2887,  2892,  2897,  2902,  2908,  2913,  2918,  2923,  2928,
    2933,  2940,  2945,  2950,  2955,  2967,  3007,  3012,  3024,  3031,
    3036,  3038,  3040,  3042,  3055,  3061,  3069,  3073,  3080,  3086,
    3093,  3100,  3107,  3114,  3121,  3128,  3139,  3149,  3150,  3154,
    3157,  3163,  3170,  3171,  3186,  3193,  3198,  3203,  3207,  3220,
    3228,  3230,  3241,  3247,  3258,  3262,  3269,  3274,  3280,  3285,
    3294,  3295,  3299,  3300,  3301,  3305,  3310,  3315,  3319,  3333,
    3339,  3346,  3353,  3360,  3370,  3373,  3381,  3385,  3392,  3407,
    3410,  3414,  3416,  3418,  3421,  3425,  3430,  3435,  3440,  3448,
    3452,  3457,  3468,  3470,  3487,  3489,  3506,  3510,  3512,  3525,
    3526,  3527,  3528,  3529,  3530,  3531,  3532,  3533,  3534,  3535,
    3536,  3537,  3538,  3539,  3540,  3541,  3542,  3543,  3544,  3545,
    3546,  3547,  3548,  3549,  3559,  3564,  3571,  3577,  3581,  3586,
    3590,  3594,  3598,  3602,  3609,  3616,  3623,  3631,  3638,  3641,
    3646,  3650,  3655,  3657,  3659,  3661,  3663,  3665,  3667,  3669,
    3671,  3673,  3675,  3677,  3679,  3681,  3683,  3685,  3687,  3689,
    3691,  3693,  3695,  3698,  3700,  3707,  3710,  3715,  3719,  3724,
    3726,  3728,  3730,  3732,  3734,  3736,  3738,  3740,  3742,  3744,
    3746,  3748,  3750,  3752,  3754,  3756,  3758,  3760,  3762,  3765,
    3767,  3775,  3781,  3787,  3807,  3819,  3825,  3831,  3835,  3840,
    3844,  3849,  3855,  3866,  3873,  3877,  3882,  3888,  3891,  3902,
    3911,  3920,  3938,  3944,  3954,  3964,  3975,  3983,  3996,  4000,
    4014
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NAME", "STRING", "INTNUM", "DATE_VALUE",
  "HINT_VALUE", "BOOL", "APPROXNUM", "NULLX", "UNKNOWN", "QUESTIONMARK",
  "SYSTEM_VARIABLE", "TEMP_VARIABLE", "EXCEPT", "UNION", "INTERSECT", "OR",
  "AND", "NOT", "COMP_NE", "COMP_GE", "COMP_GT", "COMP_EQ", "COMP_LT",
  "COMP_LE", "CNNOP", "LIKE", "BETWEEN", "IN", "IS", "'+'", "'-'", "'*'",
  "'/'", "'%'", "MOD", "'^'", "UMINUS", "'('", "')'", "'.'", "PROCEDURE",
  "DECLARE", "ELSEIF", "OUT", "INOUT", "WHILE", "LOOP", "EXIT", "CONTINUE",
  "DO", "CALL", "ARRAY", "REVERSE", "CURSOR", "OPEN", "FETCH", "CLOSE",
  "NEXT", "PRIOR", "FIRST", "LAST", "ABSOLUTE", "RELATIVE", "ADD", "ANY",
  "ALL", "ALTER", "AS", "ASC", "BEGI", "BIGINT", "BINARY", "BOOLEAN",
  "BOTH", "BY", "CASCADE", "CASE", "CHARACTER", "CLUSTER", "COMMENT",
  "COMMIT", "CONSISTENT", "COLUMN", "COLUMNS", "CREATE", "CREATETIME",
  "CURRENT_USER", "CHANGE_OBI", "SWITCH_CLUSTER", "DATE", "DATETIME",
  "DEALLOCATE", "DECIMAL", "DEFAULT", "DELETE", "DESC", "DESCRIBE",
  "DISTINCT", "DOUBLE", "DROP", "DUAL", "TRUNCATE", "ELSE", "END", "END_P",
  "ERROR", "EXECUTE", "EXISTS", "EXPLAIN", "FLOAT", "FOR", "FROM", "FULL",
  "FROZEN", "FORCE", "GLOBAL", "GLOBAL_ALIAS", "GRANT", "GROUP", "HAVING",
  "HINT_BEGIN", "HINT_END", "HOTSPOT", "IDENTIFIED", "IF", "INNER",
  "INTEGER", "INSERT", "INTO", "JOIN", "SEMI_JOIN", "KEY", "LEADING",
  "LEFT", "LIMIT", "LOCAL", "LOCKED", "LOCKWJH", "MEDIUMINT", "MEMORY",
  "MODIFYTIME", "MASTER", "NUMERIC", "OFFSET", "ON", "ORDER", "OPTION",
  "OUTER", "PARAMETERS", "PASSWORD", "PRECISION", "PREPARE", "PRIMARY",
  "READ_STATIC", "REAL", "RENAME", "REPLACE", "RESTRICT", "PRIVILEGES",
  "REVOKE", "RIGHT", "ROLLBACK", "KILL", "READ_CONSISTENCY", "NO_GROUP",
  "LONG_TRANS", "SCHEMA", "SCOPE", "SELECT", "SESSION", "SESSION_ALIAS",
  "SET", "SHOW", "SMALLINT", "SNAPSHOT", "SPFILE", "START", "STATIC",
  "SYSTEM", "STRONG", "SET_MASTER_CLUSTER", "SET_SLAVE_CLUSTER", "SLAVE",
  "TABLE", "TABLES", "THEN", "TIME", "TIMESTAMP", "TINYINT", "TRAILING",
  "TRANSACTION", "TO", "UPDATE", "USER", "USING", "VALUES", "VARCHAR",
  "VARBINARY", "WHERE", "WHEN", "WITH", "WORK", "PROCESSLIST", "QUERY",
  "CONNECTION", "WEAK", "INDEX", "STORING", "BLOOMFILTER_JOIN",
  "MERGE_JOIN", "HASH_JOIN", "AUTO_INCREMENT", "CHUNKSERVER",
  "COMPRESS_METHOD", "CONSISTENT_MODE", "EXPIRE_INFO", "GRANTS",
  "JOIN_INFO", "MERGESERVER", "REPLICA_NUM", "ROOTSERVER", "ROW_COUNT",
  "SERVER", "SERVER_IP", "SERVER_PORT", "SERVER_TYPE", "STATUS",
  "TABLE_ID", "TABLET_BLOCK_SIZE", "TABLET_MAX_SIZE", "UNLOCKED",
  "UPDATESERVER", "USE_BLOOM_FILTER", "VARIABLES", "VERBOSE", "WARNINGS",
  "';'", "','", "$accept", "sql_stmt", "stmt_list", "stmt", "expr_list",
  "column_ref", "expr_const", "simple_expr", "arith_expr", "expr",
  "in_expr", "case_expr", "case_arg", "when_clause_list", "when_clause",
  "case_default", "array_expr", "func_expr", "when_func", "when_func_name",
  "when_func_stmt", "distinct_or_all", "delete_stmt", "update_stmt",
  "update_asgn_list", "update_asgn_factor", "create_index_stmt",
  "opt_index_columns", "opt_storing", "opt_storing_columns",
  "opt_index_option_list", "index_option", "cursor_declare_stmt",
  "cursor_name", "cursor_open_stmt", "cursor_fetch_stmt",
  "cursor_fetch_into_stmt", "cursor_fetch_next_into_stmt",
  "cursor_fetch_first_into_stmt", "cursor_fetch_prior_into_stmt",
  "cursor_fetch_last_into_stmt", "cursor_fetch_absolute_into_stmt",
  "cursor_fetch_relative_into_stmt", "fetch_prior_stmt",
  "fetch_first_stmt", "fetch_last_stmt", "fetch_relative_stmt",
  "next_or_prior", "fetch_absolute_stmt", "fetch_fromto_stmt",
  "cursor_close_stmt", "create_table_stmt", "opt_if_not_exists",
  "table_element_list", "table_element", "column_definition", "data_type",
  "opt_decimal", "opt_float", "opt_precision", "opt_time_precision",
  "opt_char_length", "opt_column_attribute_list", "column_attribute",
  "opt_table_option_list", "table_option", "opt_equal_mark",
  "drop_table_stmt", "opt_if_exists", "table_list", "truncate_table_stmt",
  "drop_index_stmt", "index_list", "table_name", "insert_stmt", "opt_when",
  "replace_or_insert", "opt_insert_columns", "column_list",
  "insert_vals_list", "insert_vals", "select_stmt", "select_with_parens",
  "select_no_parens", "no_table_select", "select_clause",
  "select_into_clause", "simple_select", "opt_where", "select_limit",
  "opt_hint", "opt_hint_list", "hint_options", "hint_option",
  "opt_comma_list", "join_op_type_list", "join_op_type",
  "consistency_level", "limit_expr", "opt_select_limit", "opt_for_update",
  "parameterized_trim", "opt_groupby", "opt_order_by", "order_by",
  "sort_list", "sort_key", "opt_asc_desc", "opt_having", "opt_distinct",
  "projection", "select_expr_list", "from_list", "table_factor",
  "relation_factor", "joined_table", "join_type", "join_outer",
  "explain_stmt", "explainable_stmt", "opt_verbose", "show_stmt",
  "lock_table_stmt", "opt_limit", "opt_for_grant_user", "opt_scope",
  "opt_show_condition", "opt_like_condition", "opt_full",
  "create_user_stmt", "user_specification_list", "user_specification",
  "user", "password", "drop_user_stmt", "user_list", "set_password_stmt",
  "opt_for_user", "rename_user_stmt", "rename_info", "rename_list",
  "lock_user_stmt", "lock_spec", "opt_work",
  "opt_with_consistent_snapshot", "begin_stmt", "commit_stmt",
  "rollback_stmt", "kill_stmt", "opt_is_global", "opt_flag", "grant_stmt",
  "priv_type_list", "priv_type", "opt_privilege", "priv_level",
  "revoke_stmt", "opt_on_priv_level", "prepare_stmt", "stmt_name",
  "preparable_stmt", "variable_set_stmt", "var_and_val_list",
  "var_and_val", "var_and_array_val", "to_or_eq", "argument",
  "array_vals_list", "array_val_list", "execute_stmt", "opt_using_args",
  "argument_list", "deallocate_prepare_stmt", "deallocate_or_drop",
  "alter_table_stmt", "alter_column_actions", "alter_column_action",
  "opt_column", "opt_drop_behavior", "alter_column_behavior",
  "alter_system_stmt", "opt_force", "alter_system_actions",
  "alter_system_action", "opt_comment", "opt_config_scope", "server_type",
  "opt_cluster_or_address", "column_name", "relation_name",
  "function_name", "column_label", "unreserved_keyword",
  "create_procedure_stmt", "proc_decl", "proc_parameter_list",
  "proc_parameter", "proc_block", "proc_sect", "proc_stmts", "proc_stmt",
  "control_sect", "control_stmts", "control_stmt", "stmt_declare",
  "stmt_assign", "stmt_if", "stmt_elsifs", "stmt_elsif", "stmt_else",
  "stmt_case", "case_when_list", "case_when", "case_else", "stmt_loop",
  "stmt_while", "stmt_null", "stmt_exit", "exec_procedure_stmt",
  "drop_procedure_stmt", "show_procedure_stmt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,    43,    45,    42,    47,    37,   287,    94,   288,
      40,    41,    46,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391,   392,   393,   394,   395,
     396,   397,   398,   399,   400,   401,   402,   403,   404,   405,
     406,   407,   408,   409,   410,   411,   412,   413,   414,   415,
     416,   417,   418,   419,   420,   421,   422,   423,   424,   425,
     426,   427,   428,   429,   430,   431,   432,   433,   434,   435,
     436,   437,   438,   439,   440,   441,   442,   443,   444,   445,
     446,   447,   448,   449,   450,   451,   452,   453,   454,   455,
     456,   457,   458,   459,   460,   461,   462,   463,   464,   465,
     466,   467,   468,   469,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   479,   480,   481,   482,   483,   484,    59,
      44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   241,   242,   243,   243,   244,   244,   244,   244,   244,
     244,   244,   244,   244,   244,   244,   244,   244,   244,   244,
     244,   244,   244,   244,   244,   244,   244,   244,   244,   244,
     244,   244,   244,   244,   244,   244,   244,   244,   244,   244,
     244,   244,   244,   244,   244,   244,   244,   244,   244,   245,
     245,   246,   246,   246,   247,   247,   247,   247,   247,   247,
     247,   247,   247,   247,   248,   248,   248,   248,   248,   248,
     248,   248,   248,   248,   249,   249,   249,   249,   249,   249,
     249,   249,   249,   249,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
     250,   250,   250,   250,   250,   250,   251,   251,   252,   253,
     253,   254,   254,   255,   256,   256,   257,   257,   258,   258,
     258,   258,   258,   258,   259,   260,   261,   261,   261,   261,
     262,   262,   263,   264,   265,   265,   266,   267,   268,   269,
     269,   270,   271,   271,   271,   272,   273,   274,   275,   276,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   288,   289,   290,   291,   292,   293,   293,
     294,   294,   295,   295,   296,   297,   297,   297,   297,   297,
     297,   297,   297,   297,   297,   297,   297,   297,   297,   297,
     297,   297,   297,   297,   297,   297,   298,   298,   298,   299,
     299,   300,   300,   301,   301,   302,   302,   303,   303,   304,
     304,   304,   304,   304,   305,   305,   305,   306,   306,   306,
     306,   306,   306,   306,   306,   306,   306,   307,   307,   308,
     309,   309,   310,   310,   311,   312,   313,   313,   313,   314,
     315,   315,   315,   316,   316,   317,   317,   318,   318,   319,
     319,   320,   320,   321,   321,   322,   322,   323,   323,   324,
     324,   324,   324,   325,   325,   326,   326,   327,   328,   328,
     328,   328,   329,   329,   329,   330,   330,   330,   330,   330,
     331,   331,   332,   332,   332,   333,   333,   334,   334,   334,
     334,   334,   334,   334,   334,   334,   334,   335,   335,   336,
     336,   337,   337,   337,   338,   338,   338,   338,   339,   339,
     340,   340,   341,   341,   342,   342,   342,   342,   342,   342,
     342,   343,   343,   344,   344,   345,   346,   346,   347,   348,
     348,   348,   349,   349,   350,   350,   350,   351,   351,   351,
     351,   352,   352,   353,   353,   354,   354,   354,   354,   354,
     354,   354,   355,   356,   356,   356,   357,   357,   357,   357,
     358,   358,   359,   360,   360,   360,   360,   361,   361,   362,
     362,   362,   362,   362,   362,   362,   362,   362,   362,   362,
     362,   362,   362,   362,   362,   363,   364,   364,   364,   365,
     365,   365,   366,   366,   366,   367,   367,   367,   368,   368,
     369,   369,   370,   371,   371,   372,   373,   374,   375,   376,
     376,   377,   377,   378,   378,   379,   380,   381,   381,   382,
     383,   383,   384,   384,   385,   385,   386,   386,   387,   388,
     389,   390,   390,   391,   391,   391,   392,   393,   393,   394,
     394,   394,   394,   394,   394,   394,   394,   394,   394,   394,
     394,   395,   395,   396,   396,   397,   398,   398,   399,   400,
     401,   401,   401,   401,   402,   402,   403,   403,   404,   404,
     404,   404,   404,   404,   404,   404,   405,   406,   406,   407,
     407,   408,   409,   409,   410,   411,   411,   412,   412,   413,
     414,   414,   415,   415,   416,   416,   417,   417,   417,   417,
     418,   418,   419,   419,   419,   420,   420,   420,   420,   421,
     421,   421,   421,   421,   422,   422,   423,   423,   424,   425,
     425,   426,   426,   426,   426,   427,   427,   427,   427,   428,
     428,   428,   429,   429,   430,   430,   431,   432,   432,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   433,   433,   433,   433,   433,   433,
     433,   433,   433,   433,   434,   435,   435,   436,   436,   437,
     437,   437,   437,   437,   437,   437,   437,   438,   439,   439,
     440,   440,   441,   441,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   441,   441,   441,   441,   441,   441,
     441,   441,   441,   441,   441,   442,   442,   443,   443,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   444,   444,   444,   444,   444,   444,   444,   444,   444,
     444,   445,   445,   445,   446,   447,   447,   447,   447,   448,
     448,   449,   450,   451,   452,   452,   453,   454,   454,   455,
     455,   455,   456,   457,   458,   458,   459,   459,   460,   460,
     461
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       3,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     3,     5,     1,     1,
       1,     1,     2,     1,     1,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     1,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     4,     3,     3,     2,     3,     4,     3,     4,     3,
       4,     5,     6,     3,     4,     3,     1,     3,     5,     1,
       0,     1,     2,     4,     2,     0,     4,     4,     4,     5,
       4,     6,     4,     3,     4,     1,     1,     1,     1,     1,
       1,     1,     5,     7,     1,     3,     3,     9,     3,     2,
       0,     3,     1,     3,     0,     1,     5,     1,     2,     2,
       3,     4,     5,     5,     5,     5,     6,     7,     3,     3,
       3,     5,     1,     1,     4,     6,     2,     8,     3,     0,
       1,     3,     1,     5,     3,     1,     1,     1,     1,     1,
       2,     2,     1,     2,     1,     2,     2,     1,     2,     2,
       2,     2,     1,     1,     1,     2,     5,     3,     0,     3,
       0,     1,     0,     3,     0,     3,     0,     2,     0,     2,
       1,     2,     1,     2,     1,     3,     0,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     0,     4,
       0,     2,     1,     3,     5,     6,     0,     1,     3,     1,
       7,     4,     7,     0,     2,     1,     1,     3,     0,     1,
       3,     3,     5,     1,     3,     2,     1,     3,     3,     1,
       2,     3,     4,     5,     8,     1,     1,     9,     9,     4,
       4,     4,     0,     2,     3,     4,     4,     2,     2,     4,
       0,     3,     1,     3,     0,     1,     2,     1,     1,    14,
       4,     5,     4,     3,     4,     1,     1,     2,     0,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     0,     2,     3,     4,     4,     4,     3,     3,
       3,     0,     3,     1,     0,     3,     1,     3,     2,     0,
       1,     1,     0,     2,     0,     1,     1,     1,     2,     3,
       1,     1,     3,     1,     3,     1,     3,     2,     3,     2,
       1,     5,     1,     3,     6,     5,     2,     2,     2,     1,
       1,     0,     3,     1,     1,     1,     1,     1,     0,     3,
       5,     5,     5,     4,     4,     4,     2,     4,     3,     3,
       3,     3,     3,     3,     3,     3,     4,     2,     0,     1,
       2,     4,     1,     1,     0,     0,     2,     2,     0,     1,
       0,     1,     3,     1,     3,     4,     1,     1,     3,     1,
       3,     5,     6,     2,     0,     3,     3,     1,     3,     4,
       1,     1,     1,     0,     3,     0,     2,     3,     2,     2,
       4,     0,     1,     0,     1,     1,     6,     1,     3,     2,
       1,     1,     2,     1,     1,     1,     2,     1,     1,     1,
       1,     1,     0,     1,     1,     5,     2,     0,     4,     1,
       1,     1,     1,     1,     2,     3,     1,     3,     3,     3,
       4,     4,     5,     5,     3,     3,     3,     1,     1,     1,
       1,     3,     1,     3,     3,     2,     0,     1,     3,     3,
       1,     1,     4,     6,     1,     3,     3,     4,     4,     5,
       1,     0,     1,     1,     0,     3,     3,     3,     2,     4,
       7,     7,     6,     6,     0,     1,     1,     3,     9,     2,
       0,     3,     3,     3,     0,     1,     1,     1,     1,     3,
       6,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     6,     5,     3,     1,     2,
       3,     3,     3,     3,     4,     4,     4,     3,     0,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     1,     0,     1,     2,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       1,     4,     6,     5,     3,     9,     8,     8,     7,     1,
       2,     4,     2,     7,     2,     1,     4,     0,     2,     5,
      11,    12,     7,     1,     2,     4,     6,     5,     5,     3,
       3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
      48,     0,     0,     0,     0,     0,     0,     0,   433,   433,
       0,   500,     0,     0,     0,   501,     0,     0,   378,     0,
     256,     0,     0,     0,   255,     0,   433,   441,   290,     0,
     404,     0,   290,     0,     0,     4,    24,    23,    22,     9,
      10,    11,    13,    14,    15,    16,    17,    18,    12,    21,
      25,    47,    26,    20,     0,    19,   276,   253,   269,   334,
     275,    27,    28,    46,    35,    36,    37,    38,    39,    42,
      43,    44,    45,    40,    41,    29,    30,    31,    34,     0,
      32,    33,     5,     0,     6,     8,     7,   276,     0,   547,
     549,   550,   551,   552,   553,   554,   555,   556,   557,   558,
     559,   560,   561,   562,   563,   564,   566,   565,   567,   568,
     569,   570,   571,   572,   573,     0,   157,   548,     0,   158,
     159,   176,   524,     0,     0,   432,   436,   438,     0,   179,
       0,   179,     0,   544,   408,   362,   545,   408,     0,   240,
       0,   240,   240,   496,   469,   377,     0,   462,   450,   451,
     453,   454,   455,     0,   457,   460,   459,   458,     0,   447,
       0,     0,     0,   467,   439,   442,   443,   294,   344,   542,
       0,     0,     0,     0,   424,     0,     0,     0,     0,   474,
     476,     0,   543,   546,     0,     0,     0,   411,   402,   405,
     386,   403,     0,   405,     0,   424,     0,   398,     0,     0,
       0,     0,   435,     0,     1,     2,    48,     0,     0,   265,
     344,   344,   344,     0,     0,   322,     0,   270,     0,   588,
     574,   268,   267,     0,     0,   160,   168,   169,   170,     0,
       0,     0,   525,     0,     0,     0,     0,     0,   416,     0,
       0,     0,     0,   412,   413,     0,     0,   282,   409,   389,
     388,   669,     0,     0,     0,   419,   418,   246,     0,     0,
     494,   374,   376,   375,   373,   372,   461,   449,   452,   456,
       0,     0,   395,     0,     0,   427,   425,     0,     0,   444,
     445,     0,     0,   308,   298,     0,     0,   297,     0,   305,
     306,     0,     0,   292,   295,   345,   346,     0,   488,   487,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     475,     0,     0,     0,   670,     0,     0,     0,     0,     0,
     393,   405,   379,     0,     0,   392,   399,   405,     0,   390,
     391,   405,   394,     0,     0,   437,     0,     3,   258,   542,
      54,    56,    55,    58,    57,    59,    60,    62,    61,     0,
       0,     0,     0,   120,     0,     0,   559,    64,    65,    84,
     254,    68,    73,    69,    70,     0,    71,    51,     0,   543,
       0,     0,     0,     0,     0,     0,   322,   271,   323,   499,
       0,     0,   615,     0,     0,     0,     0,     0,   290,     0,
     663,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   589,   591,   596,
     597,   592,   593,   595,   594,   614,   613,     0,   290,     0,
      49,   174,     0,     0,   519,   526,     0,     0,     0,     0,
       0,   511,   511,   511,   511,   502,   504,     0,   430,   431,
     429,     0,     0,     0,     0,     0,     0,     0,   253,     0,
     241,     0,   239,     0,   242,   355,   360,     0,     0,   247,
     530,   489,   490,   497,   495,   463,     0,   464,   448,   473,
     472,   471,   470,   468,     0,     0,   466,     0,   440,     0,
       0,     0,     0,     0,     0,   291,     0,   296,   350,   347,
     351,   320,   484,     0,     0,   478,     0,     0,   423,     0,
       0,     0,     0,   485,   477,   479,   405,   405,   387,   406,
     407,   383,   405,   400,   384,   397,   385,     0,   133,   140,
       0,   141,     0,     0,     0,    49,     0,     0,     0,     0,
       0,     0,   251,   104,    85,    86,     0,    49,    71,   119,
       0,    72,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   290,   276,   281,   275,
     279,   280,   339,   335,   336,   318,   319,   287,   288,   272,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   616,   618,
     623,   619,   620,   622,   621,   640,   639,     0,   664,     0,
       0,     0,     0,     0,     0,   601,   600,   603,   604,   606,
     607,   608,   610,   609,   611,   612,   605,   599,   602,   598,
     587,   590,   156,   667,   290,     0,     0,   171,     0,     0,
       0,     0,     0,     0,   510,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   576,     0,     0,     0,   578,   178,
       0,   414,     0,     0,     0,   283,   142,   668,   276,     0,
     360,     0,     0,   359,   371,   369,     0,   371,   371,     0,
       0,   357,   420,     0,     0,     0,   244,     0,     0,   426,
     428,   465,     0,   303,   307,   311,   312,   313,     0,   309,
       0,   317,   316,   315,   314,     0,     0,   293,     0,   348,
       0,     0,   321,   273,   126,   127,   480,     0,   417,   421,
     481,     0,     0,   486,   382,   381,   380,     0,     0,   128,
       0,     0,     0,     0,     0,     0,   130,     0,     0,     0,
     132,   434,   282,   144,     0,     0,   259,     0,     0,    66,
       0,   125,   121,    63,   103,   102,     0,     0,     0,    99,
      97,    98,    96,    95,    94,   115,   100,     0,     0,    74,
       0,     0,   113,   116,   107,   105,   109,     0,    87,    88,
      89,    90,    91,    93,    92,     0,   139,   138,   137,   136,
      53,    52,   344,   340,   341,   338,     0,     0,     0,     0,
     189,   216,   192,   216,   202,   204,   197,   208,   212,   210,
     188,   187,   203,   208,   194,   186,   214,   214,   185,   216,
     216,     0,   615,   627,   626,   629,   630,   632,   633,   634,
     636,   635,   637,   638,   631,   625,   628,   624,     0,   617,
       0,   172,   173,     0,     0,     0,     0,     0,     0,   657,
     655,     0,   615,     0,   644,   666,    50,   175,   527,    61,
     530,   522,   523,     0,     0,   506,     0,     0,   514,   503,
       0,   511,   505,   422,   579,     0,     0,     0,   575,     0,
       0,     0,   180,   182,   415,     0,   284,   363,   243,   358,
     370,   366,     0,   367,   368,     0,   356,   245,   249,   248,
     529,   498,   446,   302,   304,     0,     0,   300,     0,   349,
     282,   282,   353,   352,   482,   483,   492,     0,   401,   396,
     328,     0,   329,     0,   330,     0,     0,   324,   129,     0,
     253,     0,   257,     0,     0,   253,    50,     0,     0,   122,
       0,   101,     0,   114,    75,    76,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   108,   106,   110,   134,     0,
     337,   285,   289,   286,     0,   199,   198,     0,   190,   211,
     195,     0,   193,   191,     0,   205,   196,   200,   201,     0,
       0,   641,     0,     0,   665,     0,     0,     0,     0,     0,
     161,     0,     0,   615,   654,     0,     0,     0,     0,     0,
     534,   520,   521,   218,     0,     0,   508,   512,   513,   507,
       0,   583,   580,   581,   582,   577,     0,   226,     0,     0,
     150,     0,     0,     0,   310,     0,   301,   320,     0,   331,
     491,     0,   325,   326,   327,   131,   145,   143,   146,   252,
     260,   263,     0,     0,   250,    67,     0,   124,   118,     0,
     111,    77,    78,    79,    80,    81,    83,    82,   117,     0,
       0,     0,     0,     0,   643,     0,     0,   659,   162,   164,
     163,   165,     0,     0,   615,   658,     0,     0,     0,     0,
     615,     0,     0,   649,     0,     0,     0,     0,   184,     0,
     518,     0,     0,   509,   584,   585,   586,     0,   238,   238,
     238,   238,   238,   238,   238,   238,   238,   238,   177,   224,
     181,     0,     0,   154,   361,   365,     0,     0,   274,   354,
       0,   342,   493,   261,     0,     0,   123,   112,     0,   215,
     207,     0,   209,   213,   642,     0,   166,     0,   656,     0,
       0,     0,     0,   652,     0,     0,   650,     0,     0,     0,
       0,     0,   220,     0,     0,     0,   222,   217,   516,   515,
     517,     0,   237,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   148,     0,   149,   147,   152,   155,
     364,     0,     0,     0,   278,   264,     0,     0,   662,   167,
     653,     0,   615,   615,   648,     0,     0,     0,   282,   533,
     531,   532,     0,   219,   221,   223,   183,   236,   233,   235,
     228,   227,   232,   231,   230,   229,   234,   225,     0,     0,
       0,   332,   343,   262,   206,   615,     0,   651,   646,     0,
     647,   322,   537,   538,   535,   536,   541,   151,   153,     0,
       0,     0,   645,   277,     0,     0,   528,     0,     0,     0,
       0,     0,     0,     0,   660,   539,     0,     0,   661,     0,
       0,     0,   299,   540
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    33,    34,    35,   419,   357,   358,   359,   770,   489,
     772,   361,   540,   751,   752,   940,   362,   363,   364,   365,
     785,   526,   582,   583,   742,   743,    38,  1020,  1113,  1176,
    1177,  1178,   584,   115,   585,    41,   586,   587,   588,   589,
     590,   591,   592,    42,    43,    44,    45,   231,    46,    47,
     593,    49,   242,   881,   882,   883,   821,   968,   972,   970,
     975,   965,  1088,  1157,  1108,  1179,  1163,    50,   254,   452,
      51,    52,   458,   897,   594,   209,    54,   531,   745,   935,
    1042,   595,   366,    57,    58,    59,   596,    60,   448,   712,
     168,   292,   293,   294,   480,   698,   699,   705,   577,   713,
     217,   527,  1121,   214,   215,   573,   574,   795,  1184,   297,
     490,   491,   911,   912,   455,   456,   679,   891,    61,   265,
     146,    62,    63,   329,   325,   199,   320,   249,   200,    64,
     243,   244,   255,   719,    65,   256,    66,   306,    67,   275,
     276,    68,   440,   126,   335,    69,    70,    71,    72,   166,
     281,    73,   158,   159,   267,   466,    74,   278,    75,   143,
     473,    76,   179,   180,   310,   300,   463,   723,   917,    77,
     260,   464,    78,    79,    80,   435,   436,   649,  1009,  1006,
      81,   236,   424,   425,   686,  1087,  1236,  1246,   367,   368,
     201,   116,   369,    82,    83,   657,   658,   220,   406,   407,
     408,   597,   598,   599,   409,   600,   601,  1082,  1083,  1084,
     602,   849,   850,   995,   603,   604,   605,   606,    84,    85,
      86
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -967
static const yytype_int16 yypact[] =
{
    3345,    26,  2721,   121,  2721,  2721,  2721,   290,   -57,   -57,
     156,  -967,    47,  2886,  2886,   175,   -59,  2721,   -72,   809,
    -967,   -31,  2721,   -11,  -967,   809,   -57,    77,   216,   960,
     504,   162,   216,   364,   -49,  -967,  -967,  -967,  -967,  -967,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,
    -967,  -967,  -967,  -967,   236,  -967,     9,   171,  -967,    62,
      81,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,   231,
    -967,  -967,  -967,   316,  -967,  -967,  -967,   354,   362,  -967,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,
    -967,  -967,  -967,  -967,  -967,   352,  -967,  -967,   403,  -967,
     664,  -967,   217,  2886,   429,  -967,  -967,  -967,   432,   319,
     429,   319,  2886,  -967,   451,  -967,  -967,   451,   209,   337,
     429,   337,   337,   271,  -967,  -967,   365,   320,  -967,   296,
    -967,  -967,  -967,   335,  -967,  -967,  -967,  -967,   -64,  -967,
    2886,   383,   429,   -62,  -967,  -967,   210,   717,   269,  -967,
      27,    48,  2925,   467,   404,  2925,   472,   511,    27,   302,
    -967,    27,  -967,  -967,   546,    79,   367,  -967,  -967,    28,
    -967,  -967,   325,    28,   409,   445,   330,   424,   324,   349,
     382,   533,   385,  2886,  -967,  -967,  3345,  2886,  2248,  -967,
     269,   269,   269,   512,   -20,    16,   402,  -967,  2721,  3124,
    -967,  -967,  -967,   493,  1591,   552,   558,  -967,  -967,   606,
     620,   561,  -967,  2925,   484,   446,   421,   368,  -967,   219,
     593,   614,  2886,   413,  -967,   509,  2886,   441,  -967,  -967,
    -967,  -967,   544,   549,  2567,  -967,   422,  2886,  2567,   650,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,
    2592,   809,  -967,   365,   471,  -967,   427,  2592,   554,  -967,
    -967,   666,   632,  -967,  -967,   637,   645,  -967,   649,  -967,
    -967,   657,   100,   717,  -967,  -967,  -967,  1753,  -967,  -967,
    2248,   339,  2248,    27,  2925,   429,   678,    27,  2925,    27,
    -967,  2248,  2414,  2248,  -967,  2886,  2886,  2886,   701,  2248,
    -967,    28,  -967,  2886,    97,  -967,  -967,    28,   706,  -967,
    -967,    28,  -967,  1338,   628,  -967,   542,  -967,    80,   460,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,   677,  2248,
    2248,  2248,  1832,  2248,   679,   689,   681,  -967,  -967,  -967,
    3766,  -967,  -967,  -967,  -967,   693,  -967,  -967,   694,   696,
     101,   101,   101,  2248,   127,   127,   627,  -967,  -967,  -967,
    2650,  2248,  3147,   -99,  2721,  2248,   739,  2248,   216,  2414,
    -967,   516,   517,   534,   535,   536,   538,   540,   543,   602,
     609,   612,   615,   616,   618,   622,   675,  3124,  -967,  -967,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,    26,   216,    -3,
    3766,  -967,   589,   840,   613,  -967,   835,   838,   839,   735,
     742,   787,   787,   787,   -10,   648,  -967,   812,  -967,  -967,
    -967,   900,   780,   852,   429,   817,   750,  1915,   171,   895,
    -967,   581,   661,  2621,   571,  2828,  -967,   429,   -40,  -967,
     -27,   677,  -967,  -967,   663,  -967,   711,  -967,  -967,  -967,
    -967,  -967,  -967,  -967,   429,   429,  -967,   429,  -967,   906,
      -1,   138,  2886,   -34,  2886,  -967,   717,  -967,  -967,  1431,
    -967,   261,  3766,   869,   871,  3766,  2248,    27,  -967,   911,
    2248,    27,   876,  3766,  -967,  3766,    28,    28,  -967,  -967,
    3766,  -967,    28,   882,  -967,   683,  -967,   883,  -967,  -967,
    2003,  -967,  2086,  2165,    13,  3523,  2248,   891,   756,  2925,
    1668,   736,  -967,  2319,  -967,  -967,   695,  3694,   395,  3766,
     738,  -967,  2925,  2248,  2248,   431,  2248,  2248,  2248,  2248,
    2248,  2248,  2248,  2248,  2517,   903,   149,  2248,  2248,  2248,
    2248,  2248,  2248,  2248,   365,  2689,   216,  -967,   927,  -967,
     927,  -967,  2969,   705,  -967,  -967,  -967,   -65,   814,  -967,
     807,  3641,   710,   714,   727,   730,   731,   732,   733,   737,
     740,   743,   745,   755,   762,   763,   764,   859,  3147,  -967,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,  2248,  -967,   532,
    1301,   937,  2411,  1509,   291,  -967,  -967,  -967,  -967,  -967,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,
    -967,  -967,  -967,  -967,   216,  2248,   970,  -967,  2925,   566,
     973,   977,   971,   981,  -967,  2925,  2925,  2925,  2886,  2925,
     388,   911,  3135,   995,  -967,   999,  1000,    21,  -967,  -967,
    2857,  -967,   911,  2886,  2248,  3766,  -967,  -967,  2542,   571,
     979,  2567,  2886,  -967,   868,  -967,  2567,   868,   868,   890,
    2886,  -967,  -967,  2886,  2886,  1019,  -967,   650,   429,  -967,
    -967,   422,   983,  -967,  -967,  -967,  -967,  -967,    23,  -967,
     785,  -967,  -967,  -967,  -967,   991,  2886,   717,  2721,  -967,
    2451,  1753,  -967,  -967,  -967,  -967,  3766,  2248,  -967,  -967,
    3766,  2248,   566,  -967,  -967,  -967,  -967,   994,  1029,  -967,
    2248,  3544,  2248,  3576,  2248,  3597,  -967,  3135,  2248,  3718,
    -967,  -967,   -89,  -967,  1012,    39,  -967,   997,  2248,  -967,
    2248,     1,  -967,  -967,  2340,  2319,  2248,  2517,   903,  1175,
    1175,  1175,  1175,  1175,  1175,  1191,  1087,  2517,  2517,  -967,
     833,  1832,  -967,  -967,  -967,  -967,  -967,   415,   955,   955,
    1001,  1001,  1001,  1001,  -967,  1002,  -967,  -967,  -967,  -967,
    -967,  -967,   269,  -967,  -967,  -967,  2248,   127,   127,   127,
    -967,   998,  -967,   998,  -967,  -967,  -967,  1004,   889,  1005,
    -967,  -967,  -967,  1004,  -967,  -967,  1006,  1006,  -967,   998,
     998,    -6,  3147,  -967,  -967,  -967,  -967,  -967,  -967,  -967,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,  1003,  -967,
     802,   917,   918,   920,   922,  1050,   650,  1010,  2248,    33,
    -967,  2484,  3147,   -60,  -967,  -967,  3766,  -967,  -967,  -967,
     982,  -967,  -967,  1062,  1064,  -967,  3135,   241,    11,  -967,
     875,   787,  -967,  -967,  1016,  3135,  3135,  3135,  -967,   610,
     942,    46,  -967,  -967,  -967,  1031,  3766,  1007,   571,  -967,
    -967,  -967,   401,  -967,  -967,  2567,  -967,  -967,  -967,  -967,
    -967,  -967,   422,  -967,  -967,   138,  2886,  -967,  1047,  -967,
     441,   -88,   571,  -967,  3766,  3766,  -967,    50,  -967,  -967,
    3766,  2248,  3766,  2248,  3766,  2248,  1049,  3766,  -967,  2925,
     171,  2248,    26,  2925,  2248,   -84,  3742,  2942,  2248,  -967,
     985,  1087,   923,  -967,  -967,  -967,  2517,  2517,  2517,  2517,
    2517,  2517,  2517,  2517,    51,  -967,  -967,  -967,  -967,  1753,
    -967,  -967,  -967,  -967,  1088,  -967,  -967,  1089,  -967,  -967,
    -967,  1093,  -967,  -967,  1094,  -967,  -967,  -967,  -967,   853,
     566,  -967,  1008,   861,  -967,   650,   650,   650,   650,   978,
     663,  1096,  2996,  3147,  -967,  1022,  2517,   726,   373,   650,
     938,  -967,  -967,  -967,   108,   303,  -967,  -967,  -967,  -967,
    2721,  -967,  1056,  1057,  1059,  -967,  1090,   288,  2857,  2925,
     921,  2886,  2248,   728,  -967,   896,  -967,   -20,  2567,  1017,
    -967,   566,  3766,  3766,  3766,  -967,  -967,  -967,  3766,  -967,
    -967,  3766,    54,  1102,  -967,  -967,  2248,  3766,  -967,  2517,
     753,   993,   993,  1105,  1105,  1105,  1105,  -967,  -967,   -18,
    1103,    57,  1104,  1108,  -967,   907,  1112,  -967,   663,   663,
     663,   663,   650,  1032,  3147,  -967,  1083,   734,  2517,  2248,
    3147,  1037,   393,  -967,  1060,   -14,  1141,   940,    53,  1159,
    -967,  1160,   566,  -967,  -967,  -967,  -967,  2925,  1148,  1148,
    1148,  1148,  1148,  1148,  1148,  1148,  1148,  1148,   959,  -967,
    -967,    58,  1176,   288,  -967,  3766,  2248,  2886,  -967,   571,
    1123,  1095,  -967,  -967,  2248,  2248,  3766,   753,  2567,  -967,
    -967,  1225,  -967,  -967,  -967,   992,   663,   650,  -967,   996,
    2517,  1025,  3346,  -967,  1009,  1106,  -967,  1128,  1109,  2567,
     280,  1213,  -967,  1228,   566,  1107,  -967,  -967,  -967,  -967,
    -967,    61,  -967,  1235,  1236,  1063,  1238,  1240,  1241,  1244,
    1247,  1248,  1249,   288,  -967,  2925,  -967,  1018,  -967,  -967,
    3766,  1214,  2248,  2248,  -967,  3766,    67,  1219,  -967,   663,
    -967,  1048,  3147,  3147,  -967,  1023,  1134,  1024,   -88,  -967,
    -967,  -967,   520,  -967,  -967,  -967,  -967,  -967,  -967,  -967,
    -967,  -967,  -967,  -967,  -967,  -967,  -967,  -967,    69,   288,
    2886,  1026,  3766,  -967,  -967,  3147,  1158,  -967,  -967,  1028,
    -967,   627,  -967,  -967,  -967,  -967,   -24,  -967,  -967,  1030,
    1162,  1216,  -967,  -967,  1251,  1255,  -967,  2886,  1222,  1033,
    1268,  1277,  1242,  1043,  -967,  -967,  1058,  2886,  -967,  1259,
    1245,  1282,  -967,  -967
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -967,  -967,  -967,  1084,  -324,  -967,  -603,  -348,  -202,   754,
     531,  -967,  -967,  -967,   541,  -967,     0,  1261,  -967,  -967,
    -967,  -967,    43,    65,  -967,   366,  -967,  -967,  -967,  -967,
    -967,    74,    17,    29,    41,  -967,  -177,  -150,  -143,  -126,
    -122,  -114,  -108,  -967,  -967,  -967,  -967,   685,  -967,  -967,
      59,  -967,  1165,  -967,   281,   665,  -558,   495,  -967,  -967,
     492,  -389,  -967,  -967,  -967,  -956,    52,  -967,   397,  1082,
    -967,  -967,  -967,  -967,    86,  -447,  -967,  -967,  -966,  -967,
     193,    68,    22,     5,  -967,   197,  -104,   211,  -729,  1131,
     -16,  -967,   863,  -286,  -967,  -967,   448,  -967,  -357,   327,
    -210,  -967,  -967,  -967,  -967,  -967,   559,  -967,  -967,  -180,
     646,  -576,   207,  -244,    31,   908,  -967,  -133,  -967,  -967,
    -967,  -967,  -967,  -967,  -967,  -967,  -144,  1223,  -967,  -967,
    -967,   919,   -78,  -537,  -967,  -438,  -967,  1167,  -967,   892,
    -967,  -967,  -967,   124,  -967,  -967,  -967,  -967,  -967,  -967,
    -967,  -967,  1339,  1097,  -967,  1092,  -967,  -967,  -967,    25,
    -967,  -967,   976,  1054,  -967,  -111,   686,  -967,  -967,  -967,
    -967,  -372,  -967,  -967,  -967,  -967,   724,   177,  -967,  -967,
    -967,  -967,  -967,   744,   515,  -967,  -967,  -967,    -8,    12,
    -967,     2,    -2,  -967,  -967,  -967,   497,  -967,  -967,  -967,
     974,  -748,  -967,   779,  -967,   -96,   -85,  -967,   298,   301,
     -83,  -967,   537,  -967,   -82,   -77,   -76,   -74,  -967,  -967,
    -967
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -547
static const yytype_int16 yytable[] =
{
     117,   666,   117,   117,   117,   377,    88,   487,   580,   524,
     454,   136,   136,   930,   454,   117,   203,    39,   578,   144,
     117,   181,    56,    87,   144,   135,   135,   182,   536,   178,
     370,   371,   372,   119,   120,   121,   860,   853,   634,   691,
     693,    40,   395,    36,   134,   137,   239,   161,   979,   322,
    -266,   298,   245,  1111,   736,   685,   318,  1244,   205,    48,
     302,  1109,   878,  1152,   904,    37,     1,   311,    55,   396,
     313,   999,   298,  1153,   982,   644,   397,   210,   211,   212,
     932,   797,   701,   270,   274,   277,    53,  1017,   301,  1007,
     980,  1030,  1058,   398,   874,  1123,  1128,   399,  1130,  1174,
    1149,   238,  1206,   607,   998,   400,   938,   683,  1223,   315,
    1237,   401,   447,   447,   873,   405,  -266,   374,   208,   916,
     530,   136,  -322,   410,   118,   884,   375,   142,  1089,   216,
     136,  1161,   575,   127,   411,   135,   412,   413,   993,   576,
     608,     1,   414,   415,   135,   416,   702,   125,   703,  1154,
     164,   929,  1028,  -333,   237,   160,  1043,   774,   136,   775,
     776,   132,  -333,   247,   303,   145,   579,   307,    56,   777,
     182,  1008,   135,   182,   704,   798,   271,   511,   271,   926,
     711,  1027,  1029,   514,   648,   162,   513,   516,  -322,   261,
     206,   272,   496,   316,   216,   165,   500,    28,   502,   128,
     684,   136,  1245,   750,  1090,   136,   769,   669,  1155,  1218,
     213,   262,   251,   671,   264,   135,   117,  1217,   138,   135,
     144,   299,   711,    39,   485,   426,   687,   498,    56,   319,
     395,   182,   263,   981,   336,   848,   393,   635,   338,   694,
     136,    56,   299,   379,   136,  1075,   498,    40,  -266,    36,
     902,    28,   136,   635,   135,   136,   136,   396,   135,   462,
     394,   879,   391,   905,   397,    48,   135,  1156,   136,   135,
     135,    37,   566,   443,    55,   136,   453,   446,   402,   933,
     453,   398,   467,  -322,   392,   399,  1018,   404,   459,   467,
    1031,   635,    53,   400,  1124,    56,   497,  1131,   933,   401,
     501,   933,   182,   405,   181,   403,   182,  1124,  1003,   933,
     182,   410,   178,   136,   136,   136,   469,  1012,  1013,  1014,
    -322,   136,   411,  1091,   412,   413,  1138,   135,   135,   135,
     414,   415,  1143,   416,   232,   135,   252,   295,   470,   167,
     486,   472,   129,  1004,   493,   437,   506,   507,   508,   695,
     696,   697,   130,   494,   512,   202,  1199,    88,   438,   471,
      56,   139,   724,   725,   204,   131,   245,   207,   726,   296,
    1098,   140,   613,   208,   538,   710,   541,  1065,   117,   682,
     462,   181,   117,  1059,   141,   218,   717,   182,   219,   178,
     721,   233,   567,   567,   567,   221,   689,   274,   374,  1092,
     234,   235,   633,   222,    56,     1,   532,   375,   223,   769,
    -276,  -276,  -276,   609,   966,  1005,   279,   280,  1079,   769,
     769,   487,  1200,   955,   393,   956,   957,   888,  1122,    56,
     977,   978,   892,   238,   431,   240,   221,   432,  1079,    56,
     961,   962,   963,   224,  1226,  1227,   241,   954,   394,   136,
     391,   136,   439,   136,   431,   248,    88,   432,  1201,   756,
     757,   758,    12,   135,   253,   673,   402,   681,   259,  1231,
     433,   122,   392,   668,   990,   404,   123,  1240,  1080,  1081,
     136,   266,   136,  1037,   269,   632,   124,   117,  1044,  1160,
     433,   709,   268,   403,   135,    20,   135,   273,  1080,  1145,
    -546,   711,  -544,   769,  1099,  1100,  1101,   183,  1102,   304,
    1103,   429,   430,   700,   308,   706,   674,   305,  1104,  1105,
    1106,   744,   746,  1107,    24,   309,   434,   182,   182,   675,
     854,   312,  -276,   676,   753,    88,    28,   677,   257,   258,
     182,  -276,   312,  -276,   893,   894,   871,   184,  1022,   314,
     792,  1204,    87,   317,   321,   942,   323,   791,   324,   327,
      32,   328,   330,   182,   678,   944,   945,   568,   570,   571,
     340,   341,   342,   333,   343,   344,   345,   773,   346,   347,
     859,   569,   569,   569,   133,   331,    56,   332,   334,   373,
     185,   186,   841,   842,   843,   844,   845,   378,   769,   769,
     769,   769,   769,   769,   769,   769,   417,   786,   645,   646,
     647,   421,   959,  1068,  1069,  1070,  1071,  -172,   855,   187,
      56,   451,   188,  -173,   652,   422,   423,  1085,   427,   787,
     426,   428,   789,   441,   442,   445,   182,   866,   867,   868,
     653,   870,   447,   182,   182,   182,   136,   182,   769,   997,
     788,  1023,   866,   444,   449,   189,   655,   656,   182,   450,
     135,   136,   457,   846,   461,   474,   136,   475,   477,   136,
     136,   478,   479,   190,   136,   135,   191,   481,   136,   869,
     673,   136,   136,   135,   889,   482,   674,   462,   135,   483,
     192,   193,   896,   453,   885,   135,   135,   484,   453,   675,
    1136,   769,   499,   676,   136,   509,   117,   677,   136,  -410,
     909,   515,   528,   194,   898,   899,   529,   301,   135,     1,
     282,  -135,   135,   195,   225,   226,   227,   228,   229,   196,
     769,   542,   453,   564,   678,  1232,   565,   908,  -545,   355,
     216,  1233,   197,  1234,  1050,  1051,  1052,  1053,  1054,  1055,
    1056,  1057,    28,   611,  1235,   615,   616,   283,   947,   948,
     949,   950,   951,   952,   953,  1189,   947,   948,   949,   950,
     951,   952,   953,   617,   618,   619,    88,   620,   230,   621,
     773,   630,   622,   636,  1119,   947,   948,   949,   950,   951,
     952,   953,   769,   538,  1077,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     543,   544,   545,   546,   547,   548,   549,   550,   551,   552,
     553,   554,   555,   556,   557,   558,   559,   560,   561,   562,
     563,   623,   284,   674,    56,   637,   462,  1127,   624,   285,
     286,   625,   946,   638,   626,   627,   675,   628,  1221,   639,
     676,   629,   640,   641,   677,   947,   948,   949,   950,   951,
     952,   953,   644,   287,    56,  1116,  1141,   147,   148,   642,
     800,   801,   802,   288,   289,   290,   643,   803,   650,   651,
     659,   678,   660,   136,   662,   804,   149,   663,   667,   805,
     806,   671,   807,   687,   136,   688,   150,   135,   808,   692,
     714,   151,   715,   152,   652,   718,   722,   453,   135,   809,
    1078,   744,   727,   728,   729,  1040,   291,   182,  1140,   153,
     653,   182,   740,   741,   747,   748,   810,  1025,  1191,   154,
     750,   654,  1049,   771,   212,   796,   655,   656,   811,   823,
     812,   799,   813,   824,    56,   947,   948,   949,   950,   951,
     952,   953,   360,   169,   814,   838,   825,   851,   155,   826,
     827,   828,   829,   170,   171,   857,   830,   861,   420,   831,
     156,   862,   832,   815,   833,   462,   462,   462,   462,   559,
     560,   561,   562,   563,   834,   863,   816,   817,   818,   462,
    1039,   835,   836,   837,   157,   864,   819,   820,   117,   875,
     866,   746,  1093,   876,   877,    56,   182,   182,   890,   136,
     887,  1243,   895,   900,   903,   906,   136,   949,   950,   951,
     952,   953,   907,  1114,   919,   918,   931,   934,   964,   563,
     135,   984,   969,   958,   967,   971,   974,   687,   985,   986,
     453,   987,   983,   988,   492,   989,   495,   947,   948,   949,
     950,   951,   952,   953,   685,   503,  1001,   505,  1002,  1010,
    1011,  1019,   462,   510,  1192,   991,  1016,  1021,   172,   173,
     947,   948,   949,   950,   951,   952,   953,   525,  1026,   746,
    1035,  1048,  1064,  1060,  1061,   182,    56,  1225,  1062,  1063,
    1067,  1073,    56,   533,   534,   535,   537,   539,  1086,  1072,
    1094,  1095,   174,  1096,  1066,   136,   554,   555,   556,   557,
     558,   559,   560,   561,   562,   563,   136,   572,  1076,   135,
    1097,  1112,   175,   176,   177,   581,  1117,   462,  1120,   610,
     135,   612,  1125,   953,  1129,  1132,  1134,   136,  1181,  1133,
     453,  1164,  1165,  1166,  1167,  1168,  1169,  1170,  1171,  1172,
    1135,   135,  1139,  1137,  1144,  1150,  1148,   746,  1151,  1158,
    1159,   453,  1162,   182,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,  1173,
    1182,   665,   552,   553,   554,   555,   556,   557,   558,   559,
     560,   561,   562,   563,    56,    56,  1175,  1183,   136,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
    1187,  1188,   135,  1195,  1196,  1190,  1197,  1202,  1203,  1207,
    1208,  1205,  1210,  1209,  1211,   136,  1212,    56,  1194,  1213,
     716,  1239,  1214,  1215,   720,   136,  1220,  1216,  1219,   135,
    1224,  1229,  1228,  1230,  1241,  1249,   635,  1242,  1248,   135,
    1247,  1253,  1254,  1255,   731,  1250,   733,   735,  1252,  1251,
     739,  1256,  1258,  1261,  1257,  1259,  1262,  1263,  1260,   943,
     337,   198,   939,  1238,   847,  1036,   246,   754,   755,  1110,
     759,   760,   761,   762,   763,   764,   765,   766,   973,   976,
     865,   778,   779,   780,   781,   782,   783,   784,  1186,   543,
     544,   545,   546,   547,   548,   549,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
     460,   339,   340,   341,   342,   376,   343,   344,   345,   707,
     346,   347,   348,  1024,  1118,   960,  1198,   913,   349,   670,
     250,   840,   326,   661,   163,   614,   504,   690,   468,   476,
     350,   351,   517,   901,   872,  1000,  1015,   839,   352,   518,
    1146,   631,   858,  1147,     0,     0,   994,     0,     0,   856,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   519,     0,     0,     0,
       0,     0,     0,     0,   520,     0,     0,   353,   886,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    89,     0,     0,     0,   521,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   354,   543,
     544,   545,   546,   547,   548,   549,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
       0,   914,     0,   522,     0,   915,     0,     0,     0,     0,
       0,     0,     0,     0,   920,     0,   922,     0,   924,     0,
       0,     0,   927,     0,     0,     0,     0,     0,     0,     0,
       0,   708,   936,   848,   937,     0,     0,     0,     0,     0,
     941,   355,   339,   340,   341,   342,     0,   343,   344,   345,
       0,   346,   347,   348,     0,   420,     0,     0,     0,   349,
     523,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   350,   351,   488,     0,     0,     0,     0,     0,   352,
     572,     0,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   356,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   295,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   353,     0,
       0,     0,     0,     0,   339,   340,   341,   342,     0,   343,
     344,   345,   992,   346,   347,   348,     0,     0,     0,   296,
       0,   349,     0,     0,     0,     0,     0,     0,     0,   354,
       0,     0,     0,   350,   351,     0,     0,     0,     0,     0,
       0,   352,   418,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     353,   169,     0,     0,     0,  1032,     0,  1033,     0,  1034,
       0,     0,   355,     0,     0,  1038,     0,     0,  1041,     0,
       0,     0,  1047,     0,     0,     0,     0,     0,     0,     0,
       0,   354,     0,     0,     0,     0,     0,     0,     1,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   356,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,     0,     0,
       0,     0,     0,     0,     0,     0,   339,   340,   341,   342,
       0,   343,   344,   345,   355,   346,   347,   348,     0,     0,
       0,     0,     0,   349,     0,     0,  1115,     0,     0,     0,
       0,     0,     0,     0,     0,   350,   351,   488,     0,     0,
       0,     0,     0,   352,     0,     0,     0,     0,     0,     0,
    1126,     0,     0,     0,     0,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   356,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
       0,     0,   353,  1142,     0,   339,   340,   341,   342,    28,
     343,   344,   345,     0,   346,   347,   348,     0,     0,     0,
       0,     0,   349,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   354,   350,   351,     0,     0,     0,     0,
    1180,     0,   352,     0,     0,     0,     0,     0,  1185,  1041,
       0,     0,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,     0,     0,     0,
       0,   353,     0,     0,     0,     0,     0,     0,   339,   340,
     341,   342,   664,   343,   344,   345,   355,   346,   347,   348,
       0,     0,     0,     0,     0,   349,   420,  1222,     0,     0,
       0,     0,   354,     0,     0,     0,     0,   350,   351,     0,
       0,     0,     0,     0,     0,   352,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   356,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,     0,     0,   353,     0,     0,     0,     0,     0,
       0,     0,     0,    28,     0,   355,   339,   340,   341,   342,
       0,   343,   344,   345,     0,   346,   347,   348,     0,     0,
       0,     0,     0,   349,     0,   354,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   350,   351,     0,     0,     0,
       0,     0,     0,   352,     0,     0,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   356,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   353,     0,     0,     0,     0,     0,   355,   339,
     340,   341,   342,     0,   343,   344,   345,     0,   346,   347,
     348,     0,     0,     0,     0,     0,   349,     0,     0,     0,
       0,     0,     0,   354,     0,     0,     0,   730,   350,   351,
       0,     0,     0,     0,     0,     0,   352,     0,     0,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   356,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   353,     0,     0,   339,   340,
     341,   342,     0,   343,   344,   345,   355,   346,   347,   348,
       0,     0,     0,     0,     0,   349,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   354,   350,   351,     0,
     732,     0,     0,     0,     0,   352,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   356,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,     0,     0,   353,     0,     0,     0,     0,     0,
       0,   339,   340,   341,   342,     0,   343,   344,   345,   355,
     346,   347,   348,     0,     0,     0,     0,     0,   349,     0,
       0,     0,     0,     0,     0,   354,     0,     0,     0,   734,
     350,   351,     0,     0,     0,     0,     0,     0,   352,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     356,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,     0,     0,   353,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   355,   545,
     546,   547,   548,   549,   550,   551,   552,   553,   554,   555,
     556,   557,   558,   559,   560,   561,   562,   563,   354,   544,
     545,   546,   547,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   562,   563,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   356,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   169,     0,     0,
       0,   355,     0,     0,     0,     0,     0,   170,   171,   543,
     544,   545,   546,   547,   548,   549,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
       0,     0,     0,     0,   133,     0,     0,     0,     0,     0,
       0,     0,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   356,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   339,   340,   341,
     342,   451,   343,   344,   345,     0,   346,   347,   348,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   767,   768,     0,     0,
     339,   340,   341,   342,   352,   343,   344,   345,     0,   346,
     347,   348,   172,   173,     0,     0,     0,     0,     0,   996,
       0,     0,     0,     0,     0,   133,     0,     0,     0,   767,
     768,     0,     0,     0,   910,     0,     0,   352,     0,     0,
       0,     0,     0,   353,     0,     0,     0,     0,     0,     0,
     133,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   221,     0,     0,   175,   176,     0,     0,
       0,     0,     0,     0,   354,   133,   353,     0,     0,   852,
       0,     0,     0,     0,     0,     0,     0,   451,     0,     0,
       0,     0,   672,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   133,     0,   465,   354,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,    89,     0,     0,     0,   355,     0,     0,
       0,     0,     0,     0,   461,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     355,   672,   169,     0,     0,     0,     0,     0,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   356,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   790,    89,     0,     0,     0,     0,     0,
       0,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   356,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   133,     0,     0,     0,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     169,     0,     0,     0,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   133,
       0,     0,     0,     0,     0,     0,     0,     0,   680,     0,
       0,     0,     0,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   169,     0,
       0,     0,     0,     0,     0,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     543,   544,   545,   546,   547,   548,   549,   550,   551,   552,
     553,   554,   555,   556,   557,   558,   559,   560,   561,   562,
     563,     0,     0,     0,     0,     0,     0,   543,   544,   545,
     546,   547,   548,   549,   550,   551,   552,   553,   554,   555,
     556,   557,   558,   559,   560,   561,   562,   563,     0,     0,
       0,     0,   880,     0,   543,   544,   545,   546,   547,   548,
     549,   550,   551,   552,   553,   554,   555,   556,   557,   558,
     559,   560,   561,   562,   563,     0,     0,     0,     0,     0,
     793,     0,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   794,     0,     0,
       0,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,     0,     0,     0,     0,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,     0,     0,     0,     0,     0,
    1046,     0,     0,     0,     0,     0,     0,     0,     0,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,     1,     0,     0,     0,   380,     0,
       0,     0,   381,   382,   383,     0,     0,     0,     0,     0,
       0,     4,   384,     6,  1074,     0,     0,     1,     0,     0,
       0,     2,     0,     0,     0,   381,   382,   383,     0,     0,
       0,     0,     0,   385,     4,   384,     6,     0,   800,   801,
     802,     0,     0,     0,     0,   803,     0,     0,     0,     0,
       0,    12,     0,   804,     0,     0,   385,   805,   806,     0,
     807,     0,     0,     0,     0,     0,   808,   386,     0,     0,
       0,     0,     0,     0,    12,     0,     0,   809,     0,     0,
       0,   387,     0,     0,    20,     0,     0,     0,     0,     0,
     386,     0,     0,     0,   810,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   387,     0,   811,    20,   812,     0,
     813,     0,     0,    24,     0,     0,     0,     0,     0,     0,
       0,     0,   814,     0,     0,   388,     0,     0,   389,     0,
       0,     0,     0,     0,     0,     0,    24,     0,     0,     0,
       0,   815,     0,     0,     0,     0,     0,     0,   388,    32,
       0,   389,     0,     0,   816,   817,   818,     0,     0,     0,
       0,     0,     0,     0,   819,   820,     0,     0,     0,     0,
       0,     0,    32,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   390,   543,   544,   545,   546,   547,   548,
     549,   550,   551,   552,   553,   554,   555,   556,   557,   558,
     559,   560,   561,   562,   563,     1,   390,     0,     0,     2,
       0,     0,     0,     0,     0,     0,     0,     0,     3,     0,
       0,     0,     4,     5,     6,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     7,     0,     0,     8,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     9,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,    11,
       0,     0,    12,    13,    14,     0,     0,    15,     0,    16,
       0,     0,     0,     0,    17,     0,    18,     0,     0,     0,
       0,     0,     0,     0,     0,    19,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    20,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
       0,     0,     0,    23,    24,     0,     0,    25,     0,    26,
      27,     0,     0,     0,     0,     0,    28,     0,     0,    29,
      30,     0,     0,     0,    31,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1193,     0,     0,     0,     0,     0,
      32,   543,   544,   545,   546,   547,   548,   549,   550,   551,
     552,   553,   554,   555,   556,   557,   558,   559,   560,   561,
     562,   563,   543,   544,   545,   546,   547,   548,   549,   550,
     551,   552,   553,   554,   555,   556,   557,   558,   559,   560,
     561,   562,   563,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   737,   543,   544,   545,   546,   547,   548,
     549,   550,   551,   552,   553,   554,   555,   556,   557,   558,
     559,   560,   561,   562,   563,   543,   544,   545,   546,   547,
     548,   549,   550,   551,   552,   553,   554,   555,   556,   557,
     558,   559,   560,   561,   562,   563,     0,   738,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   921,   543,
     544,   545,   546,   547,   548,   549,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     923,     0,     0,   822,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   925,   543,   544,   545,   546,   547,   548,   549,   550,
     551,   552,   553,   554,   555,   556,   557,   558,   559,   560,
     561,   562,   563,     0,     0,   749,   543,   544,   545,   546,
     547,   548,   549,   550,   551,   552,   553,   554,   555,   556,
     557,   558,   559,   560,   561,   562,   563,     0,     0,   928,
     543,   544,   545,   546,   547,   548,   549,   550,   551,   552,
     553,   554,   555,   556,   557,   558,   559,   560,   561,   562,
     563,     0,     0,  1045,   543,   544,   545,   546,   547,   548,
     549,   550,   551,   552,   553,   554,   555,   556,   557,   558,
     559,   560,   561,   562,   563
};

static const yytype_int16 yycheck[] =
{
       2,   448,     4,     5,     6,   215,     1,   293,   380,   333,
     254,    13,    14,   742,   258,    17,    32,     0,   375,    17,
      22,    29,     0,     1,    22,    13,    14,    29,   352,    29,
     210,   211,   212,     4,     5,     6,   639,   613,    41,   477,
      41,     0,   219,     0,    13,    14,   124,    22,    54,   193,
      41,    24,   130,  1019,    41,    82,    28,    81,   107,     0,
     171,  1017,    41,    10,    41,     0,    40,   178,     0,   219,
     181,   131,    24,    20,   822,    85,   219,    15,    16,    17,
      41,   146,   116,   147,   162,   147,     0,    41,    40,    78,
      96,    41,    41,   219,   652,    41,   114,   219,    41,    41,
     114,     4,    41,   202,   852,   219,   105,   147,    41,    30,
      41,   219,   201,   201,   651,   219,   107,   137,   202,   722,
      40,   123,    41,   219,     3,   662,   146,   186,    20,   113,
     132,  1097,     5,     9,   219,   123,   219,   219,   105,    12,
     239,    40,   219,   219,   132,   219,   180,   204,   182,    96,
      26,   240,   240,   137,   123,   186,   240,     8,   160,    10,
      11,   114,   146,   132,   172,   237,   376,   175,   146,    20,
     172,   160,   160,   175,   208,   240,   240,   321,   240,   737,
     240,   910,   911,   327,   194,   196,    89,   331,   107,   146,
     239,   160,   303,   114,   113,   118,   307,   171,   309,    43,
     240,   203,   226,   202,    96,   207,   554,   451,   155,  1175,
     148,   146,     3,   240,   146,   203,   218,  1173,    43,   207,
     218,   194,   240,   206,   124,   233,   240,   305,   206,   201,
     407,   233,   146,   239,   203,   202,   219,   240,   207,   240,
     242,   219,   194,   218,   246,   993,   324,   206,   239,   206,
     688,   171,   254,   240,   242,   257,   258,   407,   246,   259,
     219,   240,   219,   240,   407,   206,   254,   214,   270,   257,
     258,   206,   171,   242,   206,   277,   254,   246,   219,   240,
     258,   407,   270,   202,   219,   407,   240,   219,   257,   277,
     240,   240,   206,   407,   240,   273,   304,   240,   240,   407,
     308,   240,   304,   407,   312,   219,   308,   240,   866,   240,
     312,   407,   312,   315,   316,   317,   273,   875,   876,   877,
     239,   323,   407,    20,   407,   407,  1074,   315,   316,   317,
     407,   407,  1080,   407,   117,   323,   127,    68,   273,   123,
     240,   273,   186,   102,     5,   126,   315,   316,   317,   211,
     212,   213,   196,    14,   323,   193,    76,   352,   139,   273,
     338,   186,   506,   507,     0,   209,   444,   131,   512,   100,
      82,   196,   388,   202,   352,   114,   354,   980,   380,   457,
     380,   389,   384,   959,   209,   154,   497,   389,    72,   389,
     501,   174,   370,   371,   372,    41,   474,   475,   137,    96,
     183,   184,   418,    41,   382,    40,   338,   146,    56,   757,
      15,    16,    17,   384,   803,   174,   206,   207,    45,   767,
     768,   707,   142,     8,   407,    10,    11,   671,  1031,   407,
     819,   820,   676,     4,    66,     3,    41,    69,    45,   417,
     797,   798,   799,    40,  1192,  1193,   127,   771,   407,   451,
     407,   453,   233,   455,    66,     4,   451,    69,   178,    28,
      29,    30,    97,   451,   127,   453,   407,   455,   197,  1198,
     102,   181,   407,   451,   846,   407,   186,  1225,   105,   106,
     482,   161,   484,   930,   149,   417,   196,   489,   935,  1092,
     102,   489,   196,   407,   482,   130,   484,   114,   105,   106,
      40,   240,    42,   851,   216,   217,   218,     3,   220,    42,
     222,    90,    91,   482,    42,   484,   115,   113,   230,   231,
     232,   529,   530,   235,   159,    14,   158,   529,   530,   128,
     239,   240,   137,   132,   542,   530,   171,   136,   141,   142,
     542,   146,   240,   148,   677,   678,   158,    43,   147,     3,
     566,  1154,   530,   186,   229,   757,   147,   565,   113,   229,
     195,   137,   238,   565,   163,   767,   768,   370,   371,   372,
       4,     5,     6,    40,     8,     9,    10,   555,    12,    13,
      14,   370,   371,   372,     3,   236,   564,   205,   203,    77,
      86,    87,    60,    61,    62,    63,    64,   195,   946,   947,
     948,   949,   950,   951,   952,   953,   113,   564,   431,   432,
     433,     5,   792,   985,   986,   987,   988,    65,   634,   115,
     598,    40,   118,    65,    14,     5,    65,   999,   144,   564,
     638,   185,   564,    40,    20,   126,   638,   645,   646,   647,
      30,   649,   201,   645,   646,   647,   648,   649,   996,   851,
     564,   895,   660,   240,   110,   151,    46,    47,   660,   110,
     648,   663,   240,   131,    14,   194,   668,   240,   114,   671,
     672,     5,    40,   169,   676,   663,   172,    40,   680,   648,
     668,   683,   684,   671,   672,    40,   115,   687,   676,    40,
     186,   187,   680,   671,   663,   683,   684,    40,   676,   128,
    1072,  1049,    24,   132,   706,     4,   708,   136,   710,   205,
     708,     5,    84,   209,   683,   684,   174,    40,   706,    40,
       3,    40,   710,   219,    60,    61,    62,    63,    64,   225,
    1078,    42,   710,    40,   163,   215,    42,   706,    42,   173,
     113,   221,   238,   223,   946,   947,   948,   949,   950,   951,
     952,   953,   171,    14,   234,   239,   239,    40,    32,    33,
      34,    35,    36,    37,    38,  1137,    32,    33,    34,    35,
      36,    37,    38,   239,   239,   239,   771,   239,   114,   239,
     758,   106,   239,   194,  1028,    32,    33,    34,    35,    36,
      37,    38,  1140,   771,   996,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,   239,   125,   115,   822,     5,   846,  1049,   239,   132,
     133,   239,    19,   240,   239,   239,   128,   239,  1182,    24,
     132,   239,    24,    24,   136,    32,    33,    34,    35,    36,
      37,    38,    85,   156,   852,   147,  1078,    68,    69,   144,
      73,    74,    75,   166,   167,   168,   144,    80,   240,    77,
     110,   163,    40,   895,    77,    88,    87,   147,     3,    92,
      93,   240,    95,   240,   906,   194,    97,   895,   101,     3,
      41,   102,    41,   104,    14,     4,    40,   895,   906,   112,
     194,   929,    40,   240,    41,   933,   209,   929,   194,   120,
      30,   933,    41,   177,   198,   240,   129,   906,  1140,   130,
     202,    41,    19,    40,    17,   240,    46,    47,   141,   239,
     143,   137,   145,   239,   932,    32,    33,    34,    35,    36,
      37,    38,   208,     3,   157,   106,   239,    30,   159,   239,
     239,   239,   239,    13,    14,     5,   239,     4,   224,   239,
     171,     4,   239,   176,   239,   985,   986,   987,   988,    34,
      35,    36,    37,    38,   239,    24,   189,   190,   191,   999,
     932,   239,   239,   239,   195,    24,   199,   200,  1010,    14,
    1018,  1019,  1010,    14,    14,   993,  1018,  1019,   150,  1021,
      41,  1231,   132,     4,    41,   240,  1028,    34,    35,    36,
      37,    38,    41,  1021,     5,    41,    24,    40,    40,    38,
    1028,   239,   153,    41,    40,    40,    40,   240,   131,   131,
    1028,   131,    49,   131,   300,     5,   302,    32,    33,    34,
      35,    36,    37,    38,    82,   311,     4,   313,     4,   194,
      54,    40,  1072,   319,    49,    65,   134,    70,   118,   119,
      32,    33,    34,    35,    36,    37,    38,   333,    41,  1097,
      41,   106,   239,     5,     5,  1097,  1074,    49,     5,     5,
     239,     5,  1080,   349,   350,   351,   352,   353,   170,   131,
      54,    54,   152,    54,   106,  1117,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,  1128,   373,   106,  1117,
      40,   210,   172,   173,   174,   381,   240,  1137,   121,   385,
    1128,   387,    40,    38,    41,    41,   239,  1149,  1117,    41,
    1128,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,
      48,  1149,    79,   131,   127,    24,   106,  1175,   228,    10,
      10,  1149,    24,  1175,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   240,
      77,   447,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,  1192,  1193,    40,   122,  1220,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
       5,   239,  1220,   127,   106,   239,   127,    24,    10,     4,
       4,   134,     4,   180,     4,  1247,     5,  1225,   239,     5,
     496,  1220,     5,     5,   500,  1257,    42,     8,   240,  1247,
      41,   127,   239,   239,   106,    49,   240,   239,   106,  1257,
     240,    49,   239,     5,   520,    24,   522,   523,  1247,    24,
     526,     4,   239,    24,    42,   227,    41,     5,  1257,   758,
     206,    30,   751,  1219,   609,   929,   131,   543,   544,  1018,
     546,   547,   548,   549,   550,   551,   552,   553,   813,   817,
     645,   557,   558,   559,   560,   561,   562,   563,  1125,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
     258,     3,     4,     5,     6,   214,     8,     9,    10,   486,
      12,    13,    14,   905,  1027,   796,  1149,   711,    20,   451,
     137,   607,   195,   444,    25,   389,   312,   475,   271,   277,
      32,    33,    34,   687,   650,   860,   879,   598,    40,    41,
    1082,   407,   638,  1082,    -1,    -1,   849,    -1,    -1,   635,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    -1,    -1,    79,   664,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,    -1,    -1,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,   717,    -1,   135,    -1,   721,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   730,    -1,   732,    -1,   734,    -1,
      -1,    -1,   738,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,   748,   202,   750,    -1,    -1,    -1,    -1,    -1,
     756,   173,     3,     4,     5,     6,    -1,     8,     9,    10,
      -1,    12,    13,    14,    -1,   771,    -1,    -1,    -1,    20,
     192,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,    40,
     796,    -1,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,    68,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    10,   848,    12,    13,    14,    -1,    -1,    -1,   100,
      -1,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   110,
      -1,    -1,    -1,    32,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
      79,     3,    -1,    -1,    -1,   921,    -1,   923,    -1,   925,
      -1,    -1,   173,    -1,    -1,   931,    -1,    -1,   934,    -1,
      -1,    -1,   938,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   110,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
      -1,     8,     9,    10,   173,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    20,    -1,    -1,  1022,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    33,    34,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,
    1046,    -1,    -1,    -1,    -1,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
      -1,    -1,    79,  1079,    -1,     3,     4,     5,     6,   171,
       8,     9,    10,    -1,    12,    13,    14,    -1,    -1,    -1,
      -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   110,    32,    33,    -1,    -1,    -1,    -1,
    1116,    -1,    40,    -1,    -1,    -1,    -1,    -1,  1124,  1125,
      -1,    -1,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,     8,     9,    10,   173,    12,    13,    14,
      -1,    -1,    -1,    -1,    -1,    20,  1182,  1183,    -1,    -1,
      -1,    -1,   110,    -1,    -1,    -1,    -1,    32,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   171,    -1,   173,     3,     4,     5,     6,
      -1,     8,     9,    10,    -1,    12,    13,    14,    -1,    -1,
      -1,    -1,    -1,    20,    -1,   110,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,   173,     3,
       4,     5,     6,    -1,     8,     9,    10,    -1,    12,    13,
      14,    -1,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,
      -1,    -1,    -1,   110,    -1,    -1,    -1,   114,    32,    33,
      -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    10,   173,    12,    13,    14,
      -1,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   110,    32,    33,    -1,
     114,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    10,   173,
      12,    13,    14,    -1,    -1,    -1,    -1,    -1,    20,    -1,
      -1,    -1,    -1,    -1,    -1,   110,    -1,    -1,    -1,   114,
      32,    33,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   173,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,   110,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,
      -1,   173,    -1,    -1,    -1,    -1,    -1,    13,    14,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,     3,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,     3,     4,     5,
       6,    40,     8,     9,    10,    -1,    12,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    33,    -1,    -1,
       3,     4,     5,     6,    40,     8,     9,    10,    -1,    12,
      13,    14,   118,   119,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,    -1,    32,
      33,    -1,    -1,    -1,   103,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
       3,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    -1,    -1,   172,   173,    -1,    -1,
      -1,    -1,    -1,    -1,   110,     3,    79,    -1,    -1,   188,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,    -1,    34,   110,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,     3,    -1,    -1,    -1,   173,    -1,    -1,
      -1,    -1,    -1,    -1,    14,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     173,    70,     3,    -1,    -1,    -1,    -1,    -1,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,    34,     3,    -1,    -1,    -1,    -1,    -1,
      -1,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,     3,    -1,    -1,    -1,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
       3,    -1,    -1,    -1,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,     3,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,
      -1,    -1,    -1,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,     3,    -1,
      -1,    -1,    -1,    -1,    -1,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,   155,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      71,    -1,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,    98,    -1,    -1,
      -1,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,    -1,    -1,    -1,    -1,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,    -1,    -1,    -1,    -1,    -1,
     188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,    40,    -1,    -1,    -1,    44,    -1,
      -1,    -1,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,
      -1,    57,    58,    59,   188,    -1,    -1,    40,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    79,    57,    58,    59,    -1,    73,    74,
      75,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,
      -1,    97,    -1,    88,    -1,    -1,    79,    92,    93,    -1,
      95,    -1,    -1,    -1,    -1,    -1,   101,   113,    -1,    -1,
      -1,    -1,    -1,    -1,    97,    -1,    -1,   112,    -1,    -1,
      -1,   127,    -1,    -1,   130,    -1,    -1,    -1,    -1,    -1,
     113,    -1,    -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   127,    -1,   141,   130,   143,    -1,
     145,    -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   157,    -1,    -1,   171,    -1,    -1,   174,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   159,    -1,    -1,    -1,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,   171,   195,
      -1,   174,    -1,    -1,   189,   190,   191,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   199,   200,    -1,    -1,    -1,    -1,
      -1,    -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   239,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    40,   239,    -1,    -1,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    -1,
      -1,    -1,    57,    58,    59,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    72,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    83,    -1,
      -1,    -1,    87,    -1,    -1,    -1,    -1,    -1,    -1,    94,
      -1,    -1,    97,    98,    99,    -1,    -1,   102,    -1,   104,
      -1,    -1,    -1,    -1,   109,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   130,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,   158,   159,    -1,    -1,   162,    -1,   164,
     165,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,   174,
     175,    -1,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,    -1,    -1,
     195,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    70,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,   114,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    41,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    41,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    41,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    40,    44,    53,    57,    58,    59,    69,    72,    83,
      87,    94,    97,    98,    99,   102,   104,   109,   111,   120,
     130,   140,   154,   158,   159,   162,   164,   165,   171,   174,
     175,   179,   195,   242,   243,   244,   263,   264,   267,   273,
     275,   276,   284,   285,   286,   287,   289,   290,   291,   292,
     308,   311,   312,   315,   317,   322,   323,   324,   325,   326,
     328,   359,   362,   363,   370,   375,   377,   379,   382,   386,
     387,   388,   389,   392,   397,   399,   402,   410,   413,   414,
     415,   421,   434,   435,   459,   460,   461,   323,   324,     3,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   274,   432,   433,     3,   274,
     274,   274,   181,   186,   196,   204,   384,   384,    43,   186,
     196,   209,   114,     3,   355,   430,   433,   355,    43,   186,
     196,   209,   186,   400,   432,   237,   361,    68,    69,    87,
      97,   102,   104,   120,   130,   159,   171,   195,   393,   394,
     186,   400,   196,   393,   384,   118,   390,   123,   331,     3,
      13,    14,   118,   119,   152,   172,   173,   174,   257,   403,
     404,   429,   433,     3,    43,    86,    87,   115,   118,   151,
     169,   172,   186,   187,   209,   219,   225,   238,   258,   366,
     369,   431,   193,   331,     0,   107,   239,   131,   202,   316,
      15,    16,    17,   148,   344,   345,   113,   341,   154,    72,
     438,    41,    41,    56,    40,    60,    61,    62,    63,    64,
     114,   288,   117,   174,   183,   184,   422,   355,     4,   373,
       3,   127,   293,   371,   372,   373,   293,   355,     4,   368,
     368,     3,   127,   127,   309,   373,   376,   309,   309,   197,
     411,   263,   264,   315,   322,   360,   161,   395,   196,   149,
     147,   240,   355,   114,   373,   380,   381,   147,   398,   206,
     207,   391,     3,    40,   125,   132,   133,   156,   166,   167,
     168,   209,   332,   333,   334,    68,   100,   350,    24,   194,
     406,    40,   406,   429,    42,   113,   378,   429,    42,    14,
     405,   406,   240,   406,     3,    30,   114,   186,    28,   201,
     367,   229,   367,   147,   113,   365,   378,   229,   137,   364,
     238,   236,   205,    40,   203,   385,   355,   244,   355,     3,
       4,     5,     6,     8,     9,    10,    12,    13,    14,    20,
      32,    33,    40,    79,   110,   173,   224,   246,   247,   248,
     250,   252,   257,   258,   259,   260,   323,   429,   430,   433,
     350,   350,   350,    77,   137,   146,   330,   341,   195,   400,
      44,    48,    49,    50,    58,    79,   113,   127,   171,   174,
     239,   263,   264,   273,   275,   277,   278,   279,   280,   281,
     282,   283,   291,   315,   322,   327,   439,   440,   441,   445,
     446,   447,   451,   455,   456,   457,   458,   113,    41,   245,
     250,     5,     5,    65,   423,   424,   429,   144,   185,    90,
      91,    66,    69,   102,   158,   416,   417,   126,   139,   233,
     383,    40,    20,   355,   240,   126,   355,   201,   329,   110,
     110,    40,   310,   323,   354,   355,   356,   240,   313,   355,
     310,    14,   257,   407,   412,    34,   396,   430,   394,   263,
     264,   315,   322,   401,   194,   240,   396,   114,     5,    40,
     335,    40,    40,    40,    40,   124,   240,   334,    34,   250,
     351,   352,   250,     5,    14,   250,   406,   429,   373,    24,
     406,   429,   406,   250,   404,   250,   355,   355,   355,     4,
     250,   367,   355,    89,   367,     5,   367,    34,    41,    68,
      76,   100,   135,   192,   245,   250,   262,   342,    84,   174,
      40,   318,   322,   250,   250,   250,   245,   250,   323,   250,
     253,   323,    42,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    40,    42,   171,   323,   326,   328,
     326,   326,   250,   346,   347,     5,    12,   339,   339,   341,
     412,   250,   263,   264,   273,   275,   277,   278,   279,   280,
     281,   282,   283,   291,   315,   322,   327,   442,   443,   444,
     446,   447,   451,   455,   456,   457,   458,   202,   239,   274,
     250,    14,   250,   331,   403,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     106,   441,   322,   331,    41,   240,   194,     5,   240,    24,
      24,    24,   144,   144,    85,   418,   418,   418,   194,   418,
     240,    77,    14,    30,    41,    46,    47,   436,   437,   110,
      40,   372,    77,   147,     7,   250,   316,     3,   323,   354,
     356,   240,    70,   430,   115,   128,   132,   136,   163,   357,
      70,   430,   373,   147,   240,    82,   425,   240,   194,   373,
     380,   376,     3,    41,   240,   211,   212,   213,   336,   337,
     355,   116,   180,   182,   208,   338,   355,   333,    70,   432,
     114,   240,   330,   340,    41,    41,   250,   406,     4,   374,
     250,   406,    40,   408,   367,   367,   367,    40,   240,    41,
     114,   250,   114,   250,   114,   250,    41,    70,   114,   250,
      41,   177,   265,   266,   429,   319,   429,   198,   240,    41,
     202,   254,   255,   429,   250,   250,    28,    29,    30,   250,
     250,   250,   250,   250,   250,   250,   250,    32,    33,   248,
     249,    40,   251,   323,     8,    10,    11,    20,   250,   250,
     250,   250,   250,   250,   250,   261,   263,   264,   315,   322,
      34,   429,   331,    71,    98,   348,   240,   146,   240,   137,
      73,    74,    75,    80,    88,    92,    93,    95,   101,   112,
     129,   141,   143,   145,   157,   176,   189,   190,   191,   199,
     200,   297,    52,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   239,   239,   239,   239,   239,   239,   106,   444,
     250,    60,    61,    62,    63,    64,   131,   288,   202,   452,
     453,    30,   188,   352,   239,   331,   250,     5,   424,    14,
     247,     4,     4,    24,    24,   296,   429,   429,   429,   355,
     429,   158,   417,   374,   297,    14,    14,    14,    41,   240,
     155,   294,   295,   296,   374,   355,   250,    41,   354,   430,
     150,   358,   354,   358,   358,   132,   430,   314,   355,   355,
       4,   407,   376,    41,    41,   240,   240,    41,   355,   432,
     103,   353,   354,   351,   250,   250,   247,   409,    41,     5,
     250,   114,   250,   114,   250,   114,   297,   250,    41,   240,
     329,    24,    41,   240,    40,   320,   250,   250,   105,   255,
     256,   250,   249,   251,   249,   249,    19,    32,    33,    34,
      35,    36,    37,    38,   245,     8,    10,    11,    41,   350,
     347,   339,   339,   339,    40,   302,   302,    40,   298,   153,
     300,    40,   299,   298,    40,   301,   301,   302,   302,    54,
      96,   239,   442,    49,   239,   131,   131,   131,   131,     5,
     412,    65,   250,   105,   453,   454,    55,   249,   442,   131,
     425,     4,     4,   297,   102,   174,   420,    78,   160,   419,
     194,    54,   297,   297,   297,   437,   134,    41,   240,    40,
     268,    70,   147,   354,   337,   355,    41,   329,   240,   329,
      41,   240,   250,   250,   250,    41,   266,   316,   250,   322,
     429,   250,   321,   240,   316,    41,   188,   250,   106,    19,
     249,   249,   249,   249,   249,   249,   249,   249,    41,   352,
       5,     5,     5,     5,   239,   247,   106,   239,   412,   412,
     412,   412,   131,     5,   188,   442,   106,   249,   194,    45,
     105,   106,   448,   449,   450,   412,   170,   426,   303,    20,
      96,    20,    96,   432,    54,    54,    54,    40,    82,   216,
     217,   218,   220,   222,   230,   231,   232,   235,   305,   306,
     295,   319,   210,   269,   430,   250,   147,   240,   340,   354,
     121,   343,   247,    41,   240,    40,   250,   249,   114,    41,
      41,   240,    41,    41,   239,    48,   412,   131,   442,    79,
     194,   249,   250,   442,   127,   106,   449,   450,   106,   114,
      24,   228,    10,    20,    96,   155,   214,   304,    10,    10,
     247,   319,    24,   307,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   240,    41,    40,   270,   271,   272,   306,
     250,   355,    77,   122,   349,   250,   321,     5,   239,   412,
     239,   249,    49,   188,   239,   127,   106,   127,   353,    76,
     142,   178,    24,    10,   247,   134,    41,     4,     4,   180,
       4,     4,     5,     5,     5,     5,     8,   306,   319,   240,
      42,   245,   250,    41,    41,    49,   442,   442,   239,   127,
     239,   329,   215,   221,   223,   234,   427,    41,   272,   355,
     442,   106,   239,   341,    81,   226,   428,   240,   106,    49,
      24,    24,   355,    49,   239,     5,     4,    42,   239,   227,
     355,    24,    41,     5
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (&yylloc, result, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location, result); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ParseResult* result)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, result)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    ParseResult* result;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
  YYUSE (result);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ParseResult* result)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp, result)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    ParseResult* result;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, result);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, ParseResult* result)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule, result)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
    ParseResult* result;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       , result);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule, result); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, ParseResult* result)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp, result)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
    ParseResult* result;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (result);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 3: /* "NAME" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 4: /* "STRING" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 5: /* "INTNUM" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 6: /* "DATE_VALUE" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 7: /* "HINT_VALUE" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 8: /* "BOOL" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 9: /* "APPROXNUM" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 10: /* "NULLX" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 11: /* "UNKNOWN" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 12: /* "QUESTIONMARK" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 13: /* "SYSTEM_VARIABLE" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 14: /* "TEMP_VARIABLE" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 242: /* "sql_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 243: /* "stmt_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 244: /* "stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 245: /* "expr_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 246: /* "column_ref" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 247: /* "expr_const" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 248: /* "simple_expr" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 249: /* "arith_expr" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 250: /* "expr" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 251: /* "in_expr" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 252: /* "case_expr" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 253: /* "case_arg" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 254: /* "when_clause_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 255: /* "when_clause" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 256: /* "case_default" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 257: /* "array_expr" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 258: /* "func_expr" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 259: /* "when_func" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 260: /* "when_func_name" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 261: /* "when_func_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 262: /* "distinct_or_all" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 263: /* "delete_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 264: /* "update_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 265: /* "update_asgn_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 266: /* "update_asgn_factor" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 267: /* "create_index_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 268: /* "opt_index_columns" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 269: /* "opt_storing" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 270: /* "opt_storing_columns" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 271: /* "opt_index_option_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 272: /* "index_option" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 273: /* "cursor_declare_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 274: /* "cursor_name" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 275: /* "cursor_open_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 276: /* "cursor_fetch_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 277: /* "cursor_fetch_into_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 278: /* "cursor_fetch_next_into_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 279: /* "cursor_fetch_first_into_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 280: /* "cursor_fetch_prior_into_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 281: /* "cursor_fetch_last_into_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 282: /* "cursor_fetch_absolute_into_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 283: /* "cursor_fetch_relative_into_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 284: /* "fetch_prior_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 285: /* "fetch_first_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 286: /* "fetch_last_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 287: /* "fetch_relative_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 288: /* "next_or_prior" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 289: /* "fetch_absolute_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 290: /* "fetch_fromto_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 291: /* "cursor_close_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 292: /* "create_table_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 293: /* "opt_if_not_exists" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 294: /* "table_element_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 295: /* "table_element" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 296: /* "column_definition" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 297: /* "data_type" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 298: /* "opt_decimal" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 299: /* "opt_float" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 300: /* "opt_precision" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 301: /* "opt_time_precision" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 302: /* "opt_char_length" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 303: /* "opt_column_attribute_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 304: /* "column_attribute" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 305: /* "opt_table_option_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 306: /* "table_option" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 307: /* "opt_equal_mark" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 308: /* "drop_table_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 309: /* "opt_if_exists" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 310: /* "table_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 311: /* "truncate_table_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 312: /* "drop_index_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 313: /* "index_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 314: /* "table_name" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 315: /* "insert_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 316: /* "opt_when" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 317: /* "replace_or_insert" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 318: /* "opt_insert_columns" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 319: /* "column_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 320: /* "insert_vals_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 321: /* "insert_vals" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 322: /* "select_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 323: /* "select_with_parens" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 324: /* "select_no_parens" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 325: /* "no_table_select" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 326: /* "select_clause" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 327: /* "select_into_clause" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 328: /* "simple_select" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 329: /* "opt_where" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 330: /* "select_limit" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 331: /* "opt_hint" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 332: /* "opt_hint_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 333: /* "hint_options" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 334: /* "hint_option" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 335: /* "opt_comma_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 336: /* "join_op_type_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 337: /* "join_op_type" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 339: /* "limit_expr" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 340: /* "opt_select_limit" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 341: /* "opt_for_update" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 342: /* "parameterized_trim" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 343: /* "opt_groupby" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 344: /* "opt_order_by" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 345: /* "order_by" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 346: /* "sort_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 347: /* "sort_key" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 348: /* "opt_asc_desc" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 349: /* "opt_having" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 350: /* "opt_distinct" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 351: /* "projection" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 352: /* "select_expr_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 353: /* "from_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 354: /* "table_factor" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 355: /* "relation_factor" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 356: /* "joined_table" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 357: /* "join_type" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 358: /* "join_outer" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 359: /* "explain_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 360: /* "explainable_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 361: /* "opt_verbose" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 362: /* "show_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 363: /* "lock_table_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 364: /* "opt_limit" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 365: /* "opt_for_grant_user" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 367: /* "opt_show_condition" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 368: /* "opt_like_condition" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 370: /* "create_user_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 371: /* "user_specification_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 372: /* "user_specification" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 373: /* "user" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 374: /* "password" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 375: /* "drop_user_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 376: /* "user_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 377: /* "set_password_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 378: /* "opt_for_user" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 379: /* "rename_user_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 380: /* "rename_info" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 381: /* "rename_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 382: /* "lock_user_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 383: /* "lock_spec" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 384: /* "opt_work" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 386: /* "begin_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 387: /* "commit_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 388: /* "rollback_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 389: /* "kill_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 390: /* "opt_is_global" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 391: /* "opt_flag" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 392: /* "grant_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 393: /* "priv_type_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 394: /* "priv_type" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 395: /* "opt_privilege" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 396: /* "priv_level" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 397: /* "revoke_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 398: /* "opt_on_priv_level" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 399: /* "prepare_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 400: /* "stmt_name" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 401: /* "preparable_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 402: /* "variable_set_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 403: /* "var_and_val_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 404: /* "var_and_val" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 405: /* "var_and_array_val" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 406: /* "to_or_eq" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 407: /* "argument" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 408: /* "array_vals_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 409: /* "array_val_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 410: /* "execute_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 411: /* "opt_using_args" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 412: /* "argument_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 413: /* "deallocate_prepare_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 414: /* "deallocate_or_drop" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 415: /* "alter_table_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 416: /* "alter_column_actions" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 417: /* "alter_column_action" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 418: /* "opt_column" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 420: /* "alter_column_behavior" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 421: /* "alter_system_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 422: /* "opt_force" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 423: /* "alter_system_actions" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 424: /* "alter_system_action" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 425: /* "opt_comment" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 427: /* "server_type" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 428: /* "opt_cluster_or_address" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 429: /* "column_name" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 430: /* "relation_name" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 431: /* "function_name" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 432: /* "column_label" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 434: /* "create_procedure_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 435: /* "proc_decl" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 436: /* "proc_parameter_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 437: /* "proc_parameter" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 438: /* "proc_block" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 439: /* "proc_sect" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 440: /* "proc_stmts" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 441: /* "proc_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 442: /* "control_sect" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 443: /* "control_stmts" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 444: /* "control_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 445: /* "stmt_declare" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 446: /* "stmt_assign" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 447: /* "stmt_if" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 448: /* "stmt_elsifs" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 449: /* "stmt_elsif" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 450: /* "stmt_else" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 451: /* "stmt_case" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 452: /* "case_when_list" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 453: /* "case_when" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 454: /* "case_else" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 455: /* "stmt_loop" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 456: /* "stmt_while" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 457: /* "stmt_null" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 458: /* "stmt_exit" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 459: /* "exec_procedure_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 460: /* "drop_procedure_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;
      case 461: /* "show_procedure_stmt" */

	{destroy_tree((yyvaluep->node));};

	break;

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (ParseResult* result);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (ParseResult* result)
#else
int
yyparse (result)
    ParseResult* result;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[2];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_STMT_LIST, (yyvsp[(1) - (2)].node));
      result->result_tree_ = (yyval.node);
      YYACCEPT;
    ;}
    break;

  case 3:

    {
      if ((yyvsp[(3) - (3)].node) != NULL)
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
      else
        (yyval.node) = (yyvsp[(1) - (3)].node);
    ;}
    break;

  case 4:

    {
      (yyval.node) = ((yyvsp[(1) - (1)].node) != NULL) ? (yyvsp[(1) - (1)].node) : NULL;
    ;}
    break;

  case 5:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 6:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 7:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 8:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 9:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 10:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 11:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 12:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 13:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 14:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 15:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 16:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 17:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 18:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 19:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 20:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 21:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 22:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 23:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 24:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 25:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 26:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 27:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 28:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 29:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 30:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 31:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 32:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 33:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 34:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 35:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 36:

    { (yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 37:

    { (yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 38:

    { (yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 39:

    { (yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 40:

    { (yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 41:

    { (yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 42:

    { (yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 43:

    { (yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 44:

    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 45:

    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 46:

    {(yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 47:

    { (yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 48:

    { (yyval.node) = NULL; ;}
    break;

  case 49:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 50:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 51:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 52:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_NAME_FIELD, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
      dup_expr_string((yyval.node)->str_value_, result, (yylsp[(3) - (3)]).first_column, (yylsp[(3) - (3)]).last_column);
    ;}
    break;

  case 53:

    {
      ParseNode *node = NULL;
      malloc_terminal_node(node, result->malloc_pool_, T_STAR);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_NAME_FIELD, 2, (yyvsp[(1) - (3)].node), node);
    ;}
    break;

  case 54:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 55:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 56:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 57:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 58:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 59:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 60:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 61:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 62:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 63:

    { (yyvsp[(3) - (3)].node)->type_ = T_SYSTEM_VARIABLE; (yyval.node) = (yyvsp[(3) - (3)].node); ;}
    break;

  case 64:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 65:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 66:

    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 67:

    {
      ParseNode *node = NULL;
      malloc_non_terminal_node(node, result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(2) - (5)].node), (yyvsp[(4) - (5)].node));
      merge_nodes((yyval.node), result->malloc_pool_, T_EXPR_LIST, node);
    ;}
    break;

  case 68:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
      /*
      yyerror(&@1, result, "CASE expression is not supported yet!");
      YYABORT;
      */
    ;}
    break;

  case 69:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 70:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 71:

    {
    	(yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 72:

    {
    	malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_EXISTS, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 73:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 74:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 75:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_POS, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 76:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_NEG, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 77:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_ADD, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 78:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_MINUS, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 79:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_MUL, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 80:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_DIV, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 81:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_REM, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 82:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_POW, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 83:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_MOD, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 84:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 85:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_POS, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 86:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_NEG, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 87:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_ADD, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 88:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_MINUS, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 89:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_MUL, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 90:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_DIV, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 91:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_REM, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 92:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_POW, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 93:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_MOD, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 94:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_LE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 95:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_LT, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 96:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_EQ, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 97:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_GE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 98:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_GT, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 99:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_NE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 100:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_LIKE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 101:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_NOT_LIKE, 2, (yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node)); ;}
    break;

  case 102:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_AND, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 103:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_OR, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 104:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_NOT, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 105:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_IS, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 106:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_IS_NOT, 2, (yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 107:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_IS, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 108:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_IS_NOT, 2, (yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 109:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_IS, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 110:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_IS_NOT, 2, (yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 111:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_BTW, 3, (yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node));
    ;}
    break;

  case 112:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_NOT_BTW, 3, (yyvsp[(1) - (6)].node), (yyvsp[(4) - (6)].node), (yyvsp[(6) - (6)].node));
    ;}
    break;

  case 113:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_IN, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 114:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_NOT_IN, 2, (yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 115:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_CNN, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 116:

    {
    	(yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 117:

    { merge_nodes((yyval.node), result->malloc_pool_, T_EXPR_LIST, (yyvsp[(2) - (3)].node)); ;}
    break;

  case 118:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_WHEN_LIST, (yyvsp[(3) - (5)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CASE, 3, (yyvsp[(2) - (5)].node), (yyval.node), (yyvsp[(4) - (5)].node));
    ;}
    break;

  case 119:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 120:

    { (yyval.node) = NULL; ;}
    break;

  case 121:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 122:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 123:

    {
    	malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_WHEN, 2, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 124:

    { (yyval.node) = (yyvsp[(2) - (2)].node); ;}
    break;

  case 125:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_NULL); ;}
    break;

  case 126:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ARRAY, 2, (yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node));
    ;}
    break;

  case 127:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ARRAY, 2, (yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node));
    ;}
    break;

  case 128:

    {
      if (strcasecmp((yyvsp[(1) - (4)].node)->str_value_, "count") != 0)
      {
        yyerror(&(yylsp[(1) - (4)]), result, "Only COUNT function can be with '*' parameter!");
        YYABORT;
      }
      else
      {
        ParseNode* node = NULL;
        malloc_terminal_node(node, result->malloc_pool_, T_STAR);
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_COUNT, 1, node);
      }
    ;}
    break;

  case 129:

    {
      if (strcasecmp((yyvsp[(1) - (5)].node)->str_value_, "count") == 0)
      {
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_COUNT, 2, (yyvsp[(3) - (5)].node), (yyvsp[(4) - (5)].node));
      }
      else if (strcasecmp((yyvsp[(1) - (5)].node)->str_value_, "sum") == 0)
      {
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_SUM, 2, (yyvsp[(3) - (5)].node), (yyvsp[(4) - (5)].node));
      }
      else if (strcasecmp((yyvsp[(1) - (5)].node)->str_value_, "max") == 0)
      {
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_MAX, 2, (yyvsp[(3) - (5)].node), (yyvsp[(4) - (5)].node));
      }
      else if (strcasecmp((yyvsp[(1) - (5)].node)->str_value_, "min") == 0)
      {
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_MIN, 2, (yyvsp[(3) - (5)].node), (yyvsp[(4) - (5)].node));
      }
      else if (strcasecmp((yyvsp[(1) - (5)].node)->str_value_, "avg") == 0)
      {
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_AVG, 2, (yyvsp[(3) - (5)].node), (yyvsp[(4) - (5)].node));
      }
      else
      {
        yyerror(&(yylsp[(1) - (5)]), result, "Wrong system function with 'DISTINCT/ALL'!");
        YYABORT;
      }
    ;}
    break;

  case 130:

    {
      if (strcasecmp((yyvsp[(1) - (4)].node)->str_value_, "count") == 0)
      {
        if ((yyvsp[(3) - (4)].node)->type_ == T_LINK_NODE)
        {
          yyerror(&(yylsp[(1) - (4)]), result, "COUNT function only support 1 parameter!");
          YYABORT;
        }
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_COUNT, 2, NULL, (yyvsp[(3) - (4)].node));
      }
      else if (strcasecmp((yyvsp[(1) - (4)].node)->str_value_, "sum") == 0)
      {
        if ((yyvsp[(3) - (4)].node)->type_ == T_LINK_NODE)
        {
          yyerror(&(yylsp[(1) - (4)]), result, "SUM function only support 1 parameter!");
          YYABORT;
        }
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_SUM, 2, NULL, (yyvsp[(3) - (4)].node));
      }
      else if (strcasecmp((yyvsp[(1) - (4)].node)->str_value_, "max") == 0)
      {
        if ((yyvsp[(3) - (4)].node)->type_ == T_LINK_NODE)
        {
          yyerror(&(yylsp[(1) - (4)]), result, "MAX function only support 1 parameter!");
          YYABORT;
        }
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_MAX, 2, NULL, (yyvsp[(3) - (4)].node));
      }
      else if (strcasecmp((yyvsp[(1) - (4)].node)->str_value_, "min") == 0)
      {
        if ((yyvsp[(3) - (4)].node)->type_ == T_LINK_NODE)
        {
          yyerror(&(yylsp[(1) - (4)]), result, "MIN function only support 1 parameter!");
          YYABORT;
        }
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_MIN, 2, NULL, (yyvsp[(3) - (4)].node));
      }
      else if (strcasecmp((yyvsp[(1) - (4)].node)->str_value_, "avg") == 0)
      {
        if ((yyvsp[(3) - (4)].node)->type_ == T_LINK_NODE)
        {
          yyerror(&(yylsp[(1) - (4)]), result, "AVG function only support 1 parameter!");
          YYABORT;
        }
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_AVG, 2, NULL, (yyvsp[(3) - (4)].node));
      }
      else if (strcasecmp((yyvsp[(1) - (4)].node)->str_value_, "trim") == 0)
      {
        if ((yyvsp[(3) - (4)].node)->type_ == T_LINK_NODE)
        {
          yyerror(&(yylsp[(1) - (4)]), result, "TRIM function syntax error! TRIM don't take %d params", (yyvsp[(3) - (4)].node)->num_child_);
          YYABORT;
        }
        else
        {
          ParseNode* default_type = NULL;
          malloc_terminal_node(default_type, result->malloc_pool_, T_INT);
          default_type->value_ = 0;
          ParseNode* default_operand = NULL;
          malloc_terminal_node(default_operand, result->malloc_pool_, T_STRING);
          default_operand->str_value_ = " "; /* blank for default */
          default_operand->value_ = strlen(default_operand->str_value_);
          ParseNode *params = NULL;
          malloc_non_terminal_node(params, result->malloc_pool_, T_EXPR_LIST, 3, default_type, default_operand, (yyvsp[(3) - (4)].node));
          malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_SYS, 2, (yyvsp[(1) - (4)].node), params);
        }
      }
      else  /* system function */
      {
        ParseNode *params = NULL;
        merge_nodes(params, result->malloc_pool_, T_EXPR_LIST, (yyvsp[(3) - (4)].node));
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_SYS, 2, (yyvsp[(1) - (4)].node), params);
      }
    ;}
    break;

  case 131:

    {
      if (strcasecmp((yyvsp[(1) - (6)].node)->str_value_, "cast") == 0)
      {
        /*modify fanqiushi DECIMAL OceanBase_BankCommV0.2 2014_6_16:b*/
        /*$5->value_ = $5->type_;
        $5->type_ = T_INT;*/
        if((yyvsp[(5) - (6)].node)->type_!=T_TYPE_DECIMAL)
        {
            (yyvsp[(5) - (6)].node)->value_ = (yyvsp[(5) - (6)].node)->type_;
            (yyvsp[(5) - (6)].node)->type_ = T_INT;
        }
        /*modify:e*/
        ParseNode *params = NULL;
        malloc_non_terminal_node(params, result->malloc_pool_, T_EXPR_LIST, 2, (yyvsp[(3) - (6)].node), (yyvsp[(5) - (6)].node));
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_SYS, 2, (yyvsp[(1) - (6)].node), params);
      }
      else
      {
        yyerror(&(yylsp[(1) - (6)]), result, "AS support cast function only!");
        YYABORT;
      }
    ;}
    break;

  case 132:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_SYS, 2, (yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node));
    ;}
    break;

  case 133:

    {
      if (strcasecmp((yyvsp[(1) - (3)].node)->str_value_, "now") == 0 ||
          strcasecmp((yyvsp[(1) - (3)].node)->str_value_, "current_time") == 0 ||
          strcasecmp((yyvsp[(1) - (3)].node)->str_value_, "current_timestamp") == 0)
      {
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CUR_TIME, 1, (yyvsp[(1) - (3)].node));
      }
      else if (strcasecmp((yyvsp[(1) - (3)].node)->str_value_, "strict_current_timestamp") == 0)
      {
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CUR_TIME_UPS, 1, (yyvsp[(1) - (3)].node));
      }
      else
      {
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_FUN_SYS, 1, (yyvsp[(1) - (3)].node));
      }
      //yyerror(&@1, result, "system/user-define function is not supported yet!");
      //YYABORT;
    ;}
    break;

  case 134:

    {
      (yyval.node) = (yyvsp[(1) - (4)].node);
      (yyval.node)->children_[0] = (yyvsp[(3) - (4)].node);
    ;}
    break;

  case 135:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ROW_COUNT, 1, NULL);
    ;}
    break;

  case 140:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_ALL);
    ;}
    break;

  case 141:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_DISTINCT);
    ;}
    break;

  case 142:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_DELETE, 3, (yyvsp[(3) - (5)].node), (yyvsp[(4) - (5)].node), (yyvsp[(5) - (5)].node));
    ;}
    break;

  case 143:

    {
      ParseNode* assign_list = NULL;
      merge_nodes(assign_list, result->malloc_pool_, T_ASSIGN_LIST, (yyvsp[(5) - (7)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_UPDATE, 5, (yyvsp[(3) - (7)].node), assign_list, (yyvsp[(6) - (7)].node), (yyvsp[(7) - (7)].node), (yyvsp[(2) - (7)].node));
    ;}
    break;

  case 144:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 145:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 146:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ASSIGN_ITEM, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 147:

    {
		ParseNode *index_options = NULL;
		merge_nodes(index_options, result->malloc_pool_, T_INDEX_OPTION_LIST, (yyvsp[(9) - (9)].node));
		malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CREATE_INDEX, 6,
								  (yyvsp[(3) - (9)].node), /* if not exists */
								  (yyvsp[(6) - (9)].node), /* table name */
								  (yyvsp[(4) - (9)].node), /* index name */ 
		                          (yyvsp[(7) - (9)].node), /* index columns */
		                          (yyvsp[(8) - (9)].node), /* storing list */
		                          index_options /* option list */
		                        );
	;}
    break;

  case 148:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_COLUMN_LIST, (yyvsp[(2) - (3)].node));
    ;}
    break;

  case 149:

    {
  		(yyval.node)=(yyvsp[(2) - (2)].node);
  	;}
    break;

  case 150:

    {
  		(yyval.node)=NULL;
  	;}
    break;

  case 151:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_COLUMN_LIST, (yyvsp[(2) - (3)].node));
    ;}
    break;

  case 152:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 153:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 154:

    {
      (yyval.node) = NULL;
    ;}
    break;

  case 155:

    {
		(yyval.node) = (yyvsp[(1) - (1)].node);
	;}
    break;

  case 156:

    {					 
       malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_DECLARE, 2,
                                        (yyvsp[(2) - (5)].node),		/*cursor name*/
                                        (yyvsp[(5) - (5)].node)		/*result from select_stmt*/
					             ); 
	 ;}
    break;

  case 157:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 158:

    {
            malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_OPEN, 1,
            (yyvsp[(2) - (2)].node)     /* cursor_name*/
                                    );
            ;}
    break;

  case 159:

    {
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH, 1,
                   (yyvsp[(2) - (2)].node)     /* cursor_name*/
                                       );
            ;}
    break;

  case 160:

    {
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH, 1,
                   (yyvsp[(2) - (3)].node)     /* cursor_name*/
                                       );
            ;}
    break;

  case 161:

    {
           	  ParseNode* args = NULL;
           	  ParseNode* fetch = NULL;
           	  malloc_non_terminal_node(fetch, result->malloc_pool_, T_CURSOR_FETCH, 1, (yyvsp[(2) - (4)].node));
			  merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(4) - (4)].node));
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_INTO, 2, fetch, args);
            ;}
    break;

  case 162:

    {
           	  ParseNode* args = NULL;
           	  ParseNode* fetch = NULL;
           	  malloc_non_terminal_node(fetch, result->malloc_pool_, T_CURSOR_FETCH_NEXT, 1, (yyvsp[(2) - (5)].node));
			  merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(5) - (5)].node));
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_NEXT_INTO, 2, fetch, args);
            ;}
    break;

  case 163:

    {
           	  ParseNode* args = NULL;
           	  ParseNode* fetch = NULL;
           	  malloc_non_terminal_node(fetch, result->malloc_pool_, T_CURSOR_FETCH_FIRST, 1, (yyvsp[(2) - (5)].node));
			  merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(5) - (5)].node));
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_FIRST_INTO, 2, fetch, args);
            ;}
    break;

  case 164:

    {
           	  ParseNode* args = NULL;
           	  ParseNode* fetch = NULL;
           	  malloc_non_terminal_node(fetch, result->malloc_pool_, T_CURSOR_FETCH_PRIOR, 1, (yyvsp[(2) - (5)].node));
			  merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(5) - (5)].node));
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_PRIOR_INTO, 2, fetch, args);
            ;}
    break;

  case 165:

    {
           	  ParseNode* args = NULL;
           	  ParseNode* fetch = NULL;
           	  malloc_non_terminal_node(fetch, result->malloc_pool_, T_CURSOR_FETCH_LAST, 1, (yyvsp[(2) - (5)].node));
			  merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(5) - (5)].node));
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_LAST_INTO, 2, fetch, args);
            ;}
    break;

  case 166:

    {
           	  ParseNode* args = NULL;
           	  ParseNode* fetch = NULL;
           	  malloc_non_terminal_node(fetch, result->malloc_pool_, T_CURSOR_FETCH_ABSOLUTE, 2, (yyvsp[(2) - (6)].node), (yyvsp[(4) - (6)].node));
			  merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(6) - (6)].node));
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_ABS_INTO, 2, fetch, args);
            ;}
    break;

  case 167:

    {
           	  ParseNode* args = NULL;
           	  ParseNode* fetch = NULL;
           	  malloc_non_terminal_node(fetch, result->malloc_pool_, T_CURSOR_FETCH_RELATIVE, 3, (yyvsp[(2) - (7)].node), (yyvsp[(3) - (7)].node), (yyvsp[(5) - (7)].node));
			  merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(7) - (7)].node));
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_RELATIVE_INTO, 2, fetch, args);
            ;}
    break;

  case 168:

    {
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_PRIOR, 1,
                   (yyvsp[(2) - (3)].node)     /* cursor_name*/
                    
                    
                    
                                       );
            ;}
    break;

  case 169:

    {
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_FIRST, 1,
                   (yyvsp[(2) - (3)].node)     /* cursor_name*/
                                       );
            ;}
    break;

  case 170:

    {
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_LAST, 1,
                   (yyvsp[(2) - (3)].node)     /* cursor_name*/
                                       );
            ;}
    break;

  case 171:

    {
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_RELATIVE, 3,
                   (yyvsp[(2) - (5)].node),     /* cursor_name*/
                   (yyvsp[(3) - (5)].node),                  
                   (yyvsp[(5) - (5)].node)                  
                                       );
            ;}
    break;

  case 172:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 1;
    ;}
    break;

  case 173:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 0;
    ;}
    break;

  case 174:

    {
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_ABSOLUTE, 2,
                   (yyvsp[(2) - (4)].node),     /* cursor_name*/
                   (yyvsp[(4) - (4)].node)
                                       );
            ;}
    break;

  case 175:

    {
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_FETCH_FROMTO, 3,
                   (yyvsp[(2) - (6)].node),     /* cursor_name*/
                   (yyvsp[(4) - (6)].node),
                   (yyvsp[(6) - (6)].node)
                                       );
            ;}
    break;

  case 176:

    {
             malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CURSOR_CLOSE, 1,
                   (yyvsp[(2) - (2)].node)     /* cursor_name*/
                                      );
             ;}
    break;

  case 177:

    {
      ParseNode *table_elements = NULL;
      ParseNode *table_options = NULL;
      merge_nodes(table_elements, result->malloc_pool_, T_TABLE_ELEMENT_LIST, (yyvsp[(6) - (8)].node));
      merge_nodes(table_options, result->malloc_pool_, T_TABLE_OPTION_LIST, (yyvsp[(8) - (8)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CREATE_TABLE, 4,
              (yyvsp[(3) - (8)].node),                   /* if not exists */
              (yyvsp[(4) - (8)].node),                   /* table name */
              table_elements,       /* columns or primary key */
              table_options         /* table option(s) */
              );
    ;}
    break;

  case 178:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_IF_NOT_EXISTS); ;}
    break;

  case 179:

    { (yyval.node) = NULL; ;}
    break;

  case 180:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 181:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 182:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 183:

    {
      ParseNode* col_list= NULL;
      merge_nodes(col_list, result->malloc_pool_, T_COLUMN_LIST, (yyvsp[(4) - (5)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PRIMARY_KEY, 1, col_list);
    ;}
    break;

  case 184:

    {
      ParseNode *attributes = NULL;
      merge_nodes(attributes, result->malloc_pool_, T_COLUMN_ATTRIBUTES, (yyvsp[(3) - (3)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_COLUMN_DEFINITION, 3, (yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].node), attributes);
    ;}
    break;

  case 185:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_INTEGER ); ;}
    break;

  case 186:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_INTEGER); ;}
    break;

  case 187:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_INTEGER); ;}
    break;

  case 188:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_INTEGER); ;}
    break;

  case 189:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_INTEGER); ;}
    break;

  case 190:

    {
      if ((yyvsp[(2) - (2)].node) == NULL)
        malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_DECIMAL);
      else
        merge_nodes((yyval.node), result->malloc_pool_, T_TYPE_DECIMAL, (yyvsp[(2) - (2)].node));
      /* modify xsl ECNU_DECIMAL 2017_2
      yyerror(&@1, result, "DECIMAL type is not supported");
      YYABORT;
      */
      //modify e
    ;}
    break;

  case 191:

    {
      if ((yyvsp[(2) - (2)].node) == NULL)
        malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_DECIMAL);
      else
        merge_nodes((yyval.node), result->malloc_pool_, T_TYPE_DECIMAL, (yyvsp[(2) - (2)].node));
      yyerror(&(yylsp[(1) - (2)]), result, "NUMERIC type is not supported");
      YYABORT;
    ;}
    break;

  case 192:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_BOOLEAN ); ;}
    break;

  case 193:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_FLOAT, 1, (yyvsp[(2) - (2)].node)); ;}
    break;

  case 194:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_DOUBLE); ;}
    break;

  case 195:

    {
      (void)((yyvsp[(2) - (2)].node)) ; /* make bison mute */
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_DOUBLE);
    ;}
    break;

  case 196:

    {
      if ((yyvsp[(2) - (2)].node) == NULL)
        malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_TIMESTAMP);
      else
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_TIMESTAMP, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 197:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_TIMESTAMP); ;}
    break;

  case 198:

    {
      if ((yyvsp[(2) - (2)].node) == NULL)
        malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_CHARACTER);
      else
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_CHARACTER, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 199:

    {
      if ((yyvsp[(2) - (2)].node) == NULL)
        malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_CHARACTER);
      else
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_CHARACTER, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 200:

    {
      if ((yyvsp[(2) - (2)].node) == NULL)
        malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_VARCHAR);
      else
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_VARCHAR, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 201:

    {
      if ((yyvsp[(2) - (2)].node) == NULL)
        malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_VARCHAR);
      else
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_VARCHAR, 1, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 202:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_CREATETIME); ;}
    break;

  case 203:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_MODIFYTIME); ;}
    break;

  case 204:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_DATE);
      yyerror(&(yylsp[(1) - (1)]), result, "DATE type is not supported");
      YYABORT;
    ;}
    break;

  case 205:

    {
      if ((yyvsp[(2) - (2)].node) == NULL)
        malloc_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_TIME);
      else
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TYPE_TIME, 1, (yyvsp[(2) - (2)].node));
      yyerror(&(yylsp[(1) - (2)]), result, "TIME type is not supported");
      YYABORT;
    ;}
    break;

  case 206:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(2) - (5)].node), (yyvsp[(4) - (5)].node)); ;}
    break;

  case 207:

    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 208:

    { (yyval.node) = NULL; ;}
    break;

  case 209:

    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 210:

    { (yyval.node) = NULL; ;}
    break;

  case 211:

    { (yyval.node) = NULL; ;}
    break;

  case 212:

    { (yyval.node) = NULL; ;}
    break;

  case 213:

    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 214:

    { (yyval.node) = NULL; ;}
    break;

  case 215:

    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 216:

    { (yyval.node) = NULL; ;}
    break;

  case 217:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 218:

    { (yyval.node) = NULL; ;}
    break;

  case 219:

    {
      (void)((yyvsp[(2) - (2)].node)) ; /* make bison mute */
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_CONSTR_NOT_NULL);
    ;}
    break;

  case 220:

    {
      (void)((yyvsp[(1) - (1)].node)) ; /* make bison mute */
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_CONSTR_NULL);
    ;}
    break;

  case 221:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CONSTR_DEFAULT, 1, (yyvsp[(2) - (2)].node)); ;}
    break;

  case 222:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_CONSTR_AUTO_INCREMENT); ;}
    break;

  case 223:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_CONSTR_PRIMARY_KEY); ;}
    break;

  case 224:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 225:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 226:

    {
      (yyval.node) = NULL;
    ;}
    break;

  case 227:

    {
      (void)((yyvsp[(2) - (3)].node)) ; /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_JOIN_INFO, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 228:

    {
      (void)((yyvsp[(2) - (3)].node)) ; /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_EXPIRE_INFO, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 229:

    {
      (void)((yyvsp[(2) - (3)].node)) ; /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TABLET_MAX_SIZE, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 230:

    {
      (void)((yyvsp[(2) - (3)].node)) ; /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TABLET_BLOCK_SIZE, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 231:

    {
      (void)((yyvsp[(2) - (3)].node)) ; /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TABLET_ID, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 232:

    {
      (void)((yyvsp[(2) - (3)].node)) ; /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_REPLICA_NUM, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 233:

    {
      (void)((yyvsp[(2) - (3)].node)) ; /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_COMPRESS_METHOD, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 234:

    {
      (void)((yyvsp[(2) - (3)].node)) ; /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_USE_BLOOM_FILTER, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 235:

    {
      (void)((yyvsp[(2) - (3)].node)) ; /* make bison mute */
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_CONSISTENT_MODE);
      (yyval.node)->value_ = 1;
    ;}
    break;

  case 236:

    {
      (void)((yyvsp[(2) - (3)].node)); /*  make bison mute*/
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_COMMENT, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 237:

    { (yyval.node) = NULL; ;}
    break;

  case 238:

    { (yyval.node) = NULL; ;}
    break;

  case 239:

    {
      ParseNode *tables = NULL;
      merge_nodes(tables, result->malloc_pool_, T_TABLE_LIST, (yyvsp[(4) - (4)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_DROP_TABLE, 2, (yyvsp[(3) - (4)].node), tables);
    ;}
    break;

  case 240:

    { (yyval.node) = NULL; ;}
    break;

  case 241:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_IF_EXISTS); ;}
    break;

  case 242:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 243:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 244:

    {
          ParseNode *tables = NULL;
          merge_nodes(tables, result->malloc_pool_, T_TABLE_LIST, (yyvsp[(4) - (5)].node));
          malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TRUNCATE_TABLE, 3, (yyvsp[(3) - (5)].node), tables, (yyvsp[(5) - (5)].node));
        ;}
    break;

  case 245:

    {
      ParseNode *indexs = NULL;
      merge_nodes(indexs, result->malloc_pool_, T_INDEX_LIST, (yyvsp[(4) - (6)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_DROP_INDEX, 3, (yyvsp[(3) - (6)].node), indexs, (yyvsp[(6) - (6)].node));
    ;}
    break;

  case 246:

    { (yyval.node) = NULL; ;}
    break;

  case 247:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 248:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 249:

    {
	  (yyval.node) = (yyvsp[(1) - (1)].node);
	;}
    break;

  case 250:

    {
      ParseNode* val_list = NULL;
      merge_nodes(val_list, result->malloc_pool_, T_VALUE_LIST, (yyvsp[(6) - (7)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_INSERT, 6,
                              (yyvsp[(3) - (7)].node),           /* target relation */
                              (yyvsp[(4) - (7)].node),           /* column list */
                              val_list,     /* value list */
                              NULL,         /* value from sub-query */
                              (yyvsp[(1) - (7)].node),           /* is replacement */
                              (yyvsp[(7) - (7)].node)            /* when expression */
                              );
    ;}
    break;

  case 251:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_INSERT, 6,
                              (yyvsp[(3) - (4)].node),           /* target relation */
                              NULL,         /* column list */
                              NULL,         /* value list */
                              (yyvsp[(4) - (4)].node),           /* value from sub-query */
                              (yyvsp[(1) - (4)].node),           /* is replacement */
                              NULL          /* when expression */
                              );
    ;}
    break;

  case 252:

    {
      /* if opt_when is really needed, use select_with_parens instead */
      ParseNode* col_list = NULL;
      merge_nodes(col_list, result->malloc_pool_, T_COLUMN_LIST, (yyvsp[(5) - (7)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_INSERT, 6,
                              (yyvsp[(3) - (7)].node),           /* target relation */
                              col_list,     /* column list */
                              NULL,         /* value list */
                              (yyvsp[(7) - (7)].node),           /* value from sub-query */
                              (yyvsp[(1) - (7)].node),           /* is replacement */
                              NULL          /* when expression */
                              );
    ;}
    break;

  case 253:

    { (yyval.node) = NULL; ;}
    break;

  case 254:

    {
      (yyval.node) = (yyvsp[(2) - (2)].node);
    ;}
    break;

  case 255:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 1;
    ;}
    break;

  case 256:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 0;
    ;}
    break;

  case 257:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_COLUMN_LIST, (yyvsp[(2) - (3)].node));
    ;}
    break;

  case 258:

    { (yyval.node) = NULL; ;}
    break;

  case 259:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 260:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 261:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_VALUE_VECTOR, (yyvsp[(2) - (3)].node));
    ;}
    break;

  case 262:

    {
    merge_nodes((yyvsp[(4) - (5)].node), result->malloc_pool_, T_VALUE_VECTOR, (yyvsp[(4) - (5)].node));
    malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (5)].node), (yyvsp[(4) - (5)].node));
  ;}
    break;

  case 263:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 264:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 265:

    {
      (yyval.node) = (yyvsp[(1) - (2)].node);
      (yyval.node)->children_[14] = (yyvsp[(2) - (2)].node);
      if ((yyval.node)->children_[12] == NULL && (yyvsp[(2) - (2)].node) != NULL)
      {
        malloc_terminal_node((yyval.node)->children_[12], result->malloc_pool_, T_BOOL);
        (yyval.node)->children_[12]->value_ = 1;
      }
    ;}
    break;

  case 266:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 267:

    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 268:

    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 269:

    {
      (yyval.node)= (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 270:

    {
      (yyval.node) = (yyvsp[(1) - (2)].node);
      (yyval.node)->children_[12] = (yyvsp[(2) - (2)].node);
    ;}
    break;

  case 271:

    {
      /* use the new order by to replace old one */
      ParseNode* select = (ParseNode*)(yyvsp[(1) - (3)].node);
      if (select->children_[10])
        destroy_tree(select->children_[10]);
      select->children_[10] = (yyvsp[(2) - (3)].node);
      select->children_[12] = (yyvsp[(3) - (3)].node);
      (yyval.node) = select;
    ;}
    break;

  case 272:

    {
      /* use the new order by to replace old one */
      ParseNode* select = (ParseNode*)(yyvsp[(1) - (4)].node);
      if ((yyvsp[(2) - (4)].node))
      {
        if (select->children_[10])
          destroy_tree(select->children_[10]);
        select->children_[10] = (yyvsp[(2) - (4)].node);
      }

      /* set limit value */
      if (select->children_[11])
        destroy_tree(select->children_[11]);
      select->children_[11] = (yyvsp[(3) - (4)].node);
      select->children_[12] = (yyvsp[(4) - (4)].node);
      (yyval.node) = select;
    ;}
    break;

  case 273:

    {
      ParseNode* project_list = NULL;
      merge_nodes(project_list, result->malloc_pool_, T_PROJECT_LIST, (yyvsp[(4) - (5)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SELECT, 15,
                              (yyvsp[(3) - (5)].node),             /* 1. distinct */
                              project_list,   /* 2. select clause */
                              NULL,           /* 3. from clause */
                              NULL,           /* 4. where */
                              NULL,           /* 5. group by */
                              NULL,           /* 6. having */
                              NULL,           /* 7. set operation */
                              NULL,           /* 8. all specified? */
                              NULL,           /* 9. former select stmt */
                              NULL,           /* 10. later select stmt */
                              NULL,           /* 11. order by */
                              (yyvsp[(5) - (5)].node),             /* 12. limit */
                              NULL,           /* 13. for update */
                              (yyvsp[(2) - (5)].node),             /* 14 hints */
                              NULL            /* 15 when clause */
                              );
    ;}
    break;

  case 274:

    {
      ParseNode* project_list = NULL;
      merge_nodes(project_list, result->malloc_pool_, T_PROJECT_LIST, (yyvsp[(4) - (8)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SELECT, 15,
                              (yyvsp[(3) - (8)].node),             /* 1. distinct */
                              project_list,   /* 2. select clause */
                              NULL,           /* 3. from clause */
                              (yyvsp[(7) - (8)].node),             /* 4. where */
                              NULL,           /* 5. group by */
                              NULL,           /* 6. having */
                              NULL,           /* 7. set operation */
                              NULL,           /* 8. all specified? */
                              NULL,           /* 9. former select stmt */
                              NULL,           /* 10. later select stmt */
                              NULL,           /* 11. order by */
                              (yyvsp[(8) - (8)].node),             /* 12. limit */
                              NULL,           /* 13. for update */
                              (yyvsp[(2) - (8)].node),             /* 14 hints */
                              NULL            /* 15 when clause */
                              );
    ;}
    break;

  case 275:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 276:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 277:

    {
						  ParseNode* project_list = NULL;
						  ParseNode* from_list = NULL;
						  ParseNode* select = NULL;
						  ParseNode* args = NULL;
              merge_nodes(project_list, result->malloc_pool_, T_PROJECT_LIST, (yyvsp[(3) - (9)].node));
              merge_nodes(from_list, result->malloc_pool_, T_FROM_LIST, (yyvsp[(7) - (9)].node));
						  malloc_non_terminal_node(select, result->malloc_pool_, T_SELECT, 16,
                              NULL,           /* 1. distinct */
                              project_list,   /* 2. select clause */
                              from_list,      /* 3. from clause */
                              (yyvsp[(8) - (9)].node),             /* 4. where */
                              NULL,           /* 5. group by */
                              NULL,           /* 6. having */
                              NULL,           /* 7. set operation */
                              NULL,           /* 8. all specified? */
                              NULL,           /* 9. former select stmt */
                              NULL,           /* 10. later select stmt */
                              NULL,           /* 11. order by */
                              NULL,             /* 12. limit */
                              (yyvsp[(9) - (9)].node),           /* 13. for update */
                              (yyvsp[(2) - (9)].node),             /* 14 hints */
                              NULL            /* 15 when clause */
                              ,NULL
                              );
              merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(5) - (9)].node));
						  malloc_non_terminal_node((yyval.node),result->malloc_pool_, T_SELECT_INTO, 2, args, select);
						;}
    break;

  case 278:

    {
      ParseNode* project_list = NULL;
      ParseNode* from_list = NULL;
      merge_nodes(project_list, result->malloc_pool_, T_PROJECT_LIST, (yyvsp[(4) - (9)].node));
      merge_nodes(from_list, result->malloc_pool_, T_FROM_LIST, (yyvsp[(6) - (9)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SELECT, 15,
                              (yyvsp[(3) - (9)].node),             /* 1. distinct */
                              project_list,   /* 2. select clause */
                              from_list,      /* 3. from clause */
                              (yyvsp[(7) - (9)].node),             /* 4. where */
                              (yyvsp[(8) - (9)].node),             /* 5. group by */
                              (yyvsp[(9) - (9)].node),             /* 6. having */
                              NULL,           /* 7. set operation */
                              NULL,           /* 8. all specified? */
                              NULL,           /* 9. former select stmt */
                              NULL,           /* 10. later select stmt */
                              NULL,           /* 11. order by */
                              NULL,           /* 12. limit */
                              NULL,           /* 13. for update */
                              (yyvsp[(2) - (9)].node),             /* 14 hints */
                              NULL            /* 15 when clause */
                              );
    ;}
    break;

  case 279:

    {
      ParseNode* set_op = NULL;
      malloc_terminal_node(set_op, result->malloc_pool_, T_SET_UNION);
	    malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SELECT, 15,
                              NULL,           /* 1. distinct */
                              NULL,           /* 2. select clause */
                              NULL,           /* 3. from clause */
                              NULL,           /* 4. where */
                              NULL,           /* 5. group by */
                              NULL,           /* 6. having */
                              set_op,   /* 7. set operation */
                              (yyvsp[(3) - (4)].node),             /* 8. all specified? */
                              (yyvsp[(1) - (4)].node),             /* 9. former select stmt */
                              (yyvsp[(4) - (4)].node),             /* 10. later select stmt */
                              NULL,           /* 11. order by */
                              NULL,           /* 12. limit */
                              NULL,           /* 13. for update */
                              NULL,           /* 14 hints */
                              NULL            /* 15 when clause */
                              );
    ;}
    break;

  case 280:

    {
      ParseNode* set_op = NULL;
      malloc_terminal_node(set_op, result->malloc_pool_, T_SET_INTERSECT);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SELECT, 15,
                              NULL,           /* 1. distinct */
                              NULL,           /* 2. select clause */
                              NULL,           /* 3. from clause */
                              NULL,           /* 4. where */
                              NULL,           /* 5. group by */
                              NULL,           /* 6. having */
                              set_op,   /* 7. set operation */
                              (yyvsp[(3) - (4)].node),             /* 8. all specified? */
                              (yyvsp[(1) - (4)].node),             /* 9. former select stmt */
                              (yyvsp[(4) - (4)].node),             /* 10. later select stmt */
                              NULL,           /* 11. order by */
                              NULL,           /* 12. limit */
                              NULL,           /* 13. for update */
                              NULL,           /* 14 hints */
                              NULL            /* 15 when clause */
                              );
    ;}
    break;

  case 281:

    {
      ParseNode* set_op = NULL;
      malloc_terminal_node(set_op, result->malloc_pool_, T_SET_EXCEPT);
	    malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SELECT, 15,
                              NULL,           /* 1. distinct */
                              NULL,           /* 2. select clause */
                              NULL,           /* 3. from clause */
                              NULL,           /* 4. where */
                              NULL,           /* 5. group by */
                              NULL,           /* 6. having */
                              set_op,   /* 7. set operation */
                              (yyvsp[(3) - (4)].node),             /* 8. all specified? */
                              (yyvsp[(1) - (4)].node),             /* 9. former select stmt */
                              (yyvsp[(4) - (4)].node),             /* 10. later select stmt */
                              NULL,           /* 11. order by */
                              NULL,           /* 12. limit */
                              NULL,           /* 13. for update */
                              NULL,           /* 14 hints */
                              NULL            /* 15 when clause */
                              );
    ;}
    break;

  case 282:

    {(yyval.node) = NULL;;}
    break;

  case 283:

    {
      (yyval.node) = (yyvsp[(2) - (2)].node);
    ;}
    break;

  case 284:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_WHERE_CLAUSE, 2, (yyvsp[(3) - (3)].node), (yyvsp[(2) - (3)].node));
    ;}
    break;

  case 285:

    {
      if ((yyvsp[(2) - (4)].node)->type_ == T_QUESTIONMARK && (yyvsp[(4) - (4)].node)->type_ == T_QUESTIONMARK)
      {
        (yyvsp[(4) - (4)].node)->value_++;
      }
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LIMIT_CLAUSE, 2, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 286:

    {
      if ((yyvsp[(2) - (4)].node)->type_ == T_QUESTIONMARK && (yyvsp[(4) - (4)].node)->type_ == T_QUESTIONMARK)
      {
        (yyvsp[(4) - (4)].node)->value_++;
      }
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LIMIT_CLAUSE, 2, (yyvsp[(4) - (4)].node), (yyvsp[(2) - (4)].node));
    ;}
    break;

  case 287:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LIMIT_CLAUSE, 2, (yyvsp[(2) - (2)].node), NULL);
    ;}
    break;

  case 288:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LIMIT_CLAUSE, 2, NULL, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 289:

    {
      if ((yyvsp[(2) - (4)].node)->type_ == T_QUESTIONMARK && (yyvsp[(4) - (4)].node)->type_ == T_QUESTIONMARK)
      {
        (yyvsp[(4) - (4)].node)->value_++;
      }
    	malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LIMIT_CLAUSE, 2, (yyvsp[(4) - (4)].node), (yyvsp[(2) - (4)].node));
    ;}
    break;

  case 290:

    {
      (yyval.node) = NULL;
    ;}
    break;

  case 291:

    {
      if ((yyvsp[(2) - (3)].node))
      {
        merge_nodes((yyval.node), result->malloc_pool_, T_HINT_OPTION_LIST, (yyvsp[(2) - (3)].node));
      }
      else
      {
        (yyval.node) = NULL;
      }
    ;}
    break;

  case 292:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 293:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 294:

    {
      (yyval.node) = NULL;
    ;}
    break;

  case 295:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 296:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 297:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_READ_STATIC);
    ;}
    break;

  case 298:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_HOTSPOT);
    ;}
    break;

  case 299:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SEMI_JOIN, 6, (yyvsp[(3) - (14)].node), (yyvsp[(5) - (14)].node), (yyvsp[(7) - (14)].node), (yyvsp[(9) - (14)].node), (yyvsp[(11) - (14)].node), (yyvsp[(13) - (14)].node));
    ;}
    break;

  case 300:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_READ_CONSISTENCY);
      (yyval.node)->value_ = (yyvsp[(3) - (4)].ival);
    ;}
    break;

  case 301:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_USE_INDEX, 2, (yyvsp[(3) - (5)].node), (yyvsp[(4) - (5)].node));
    ;}
    break;

  case 302:

    {
	  (void)((yyvsp[(1) - (4)].node));
	  (void)((yyvsp[(3) - (4)].node));
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_UNKOWN_HINT);
    ;}
    break;

  case 303:

    {
      (yyval.node) = (yyvsp[(2) - (3)].node);
    ;}
    break;

  case 304:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_JOIN_OP_TYPE_LIST, (yyvsp[(3) - (4)].node));
    ;}
    break;

  case 305:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_NO_GROUP);
    ;}
    break;

  case 306:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_LONG_TRANS);
    ;}
    break;

  case 307:

    {
      (yyval.node) = (yyvsp[(1) - (2)].node);
    ;}
    break;

  case 308:

    {
      (yyval.node) = NULL;
    ;}
    break;

  case 309:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 310:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 311:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BLOOMFILTER_JOIN);
    ;}
    break;

  case 312:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_MERGE_JOIN);
    ;}
    break;

  case 313:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_HASH_JOIN_SINGLE);
    ;}
    break;

  case 314:

    {
    (yyval.ival) = 3;
  ;}
    break;

  case 315:

    {
    (yyval.ival) = 4;
  ;}
    break;

  case 316:

    {
    (yyval.ival) = 1;
  ;}
    break;

  case 317:

    {
    (yyval.ival) = 2;
  ;}
    break;

  case 318:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 319:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 320:

    { (yyval.node) = NULL; ;}
    break;

  case 321:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 322:

    { (yyval.node) = NULL; ;}
    break;

  case 323:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 1;
    ;}
    break;

  case 324:

    {
      ParseNode *default_type = NULL;
      malloc_terminal_node(default_type, result->malloc_pool_, T_INT);
      default_type->value_ = 0;
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_EXPR_LIST, 3, default_type, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 325:

    {
      ParseNode *default_type = NULL;
      malloc_terminal_node(default_type, result->malloc_pool_, T_INT);
      default_type->value_ = 0;
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_EXPR_LIST, 3, default_type, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 326:

    {
      ParseNode *default_type = NULL;
      malloc_terminal_node(default_type, result->malloc_pool_, T_INT);
      default_type->value_ = 1;
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_EXPR_LIST, 3, default_type, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 327:

    {
      ParseNode *default_type = NULL;
      malloc_terminal_node(default_type, result->malloc_pool_, T_INT);
      default_type->value_ = 2;
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_EXPR_LIST, 3, default_type, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 328:

    {
      ParseNode *default_type = NULL;
      malloc_terminal_node(default_type, result->malloc_pool_, T_INT);
      default_type->value_ = 0;
      ParseNode *default_operand = NULL;
      malloc_terminal_node(default_operand, result->malloc_pool_, T_STRING);
      default_operand->str_value_ = " "; /* blank for default */
      default_operand->value_ = strlen(default_operand->str_value_);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_EXPR_LIST, 3, default_type, default_operand, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 329:

    {
      ParseNode *default_type = NULL;
      malloc_terminal_node(default_type, result->malloc_pool_, T_INT);
      default_type->value_ = 1;
      ParseNode *default_operand = NULL;
      malloc_terminal_node(default_operand, result->malloc_pool_, T_STRING);
      default_operand->str_value_ = " "; /* blank for default */
      default_operand->value_ = strlen(default_operand->str_value_);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_EXPR_LIST, 3, default_type, default_operand, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 330:

    {
      ParseNode *default_type = NULL;
      malloc_terminal_node(default_type, result->malloc_pool_, T_INT);
      default_type->value_ = 2;
      ParseNode *default_operand = NULL;
      malloc_terminal_node(default_operand, result->malloc_pool_, T_STRING);
      default_operand->str_value_ = " "; /* blank for default */
      default_operand->value_ = strlen(default_operand->str_value_);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_EXPR_LIST, 3, default_type, default_operand, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 331:

    { (yyval.node) = NULL; ;}
    break;

  case 332:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_EXPR_LIST, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 333:

    { (yyval.node) = (yyvsp[(1) - (1)].node);;}
    break;

  case 334:

    { (yyval.node) = NULL; ;}
    break;

  case 335:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_SORT_LIST, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 336:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 337:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 338:

    {
    	malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SORT_KEY, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 339:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_SORT_ASC); ;}
    break;

  case 340:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_SORT_ASC); ;}
    break;

  case 341:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_SORT_DESC); ;}
    break;

  case 342:

    { (yyval.node) = 0; ;}
    break;

  case 343:

    {
      (yyval.node) = (yyvsp[(2) - (2)].node);
    ;}
    break;

  case 344:

    {
      (yyval.node) = NULL;
    ;}
    break;

  case 345:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_ALL);
    ;}
    break;

  case 346:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_DISTINCT);
    ;}
    break;

  case 347:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROJECT_STRING, 1, (yyvsp[(1) - (1)].node));
      dup_expr_string((yyval.node)->str_value_, result, (yylsp[(1) - (1)]).first_column, (yylsp[(1) - (1)]).last_column);
    ;}
    break;

  case 348:

    {
      ParseNode* alias_node = NULL;
      malloc_non_terminal_node(alias_node, result->malloc_pool_, T_ALIAS, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROJECT_STRING, 1, alias_node);
      dup_expr_string((yyval.node)->str_value_, result, (yylsp[(1) - (2)]).first_column, (yylsp[(1) - (2)]).last_column);
      dup_expr_string(alias_node->str_value_, result, (yylsp[(2) - (2)]).first_column, (yylsp[(2) - (2)]).last_column);
    ;}
    break;

  case 349:

    {
      ParseNode* alias_node = NULL;
      malloc_non_terminal_node(alias_node, result->malloc_pool_, T_ALIAS, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROJECT_STRING, 1, alias_node);
      dup_expr_string((yyval.node)->str_value_, result, (yylsp[(1) - (3)]).first_column, (yylsp[(1) - (3)]).last_column);
      dup_expr_string(alias_node->str_value_, result, (yylsp[(3) - (3)]).first_column, (yylsp[(3) - (3)]).last_column);
    ;}
    break;

  case 350:

    {
      ParseNode* star_node = NULL;
      malloc_terminal_node(star_node, result->malloc_pool_, T_STAR);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROJECT_STRING, 1, star_node);
      dup_expr_string((yyval.node)->str_value_, result, (yylsp[(1) - (1)]).first_column, (yylsp[(1) - (1)]).last_column);
    ;}
    break;

  case 351:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 352:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 353:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 354:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 355:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 356:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ALIAS, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 357:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ALIAS, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 358:

    {
    	malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ALIAS, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 359:

    {
    	malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ALIAS, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 360:

    {
    	(yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 361:

    {
    	malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ALIAS, 2, (yyvsp[(2) - (5)].node), (yyvsp[(5) - (5)].node));
    	yyerror(&(yylsp[(1) - (5)]), result, "qualied joined table can not be aliased!");
      YYABORT;
    ;}
    break;

  case 362:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 363:

    {
    	(yyval.node) = (yyvsp[(2) - (3)].node);
    ;}
    break;

  case 364:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_JOINED_TABLE, 4, (yyvsp[(2) - (6)].node), (yyvsp[(1) - (6)].node), (yyvsp[(4) - (6)].node), (yyvsp[(6) - (6)].node));
    ;}
    break;

  case 365:

    {
      ParseNode* node = NULL;
      malloc_terminal_node(node, result->malloc_pool_, T_JOIN_INNER);
    	malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_JOINED_TABLE, 4, node, (yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node));
    ;}
    break;

  case 366:

    {
      /* make bison mute */
      (void)((yyvsp[(2) - (2)].node));
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_JOIN_FULL);
    ;}
    break;

  case 367:

    {
      /* make bison mute */
      (void)((yyvsp[(2) - (2)].node));
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_JOIN_LEFT);
    ;}
    break;

  case 368:

    {
      /* make bison mute */
      (void)((yyvsp[(2) - (2)].node));
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_JOIN_RIGHT);
    ;}
    break;

  case 369:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_JOIN_INNER);
    ;}
    break;

  case 370:

    { (yyval.node) = NULL; ;}
    break;

  case 371:

    { (yyval.node) = NULL; ;}
    break;

  case 372:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_EXPLAIN, 1, (yyvsp[(3) - (3)].node));
      (yyval.node)->value_ = ((yyvsp[(2) - (3)].node) ? 1 : 0); /* positive: verbose */
    ;}
    break;

  case 373:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 374:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 375:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 376:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 377:

    { (yyval.node) = (ParseNode*)1; ;}
    break;

  case 378:

    { (yyval.node) = NULL; ;}
    break;

  case 379:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_TABLES, 1, (yyvsp[(3) - (3)].node)); ;}
    break;

  case 380:

    {  malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_INDEX, 2, (yyvsp[(4) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 381:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_COLUMNS, 2, (yyvsp[(4) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 382:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_COLUMNS, 2, (yyvsp[(4) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 383:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_TABLE_STATUS, 1, (yyvsp[(4) - (4)].node)); ;}
    break;

  case 384:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_SERVER_STATUS, 1, (yyvsp[(4) - (4)].node)); ;}
    break;

  case 385:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_VARIABLES, 1, (yyvsp[(4) - (4)].node));
      (yyval.node)->value_ = (yyvsp[(2) - (4)].ival);
    ;}
    break;

  case 386:

    { malloc_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_SCHEMA); ;}
    break;

  case 387:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_CREATE_TABLE, 1, (yyvsp[(4) - (4)].node)); ;}
    break;

  case 388:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_COLUMNS, 2, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 389:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_COLUMNS, 2, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 390:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_WARNINGS, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 391:

    {
      if ((yyvsp[(2) - (3)].node)->type_ != T_FUN_COUNT)
      {
        yyerror(&(yylsp[(1) - (3)]), result, "Only COUNT(*) function is supported in SHOW WARNINGS statement!");
        YYABORT;
      }
      else
      {
        malloc_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_WARNINGS);
        (yyval.node)->value_ = 1;
      }
    ;}
    break;

  case 392:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_GRANTS, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 393:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_PARAMETERS, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 394:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_PROCESSLIST);
      (yyval.node)->value_ = (yyvsp[(2) - (3)].ival);
    ;}
    break;

  case 395:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LOCK_TABLE, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 396:

    {
      if ((yyvsp[(2) - (4)].node)->value_ < 0 || (yyvsp[(4) - (4)].node)->value_ < 0)
      {
        yyerror(&(yylsp[(1) - (4)]), result, "OFFSET/COUNT must not be less than 0!");
        YYABORT;
      }
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_LIMIT, 2, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 397:

    {
      if ((yyvsp[(2) - (2)].node)->value_ < 0)
      {
        yyerror(&(yylsp[(1) - (2)]), result, "COUNT must not be less than 0!");
        YYABORT;
      }
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SHOW_LIMIT, 2, NULL, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 398:

    { (yyval.node) = NULL; ;}
    break;

  case 399:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 400:

    { (yyval.node) = NULL; ;}
    break;

  case 401:

    { (yyval.node) = NULL; ;}
    break;

  case 402:

    { (yyval.ival) = 1; ;}
    break;

  case 403:

    { (yyval.ival) = 0; ;}
    break;

  case 404:

    { (yyval.ival) = 0; ;}
    break;

  case 405:

    { (yyval.node) = NULL; ;}
    break;

  case 406:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_LIKE, 1, (yyvsp[(2) - (2)].node)); ;}
    break;

  case 407:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_WHERE_CLAUSE, 1, (yyvsp[(2) - (2)].node)); ;}
    break;

  case 408:

    { (yyval.node) = NULL; ;}
    break;

  case 409:

    { malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OP_LIKE, 1, (yyvsp[(1) - (1)].node)); ;}
    break;

  case 410:

    { (yyval.ival) = 0; ;}
    break;

  case 411:

    { (yyval.ival) = 1; ;}
    break;

  case 412:

    {
        merge_nodes((yyval.node), result->malloc_pool_, T_CREATE_USER, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 413:

    {
        (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 414:

    {
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 415:

    {
        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CREATE_USER_SPEC, 2, (yyvsp[(1) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 416:

    {
        (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 417:

    {
        (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 418:

    {
        merge_nodes((yyval.node), result->malloc_pool_, T_DROP_USER, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 419:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 420:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 421:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SET_PASSWORD, 2, (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node));
    ;}
    break;

  case 422:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SET_PASSWORD, 2, (yyvsp[(3) - (6)].node), (yyvsp[(6) - (6)].node));
    ;}
    break;

  case 423:

    {
      (yyval.node) = (yyvsp[(2) - (2)].node);
    ;}
    break;

  case 424:

    {
      (yyval.node) = NULL;
    ;}
    break;

  case 425:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_RENAME_USER, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 426:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_RENAME_INFO, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 427:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 428:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 429:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LOCK_USER, 2, (yyvsp[(3) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 430:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 1;
    ;}
    break;

  case 431:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 0;
    ;}
    break;

  case 432:

    {
      (void)(yyval.node);
    ;}
    break;

  case 433:

    {
      (void)(yyval.node);
    ;}
    break;

  case 434:

    {
      (yyval.ival) = 1;
    ;}
    break;

  case 435:

    {
      (yyval.ival) = 0;
    ;}
    break;

  case 436:

    {
      (void)(yyvsp[(2) - (2)].node);
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BEGIN);
      (yyval.node)->value_ = 0;
    ;}
    break;

  case 437:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BEGIN);
      (yyval.node)->value_ = (yyvsp[(3) - (3)].ival);
    ;}
    break;

  case 438:

    {
      (void)(yyvsp[(2) - (2)].node);
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_COMMIT);
    ;}
    break;

  case 439:

    {
      (void)(yyvsp[(2) - (2)].node);
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_ROLLBACK);
    ;}
    break;

  case 440:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_KILL, 3, (yyvsp[(2) - (4)].node), (yyvsp[(3) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 441:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 0;
    ;}
    break;

  case 442:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 1;
    ;}
    break;

  case 443:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 0;
    ;}
    break;

  case 444:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 1;
    ;}
    break;

  case 445:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_BOOL);
      (yyval.node)->value_ = 0;
    ;}
    break;

  case 446:

    {
      ParseNode *privileges_node = NULL;
      ParseNode *users_node = NULL;
      merge_nodes(privileges_node, result->malloc_pool_, T_PRIVILEGES, (yyvsp[(2) - (6)].node));
      merge_nodes(users_node, result->malloc_pool_, T_USERS, (yyvsp[(6) - (6)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_GRANT,
                                 3, privileges_node, (yyvsp[(4) - (6)].node), users_node);
    ;}
    break;

  case 447:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 448:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 449:

    {
      (void)(yyvsp[(2) - (2)].node);                 /* useless */
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_ALL;
    ;}
    break;

  case 450:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_ALTER;
    ;}
    break;

  case 451:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_CREATE;
    ;}
    break;

  case 452:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_CREATE_USER;
    ;}
    break;

  case 453:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_DELETE;
    ;}
    break;

  case 454:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_DROP;
    ;}
    break;

  case 455:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_DROP;
    ;}
    break;

  case 456:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_GRANT_OPTION;
    ;}
    break;

  case 457:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_INSERT;
    ;}
    break;

  case 458:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_UPDATE;
    ;}
    break;

  case 459:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_SELECT;
    ;}
    break;

  case 460:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_TYPE);
      (yyval.node)->value_ = OB_PRIV_REPLACE;
    ;}
    break;

  case 461:

    {
      (void)(yyval.node);
    ;}
    break;

  case 462:

    {
      (void)(yyval.node);
    ;}
    break;

  case 463:

    {
      /* means global priv_level */
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_LEVEL);
    ;}
    break;

  case 464:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PRIV_LEVEL, 1, (yyvsp[(1) - (1)].node));
    ;}
    break;

  case 465:

    {
      ParseNode *privileges_node = NULL;
      ParseNode *priv_level = NULL;
      merge_nodes(privileges_node, result->malloc_pool_, T_PRIVILEGES, (yyvsp[(2) - (5)].node));
      if ((yyvsp[(3) - (5)].node) == NULL)
      {
        /* check privileges: should have and only have ALL PRIVILEGES, GRANT OPTION */
        int check_ok = 0;
        if (2 == privileges_node->num_child_)
        {
          assert(privileges_node->children_[0]->num_child_ == 0);
          assert(privileges_node->children_[0]->type_ == T_PRIV_TYPE);
          assert(privileges_node->children_[1]->num_child_ == 0);
          assert(privileges_node->children_[1]->type_ == T_PRIV_TYPE);
          if ((privileges_node->children_[0]->value_ == OB_PRIV_ALL
               && privileges_node->children_[1]->value_ == OB_PRIV_GRANT_OPTION)
              || (privileges_node->children_[1]->value_ == OB_PRIV_ALL
                  && privileges_node->children_[0]->value_ == OB_PRIV_GRANT_OPTION))
          {
            check_ok = 1;
          }
        }
        if (!check_ok)
        {
          yyerror(&(yylsp[(1) - (5)]), result, "support only ALL PRIVILEGES, GRANT OPTION");
          YYABORT;
        }
      }
      else
      {
        priv_level = (yyvsp[(3) - (5)].node);
      }
      ParseNode *users_node = NULL;
      merge_nodes(users_node, result->malloc_pool_, T_USERS, (yyvsp[(5) - (5)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_REVOKE,
                                 3, privileges_node, priv_level, users_node);
    ;}
    break;

  case 466:

    {
      (yyval.node) = (yyvsp[(2) - (2)].node);
    ;}
    break;

  case 467:

    {
      (yyval.node) = NULL;
    ;}
    break;

  case 468:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PREPARE, 2, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 469:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 470:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 471:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 472:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 473:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 474:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_VARIABLE_SET, (yyvsp[(2) - (2)].node));;
      (yyval.node)->value_ = 2;
    ;}
    break;

  case 475:

    {
      (yyval.node) = (yyvsp[(3) - (3)].node);
    ;}
    break;

  case 476:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 477:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 478:

    {
      (void)((yyvsp[(2) - (3)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_VAR_VAL, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
      (yyval.node)->value_ = 2;
    ;}
    break;

  case 479:

    {
      (void)((yyvsp[(2) - (3)].node));
      (yyvsp[(1) - (3)].node)->type_ = T_SYSTEM_VARIABLE;
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_VAR_VAL, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
      (yyval.node)->value_ = 2;
    ;}
    break;

  case 480:

    {
      (void)((yyvsp[(3) - (4)].node));
      (yyvsp[(2) - (4)].node)->type_ = T_SYSTEM_VARIABLE;
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_VAR_VAL, 2, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
      (yyval.node)->value_ = 1;
    ;}
    break;

  case 481:

    {
      (void)((yyvsp[(3) - (4)].node));
      (yyvsp[(2) - (4)].node)->type_ = T_SYSTEM_VARIABLE;
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_VAR_VAL, 2, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
      (yyval.node)->value_ = 2;
    ;}
    break;

  case 482:

    {
      (void)((yyvsp[(4) - (5)].node));
      (yyvsp[(3) - (5)].node)->type_ = T_SYSTEM_VARIABLE;
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_VAR_VAL, 2, (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node));
      (yyval.node)->value_ = 1;
    ;}
    break;

  case 483:

    {
      (void)((yyvsp[(4) - (5)].node));
      (yyvsp[(3) - (5)].node)->type_ = T_SYSTEM_VARIABLE;
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_VAR_VAL, 2, (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node));
      (yyval.node)->value_ = 2;
    ;}
    break;

  case 484:

    {
      (void)((yyvsp[(2) - (3)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_VAR_VAL, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
      (yyval.node)->value_ = 2;
    ;}
    break;

  case 485:

    {
      (void)((yyvsp[(2) - (3)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_VAR_VAL, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
      (yyval.node)->value_ = 2;
    ;}
    break;

  case 486:

    {
      (void)((yyvsp[(2) - (3)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_VAR_ARRAY_VAL, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
      (yyval.node)->value_ = 2;
    ;}
    break;

  case 487:

    { (yyval.node) = NULL; ;}
    break;

  case 488:

    { (yyval.node) = NULL; ;}
    break;

  case 489:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 490:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 491:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_ARRAY_VAL, (yyvsp[(2) - (3)].node));
    ;}
    break;

  case 492:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 493:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 494:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_EXECUTE, 2, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 495:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(2) - (2)].node));
    ;}
    break;

  case 496:

    {
      (yyval.node) = NULL;
    ;}
    break;

  case 497:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 498:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 499:

    {
      (void)((yyvsp[(1) - (3)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_DEALLOCATE, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 500:

    { (yyval.node) = NULL; ;}
    break;

  case 501:

    { (yyval.node) = NULL; ;}
    break;

  case 502:

    {
      ParseNode *alter_actions = NULL;
      merge_nodes(alter_actions, result->malloc_pool_, T_ALTER_ACTION_LIST, (yyvsp[(4) - (4)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ALTER_TABLE, 2, (yyvsp[(3) - (4)].node), alter_actions);
    ;}
    break;

  case 503:

    {
      yyerror(&(yylsp[(1) - (6)]), result, "Table rename is not supported now");
      YYABORT;
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_TABLE_RENAME, 1, (yyvsp[(6) - (6)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ALTER_ACTION_LIST, 1, (yyval.node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ALTER_TABLE, 2, (yyvsp[(3) - (6)].node), (yyval.node));
    ;}
    break;

  case 504:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 505:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 506:

    {
      (void)((yyvsp[(2) - (3)].node)); /* make bison mute */
      (yyval.node) = (yyvsp[(3) - (3)].node); /* T_COLUMN_DEFINITION */
    ;}
    break;

  case 507:

    {
      (void)((yyvsp[(2) - (4)].node)); /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_COLUMN_DROP, 1, (yyvsp[(3) - (4)].node));
      (yyval.node)->value_ = (yyvsp[(4) - (4)].ival);
    ;}
    break;

  case 508:

    {
      (void)((yyvsp[(2) - (4)].node)); /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_COLUMN_ALTER, 2, (yyvsp[(3) - (4)].node), (yyvsp[(4) - (4)].node));
    ;}
    break;

  case 509:

    {
      (void)((yyvsp[(2) - (5)].node)); /* make bison mute */
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_COLUMN_RENAME, 2, (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node));
    ;}
    break;

  case 510:

    { (yyval.node) = NULL; ;}
    break;

  case 511:

    { (yyval.node) = NULL; ;}
    break;

  case 512:

    { (yyval.ival) = 2; ;}
    break;

  case 513:

    { (yyval.ival) = 1; ;}
    break;

  case 514:

    { (yyval.ival) = 0; ;}
    break;

  case 515:

    {
      (void)((yyvsp[(3) - (3)].node)); /* make bison mute */
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_CONSTR_NOT_NULL);
    ;}
    break;

  case 516:

    {
      (void)((yyvsp[(3) - (3)].node)); /* make bison mute */
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_CONSTR_NULL);
    ;}
    break;

  case 517:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CONSTR_DEFAULT, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 518:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_NULL);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CONSTR_DEFAULT, 1, (yyval.node));
    ;}
    break;

  case 519:

    {
      merge_nodes((yyval.node), result->malloc_pool_, T_SYTEM_ACTION_LIST, (yyvsp[(4) - (4)].node));
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_ALTER_SYSTEM, 1, (yyval.node));
    ;}
    break;

  case 520:

    {
      ParseNode *node = NULL;
      malloc_terminal_node(node, result->malloc_pool_, T_SET_MASTER_SLAVE);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CHANGE_OBI, 3, node, (yyvsp[(7) - (7)].node), (yyvsp[(3) - (7)].node));
    ;}
    break;

  case 521:

    {
      ParseNode *node = NULL;
      malloc_terminal_node(node, result->malloc_pool_, T_SET_MASTER_SLAVE);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CHANGE_OBI, 3, node, (yyvsp[(7) - (7)].node), (yyvsp[(3) - (7)].node));
    ;}
    break;

  case 522:

    {
      ParseNode *node = NULL;
      malloc_terminal_node(node, result->malloc_pool_, T_SET_MASTER);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CHANGE_OBI, 2, node, (yyvsp[(6) - (6)].node));
    ;}
    break;

  case 523:

    {
      ParseNode *node = NULL;
      malloc_terminal_node(node, result->malloc_pool_, T_SET_SLAVE);
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CHANGE_OBI, 2, node, (yyvsp[(6) - (6)].node));
    ;}
    break;

  case 524:

    {
      (yyval.node) = NULL;
    ;}
    break;

  case 525:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_FORCE);
    ;}
    break;

  case 526:

    {
      (yyval.node) = (yyvsp[(1) - (1)].node);
    ;}
    break;

  case 527:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 528:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SYSTEM_ACTION, 5,
                               (yyvsp[(1) - (9)].node),    /* param_name */
                               (yyvsp[(3) - (9)].node),    /* param_value */
                               (yyvsp[(4) - (9)].node),    /* comment */
                               (yyvsp[(8) - (9)].node),    /* server type */
                               (yyvsp[(9) - (9)].node)     /* cluster or IP/port */
                               );
      (yyval.node)->value_ = (yyvsp[(5) - (9)].ival);                /* scope */
    ;}
    break;

  case 529:

    { (yyval.node) = (yyvsp[(2) - (2)].node); ;}
    break;

  case 530:

    { (yyval.node) = NULL; ;}
    break;

  case 531:

    { (yyval.ival) = 0; ;}
    break;

  case 532:

    { (yyval.ival) = 1; ;}
    break;

  case 533:

    { (yyval.ival) = 2; ;}
    break;

  case 534:

    { (yyval.ival) = 2; ;}
    break;

  case 535:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_INT);
      (yyval.node)->value_ = 1;
    ;}
    break;

  case 536:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_INT);
      (yyval.node)->value_ = 4;
    ;}
    break;

  case 537:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_INT);
      (yyval.node)->value_ = 2;
    ;}
    break;

  case 538:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_INT);
      (yyval.node)->value_ = 3;
    ;}
    break;

  case 539:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_CLUSTER, 1, (yyvsp[(3) - (3)].node));
    ;}
    break;

  case 540:

    {
      malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_SERVER_ADDRESS, 2, (yyvsp[(3) - (6)].node), (yyvsp[(6) - (6)].node));
    ;}
    break;

  case 541:

    { (yyval.node) = NULL; ;}
    break;

  case 542:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 543:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_IDENT);
      (yyval.node)->str_value_ = parse_strdup((yyvsp[(1) - (1)].non_reserved_keyword)->keyword_name, result->malloc_pool_);
      if ((yyval.node)->str_value_ == NULL)
      {
        yyerror(NULL, result, "No more space for string duplicate");
        YYABORT;
      }
      else
      {
        (yyval.node)->value_ = strlen((yyval.node)->str_value_);
      }
    ;}
    break;

  case 544:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 545:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_IDENT);
      (yyval.node)->str_value_ = parse_strdup((yyvsp[(1) - (1)].non_reserved_keyword)->keyword_name, result->malloc_pool_);
      if ((yyval.node)->str_value_ == NULL)
      {
        yyerror(NULL, result, "No more space for string duplicate");
        YYABORT;
      }
      else
      {
        (yyval.node)->value_ = strlen((yyval.node)->str_value_);
      }
    ;}
    break;

  case 547:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 548:

    {
      malloc_terminal_node((yyval.node), result->malloc_pool_, T_IDENT);
      (yyval.node)->str_value_ = parse_strdup((yyvsp[(1) - (1)].non_reserved_keyword)->keyword_name, result->malloc_pool_);
      if ((yyval.node)->str_value_ == NULL)
      {
        yyerror(NULL, result, "No more space for string duplicate");
        YYABORT;
      }
    ;}
    break;

  case 574:

    {
								malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_CREATE, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
							;}
    break;

  case 575:

    {
					ParseNode *params = NULL;
        			merge_nodes(params, result->malloc_pool_, T_PARAM_LIST, (yyvsp[(5) - (6)].node));
					malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_DECL, 2, (yyvsp[(3) - (6)].node), params);
				;}
    break;

  case 576:

    {
					ParseNode *params = NULL;
					malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_DECL, 2, (yyvsp[(3) - (5)].node), params);
				;}
    break;

  case 577:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node));
					;}
    break;

  case 578:

    {
						(yyval.node) = (yyvsp[(1) - (1)].node);
					;}
    break;

  case 579:

    {
                malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PARAM_DEFINITION, 3, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node), NULL);
					;}
    break;

  case 580:

    {
            malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_IN_PARAM_DEFINITION, 3, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node), NULL);
					;}
    break;

  case 581:

    {
            malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OUT_PARAM_DEFINITION, 3, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node), NULL);
					;}
    break;

  case 582:

    {
            malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_INOUT_PARAM_DEFINITION, 3, (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node), NULL);
          ;}
    break;

  case 583:

    {
          ParseNode *array_flag = NULL;
          malloc_terminal_node(array_flag, result->malloc_pool_, T_BOOL);
          array_flag->value_ = 1;
          malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PARAM_DEFINITION, 3, (yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].node), array_flag);
        ;}
    break;

  case 584:

    {
          ParseNode *array_flag = NULL;
          malloc_terminal_node(array_flag, result->malloc_pool_, T_BOOL);
          array_flag->value_ = 1;
          malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_IN_PARAM_DEFINITION, 3, (yyvsp[(2) - (4)].node), (yyvsp[(3) - (4)].node), array_flag);
        ;}
    break;

  case 585:

    {
          ParseNode *array_flag = NULL;
          malloc_terminal_node(array_flag, result->malloc_pool_, T_BOOL);
          array_flag->value_ = 1;
          malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_OUT_PARAM_DEFINITION, 3, (yyvsp[(2) - (4)].node), (yyvsp[(3) - (4)].node), array_flag);
        ;}
    break;

  case 586:

    {
          ParseNode *array_flag = NULL;
          malloc_terminal_node(array_flag, result->malloc_pool_, T_BOOL);
          array_flag->value_ = 1;
          malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_INOUT_PARAM_DEFINITION, 3, (yyvsp[(2) - (4)].node), (yyvsp[(3) - (4)].node), array_flag);
        ;}
    break;

  case 587:

    { 
					//malloc_non_terminal_node($$, result->malloc_pool_, T_PROCEDURE_BLOCK, 1, $2);
					(yyval.node)=(yyvsp[(2) - (3)].node);
				;}
    break;

  case 588:

    { 
					(yyval.node)=NULL;
				;}
    break;

  case 589:

    { 
        			merge_nodes((yyval.node), result->malloc_pool_, T_PROCEDURE_STMTS, (yyvsp[(1) - (1)].node));
				;}
    break;

  case 590:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
				;}
    break;

  case 591:

    {
					(yyval.node)=(yyvsp[(1) - (1)].node);
				;}
    break;

  case 592:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 593:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 594:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 595:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 596:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 597:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 598:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 599:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 600:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 601:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 602:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 603:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 604:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 605:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 606:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 607:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 608:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 609:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 610:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 611:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 612:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 613:

    { (yyval.node) =(yyvsp[(1) - (1)].node); ;}
    break;

  case 614:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 615:

    { 
					(yyval.node)=NULL;
				;}
    break;

  case 616:

    { 
        			merge_nodes((yyval.node), result->malloc_pool_, T_PROCEDURE_STMTS, (yyvsp[(1) - (1)].node));
				;}
    break;

  case 617:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
				;}
    break;

  case 618:

    {
					(yyval.node)=(yyvsp[(1) - (1)].node);
				;}
    break;

  case 619:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 620:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 621:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 622:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 623:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 624:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 625:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 626:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 627:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 628:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 629:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 630:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 631:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 632:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 633:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 634:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 635:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 636:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 637:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 638:

    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 639:

    { (yyval.node) =(yyvsp[(1) - (1)].node); ;}
    break;

  case 640:

    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 641:

    {
						ParseNode *args = NULL;
						merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(2) - (4)].node));
            malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_DECLARE, 4, args, (yyvsp[(3) - (4)].node), NULL, NULL);
					;}
    break;

  case 642:

    {
						ParseNode *args = NULL;
						merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(2) - (6)].node));
            malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_DECLARE, 4, args, (yyvsp[(3) - (6)].node), (yyvsp[(5) - (6)].node), NULL);
          ;}
    break;

  case 643:

    {
            ParseNode *args = NULL;
            ParseNode *arr = NULL;
            malloc_terminal_node(arr, result->malloc_pool_, T_BOOL);
            arr->value_ = 1;
            merge_nodes(args, result->malloc_pool_, T_ARGUMENT_LIST, (yyvsp[(2) - (5)].node));
            malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_DECLARE, 4,
                                     args,  //arguments
                                     (yyvsp[(3) - (5)].node),    //data type
                                     NULL, 	//default value
                                     arr 		//is array
                                     );
          ;}
    break;

  case 644:

    {
						ParseNode *var_list = NULL;
						merge_nodes(var_list, result->malloc_pool_, T_VAR_VAL_LIST, (yyvsp[(2) - (3)].node));
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_ASSGIN, 1,var_list);
					;}
    break;

  case 645:

    {
						ParseNode *elsifs = NULL;
						merge_nodes(elsifs, result->malloc_pool_, T_PROCEDURE_ELSEIFS, (yyvsp[(5) - (9)].node));
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_IF, 4, (yyvsp[(2) - (9)].node), (yyvsp[(4) - (9)].node), elsifs ,(yyvsp[(6) - (9)].node));
					;}
    break;

  case 646:

    {
						ParseNode *elsifs = NULL;
						merge_nodes(elsifs, result->malloc_pool_, T_PROCEDURE_ELSEIFS, (yyvsp[(5) - (8)].node));
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_IF, 4, (yyvsp[(2) - (8)].node), (yyvsp[(4) - (8)].node), elsifs ,NULL);
					;}
    break;

  case 647:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_IF, 4, (yyvsp[(2) - (8)].node), (yyvsp[(4) - (8)].node), NULL ,(yyvsp[(5) - (8)].node));
					;}
    break;

  case 648:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_IF, 4, (yyvsp[(2) - (7)].node), (yyvsp[(4) - (7)].node), NULL ,NULL);
					;}
    break;

  case 649:

    {
						(yyval.node)=(yyvsp[(1) - (1)].node);
					;}
    break;

  case 650:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
					;}
    break;

  case 651:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_ELSEIF, 2, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
					;}
    break;

  case 652:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_ELSE, 1, (yyvsp[(2) - (2)].node));
						//$$ = $2;
					;}
    break;

  case 653:

    {
						ParseNode *casewhenlist = NULL;
						merge_nodes(casewhenlist, result->malloc_pool_, T_PROCEDURE_CASE_WHEN_LIST, (yyvsp[(3) - (7)].node));
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_CASE, 3, (yyvsp[(2) - (7)].node), casewhenlist, (yyvsp[(4) - (7)].node));
					;}
    break;

  case 654:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_LINK_NODE, 2, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
					;}
    break;

  case 655:

    {
						(yyval.node)=(yyvsp[(1) - (1)].node);
					;}
    break;

  case 656:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_CASE_WHEN, 2, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node));
					;}
    break;

  case 657:

    {
						(yyval.node) = NULL;
					;}
    break;

  case 658:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_ELSE, 1, (yyvsp[(2) - (2)].node));
						//$$ = $2;
					;}
    break;

  case 659:

    {
            malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_LOOP, 5,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     (yyvsp[(2) - (5)].node));
          ;}
    break;

  case 660:

    {
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_LOOP, 5,
                                       (yyvsp[(2) - (11)].node),
                                       NULL,
                                       (yyvsp[(4) - (11)].node),
                                       (yyvsp[(6) - (11)].node),
                                       (yyvsp[(8) - (11)].node));
            ;}
    break;

  case 661:

    {
              ParseNode *rev_flag = NULL;
              malloc_terminal_node(rev_flag, result->malloc_pool_, T_BOOL);
              rev_flag->value_ = 1;
              malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_LOOP, 5,
                                       (yyvsp[(2) - (12)].node), 				//loop counter
                                       rev_flag,  //reverse loop
                                       (yyvsp[(5) - (12)].node),        //lowest_number
                                       (yyvsp[(7) - (12)].node),        //highest number
                                       (yyvsp[(9) - (12)].node));       //loop body
            ;}
    break;

  case 662:

    {
						malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_WHILE, 2, (yyvsp[(2) - (7)].node), (yyvsp[(4) - (7)].node));
					;}
    break;

  case 663:

    {
						/* We do not bother building a node for NULL */
						(yyval.node) = NULL;
					;}
    break;

  case 664:

    {
                        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_EXIT, 1, NULL);
						(yyval.node)->value_=1;
					;}
    break;

  case 665:

    {
                        malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_EXIT, 1, (yyvsp[(3) - (4)].node));//when_expr
                    ;}
    break;

  case 666:

    {
        					ParseNode *param_list = NULL;
            				merge_nodes(param_list, result->malloc_pool_, T_EXPR_LIST, (yyvsp[(4) - (6)].node));
        					
                            malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_EXEC, 3, (yyvsp[(2) - (6)].node), param_list, (yyvsp[(6) - (6)].node));
						;}
    break;

  case 667:

    {
							ParseNode *params = NULL;
                            malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_EXEC, 3, (yyvsp[(2) - (5)].node), params, (yyvsp[(5) - (5)].node));
                        ;}
    break;

  case 668:

    {
							malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_DROP, 2, (yyvsp[(5) - (5)].node),(yyvsp[(5) - (5)].node));
						;}
    break;

  case 669:

    {
							malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_DROP, 1, (yyvsp[(3) - (3)].node));
						;}
    break;

  case 670:

    {
							malloc_non_terminal_node((yyval.node), result->malloc_pool_, T_PROCEDURE_SHOW, 1, (yyvsp[(3) - (3)].node));
						;}
    break;



      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, result, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (&yylloc, result, yymsg);
	  }
	else
	  {
	    yyerror (&yylloc, result, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc, result);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp, result);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, result, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc, result);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp, result);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}





void yyerror(YYLTYPE* yylloc, ParseResult* p, char* s, ...)
{
  if (p != NULL)
  {
    p->result_tree_ = 0;
    va_list ap;
    va_start(ap, s);
    vsnprintf(p->error_msg_, MAX_ERROR_MSG, s, ap);
    if (yylloc != NULL)
    {
      if (p->input_sql_[yylloc->first_column - 1] != '\'')
        p->start_col_ = yylloc->first_column;
      p->end_col_ = yylloc->last_column;
      p->line_ = yylloc->first_line;
    }
  }
}

int parse_init(ParseResult* p)
{
  int ret = 0;  // can not include C++ file "ob_define.h"
  if (!p || !p->malloc_pool_)
  {
    ret = -1;
    if (p)
    {
      snprintf(p->error_msg_, MAX_ERROR_MSG, "malloc_pool_ must be set");
    }
  }
  if (ret == 0)
  {
    ret = yylex_init_extra(p, &(p->yyscan_info_));
  }
  return ret;
}

int parse_terminate(ParseResult* p)
{
  return yylex_destroy(p->yyscan_info_);
}

int parse_sql(ParseResult* p, const char* buf, size_t len)
{
  int ret = -1;
  p->result_tree_ = 0;
  p->error_msg_[0] = 0;
  p->input_sql_ = buf;
  p->input_sql_len_ = len;
  p->start_col_ = 1;
  p->end_col_ = 1;
  p->line_ = 1;
  p->yycolumn_ = 1;
  p->yylineno_ = 1;
  p->tmp_literal_ = NULL;

  if (buf == NULL || len <= 0)
  {
    snprintf(p->error_msg_, MAX_ERROR_MSG, "Input SQL can not be empty");
    return ret;
  }

  while(len > 0 && isspace(buf[len - 1]))
    --len;

  if (len <= 0)
  {
    snprintf(p->error_msg_, MAX_ERROR_MSG, "Input SQL can not be while space only");
    return ret;
  }

  YY_BUFFER_STATE bp;

  //bp = yy_scan_string(buf, p->yyscan_info_);
  bp = yy_scan_bytes(buf, len, p->yyscan_info_);
  yy_switch_to_buffer(bp, p->yyscan_info_);
  ret = yyparse(p);
  yy_delete_buffer(bp, p->yyscan_info_);
  return ret;
}

