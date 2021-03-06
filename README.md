# Working Development Tree For the Fuzix Compiler Kit

## Design

cc0 is a tool that tokenizes a C file and handles all the messy
number conversions and string quoting to produce a token stream for a
compiler proper to consume. It also extracts all the identifiers and numbers
them, before writing them out in a table.

cc1 takes the tokenized stream and generates an output stream that consists
of descriptors of program structure (function/do while/statement etc) with
expression trees embedded within.

cc2 will then turn this into code.

In theory it ought to also be possible to add a cc1b that further optimizes the
trees from cc1.

## Status

The compiler can parse and generate output for the full Fuzix codebase. The
code generator being worked on first is for the 8085 and it's currently at
the point it can build a kernel and user space although the userspace at
this point dies somewhere in init. The code generator still needs some work,
and the support libraries a fair bit of debugging. The front end and core
compiler should now be reasonably stable although some bugs undoubtedly
remain.

## Installation

As a cross compiler the front end expects it all to live in /opt/cc85. The
tool chain provides the compiler front end and phases. For cpp for now
symlink the native gcc /lib/cpp.

The assembler, linker and support tools are the 8085 pieces from the
assembler/linker toolchain currently in the CC6303 repository. In the
as68 directory of that git do a make clean; make -f Makefile.8085 and then
copy the various xx85 tools it produces into the /opt/cc85 space.

## Intended C Subset

The goal is to support the following

### Types

* char, short, int, long, signed and unsigned
* float, double
* struct, union
* enum
* typedef

Currently the compiler requires that the target types all fit into the host
unsigned long type.

Currently the compiler hardcodes assumptions that a char is 8bits, short
16bit and long 32bits (see tree.c:constify and helpers). This needs to be
addressed.

### Storage classes

auto, static, extern, typedef

register is accepted but currently ignored.

### C Syntax

* standard keywords and flow control
* labels, and goto
* statements and expressions
* declarations
* ANSI C function declarations

### Intentionally Omitted

Things that add size and complexity or are just pointless.

* K&R function declarations
* Most C95 stuff - wide char, digraph etc
* Most C99 bloat by committee
* C11 bloat by committee
* struct/union passing, struct/union returns and other related badness
* bitfields
* const and volatile typing. To do these makes type handling really really tricky. They are accepted so that code with them can build.

###

Known incompatibilities (some to be fixed)

* The constant value -32768 does not always get typed correctly. The reason for this is a complicated story about how cc0/cc1 interact.
* Many C compilers permit (void) to 'cast' the result of a call away, we do not.
* Local variables have a single function wide scope not a block scope

## Internals

### cc0

Takes input from stdin and outputs tokens to stdout. The core of the logic
is pretty basic, the only oddity is using strchr() in a few places because
it's often hand optimized assembler. Tokens are 16bits. C has some specific
rules on tokenizing which make it simple at the cost of producing unexpected
results from stuff like x+++++y; (x++ ++ +y).

All names are translated into a 16bit token number. So for example every
occurence of "fred" might be 0x8004. The cc0 stage has no understanding of
C scoping so 0x8004 isn't tied to any kind of scope, merely a group of
letters.

After tokenizing it writes the symbol table out to disk as well. It turns
out that the compiler phase has no use at all for symbol names and they
take a lot of space to store and slow down comparisons.

### cc1

This is essentially a hand coded recursive descent parser. Higher level
constructs are described by headers and footers. Within these blocks the
compiler stores expression trees per statement. Trees do not span statements
nor does the compiler do anything at a higher level. There is enough
information to turn functions or even entire programs into a single tree if
the code generator or an optimizer pass wished.

The biggest challenge on a small machine is the memory management. To keep
things tight types are packed into 16bits. Where the type is complex it
contains an index to an object in the symbol table which describes the type
in question (and if the type is named also has the type naming attached).

Various per object fields are packed into runs of 16bit values, such as
struct field information and array sizes.

To maximise memory efficiency without losing the checking the compiler packs
all functions with the same signature into the same type. As most functions
actually have one of a very small number of prototypes this saves a lot of
room.

### cc2

For now just testing a very simple left hand walking code generator with
minimal awareness of consts and names that can be directly accessed. This
should suit simpler processors like the 6502, 680x, 8080, 8085 etc but isn't
a good model for register oriented ones.

On the other hand it's ludicrously easy to change it to produce fairly bad
code for any processor you want.

## Credits

The expression parser was created by turning the public domain SmallC 3.0 one
into a more traditional tree building recursive parser and testing it in
SmallC. The rest of the code is original although the design is influenced by
several small C subset compilers and also ANSI pcc.

## Licence

Compiler (not any runtime)	:	GPLv3

copt is from Z88DK. Z88DK is under the Clarified Artistic License
