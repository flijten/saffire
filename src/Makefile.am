########################################################################
# INIT
########################################################################

# Set our optimization / debug flags
if DEBUG
  AM_CFLAGS = -ggdb  -O0 -D__DEBUG
  AM_CXXFLAGS = -ggdb -O0 -D__DEBUG
else
  AM_CFLAGS = -O2
  AM_CXXFLAGS = -O2
endif

if PARSEDEBUG
  AM_CFLAGS += -D__PARSEDEBUG
  AM_YFLAGS = -t -v
else
  AM_YFLAGS =
endif

# Add top include dir
AM_CFLAGS += -I$(top_srcdir)/src/include

# Complain about everything, except unused functions
AM_CFLAGS += -Wall -Wno-unused-function

noinst_LIBRARIES =

########################################################################
# components/compiler
########################################################################

# Some additional rules for flex/bison file handling
BUILT_SOURCES=components/compiler/lex.yy.c
AM_LFLAGS = --outfile=components/compiler/$(LEX_OUTPUT_ROOT).c --header-file=include/compiler/$(LEX_OUTPUT_ROOT).h
AM_YFLAGS += -d

# parser.tab.h cannot be used before parser.tab.c
include/compiler/parser.tab.h: components/compiler/parser.tab.c
	mv compiler/parser.tab.h include/compiler/parser.tab.h

# parser.tab.c is created by bison
components/compiler/parser.tab.c: components/compiler/saffire.y
	$(YACC) $(AM_YFLAGS) -o $@ $<
	mv components/compiler/parser.tab.h include/compiler/parser.tab.h

# lex.yy.c is created by lex
components/compiler/lex.yy.c: components/compiler/saffire.l
	$(LEX) $(AM_LFLAGS) $<

# Cleanup these files on a "make clean"
CLEANFILES = components/compiler/parser.tab.c \
             include/compiler/parser.tab.h \
             components/compiler/lex.yy.c \
             include/compiler/lex.yy.h


noinst_LIBRARIES += libcompiler.a
libcompiler_a_SOURCES = components/compiler/parser.tab.c \
                        components/compiler/lex.yy.c \
                        components/compiler/ast.c \
                        components/compiler/dot.c \
                        components/compiler/bytecode.c \
                        components/compiler/saffire_compiler.c


########################################################################
# components/fastcgi
########################################################################

noinst_LIBRARIES += libfastcgi.a
libfastcgi_a_SOURCES = components/fastcgi/fastcgi_srv.c


########################################################################
# components/general
########################################################################

noinst_LIBRARIES += libgeneral.a
libgeneral_a_SOURCES = components/general/hashtable.c \
                       components/general/hash/chained.c \
                       components/general/hash/hash_funcs.c \
                       components/general/smm.c \
                       components/general/md5.c \
                       components/general/dll.c \
                       components/general/ini.c \
                       components/general/stack.c \
                       components/general/parse_options.c


########################################################################
# components/interpreter
########################################################################

noinst_LIBRARIES += libinterpreter.a
libinterpreter_a_SOURCES = components/interpreter/interpreter.c \
                           components/interpreter/context.c \
                           components/interpreter/errors.c


########################################################################
# components/modules
########################################################################

noinst_LIBRARIES += libmodules.a
libmodules_a_SOURCES = components/modules/module_api.c \
                       components/modules/io.c \
                       components/modules/saffire.c


########################################################################
# components/objects
########################################################################

noinst_LIBRARIES += libobjects.a
libobjects_a_SOURCES = components/objects/object.c \
                       components/objects/base.c \
                       components/objects/null.c \
                       components/objects/boolean.c \
                       components/objects/numerical.c \
                       components/objects/string.c \
                       components/objects/regex.c \
                       components/objects/code.c \
                       components/objects/method.c


########################################################################
# components/repl
########################################################################

noinst_LIBRARIES += librepl.a
librepl_a_SOURCES = components/repl/repl.c


########################################################################
# components/vm
########################################################################

noinst_LIBRARIES += libvm.a
libvm_a_SOURCES = components/vm/vm.c


########################################################################
# main/saffire
########################################################################

SAFFIRE_LIBS = \
                 libinterpreter.a \
                 libobjects.a \
                 libcompiler.a \
                 libfastcgi.a \
                 librepl.a \
                 libvm.a \
                 libmodules.a \
                 libgeneral.a



bin_PROGRAMS = saffire

saffire_LDADD = $(SAFFIRE_LIBS) -lpcre -lfcgi

saffire_SOURCES = main/saffire.c \
                  main/commands/config.c main/commands/fastcgi.c main/commands/lint.c \
                  main/commands/exec.c main/commands/cli.c main/commands/help.c \
                  main/commands/version.c main/commands/compile.c

