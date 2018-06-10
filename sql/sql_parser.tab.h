
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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



