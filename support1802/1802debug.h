const char *opnames[] = {
	"shift1",
	"pushc",
	"push",
	"popc",
	"pop",
	"shr",
	"shru",
	"shl",
	"plus",
	"minus",
	"mul",
	"div",
	"divu",
	"rem",
	"remu",
	"negate",
	"band",
	"or",
	"xor",
	"cpl",
	"assignc",
	"assign",
	"derefc",
	"deref",
	"constc",
	"const",
	"notc",
	"not",
	"boolc",
	"bool",
	"extc",
	"extuc",
	"ext",
	"extu",
	"f2l",
	"l2f",
	"f2ul",
	"ul2f",
	"xxeqc",
	"xxequc",
	"xxeq",
	"xxequ",
	"xxeqpostc",
	"xxeqpost",
	"postincc",
	"postinc",
	"callfname",
	"callfunc",
	"jfalse",
	"jtrue",
	"jump",
	"switchc",
	"switch",
	"cceq",
	"cclt",
	"ccltu",
	"cclteq",
	"ccltequ",
	"nrefc",
	"nref",
	"lrefc",
	"lref",
	"nstorec",
	"nstore",
	"lstorec",
	"lstore",
	"local",
	"plusconst",
	"plus4",
	"plus3",
	"plus2",
	"plus1",
	"minus4",
	"minus3",
	"minus2",
	"minus1",
	"fnenter",
	"fnexit",
	"cleanup",
	"native",
	"byte",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"shift0",
	"pushl",
	"popl",
	"shrl",
	"shrul",
	"shll",
	"plusf",
	"plusl",
	"minusf",
	"minusl",
	"mulf",
	"mull",
	"divf",
	"divl",
	"divul",
	"remf",
	"reml",
	"remul",
	"negatef",
	"negatel",
	"bandl",
	"orl",
	"xorl",
	"cpll",
	"assignl",
	"derefl",
	"constl",
	"notl",
	"booll",
	"xxeql",
	"xxequl",
	"xxeqpostl",
	"postincf",
	"postincl",
	"switchl",
	"cceqf",
	"cceql",
	"ccltf",
	"ccltl",
	"ccltul",
	"cclteqf",
	"cclteql",
	"ccltequl",
	"nrefl",
	"lrefl",
	"nstorel",
	"lstorel",
	"r0refc",
	"r0ref",
	"r0storec",
	"r0store",
	"r0derefc",
	"r0deref",
	"r0inc1",
	"r0inc2",
	"r0dec",
	"r0dec2",
	"r0drfpost",
	"r0drfpre",
	"r1refc",
	"r1ref",
	"r1storec",
	"r1store",
	"r1derefc",
	"r1deref",
	"r1inc1",
	"r1inc2",
	"r1dec",
	"r1dec2",
	"r1drfpost",
	"r1drfpre",
	"r2refc",
	"r2ref",
	"r2storec",
	"r2store",
	"r2derefc",
	"r2deref",
	"r2inc1",
	"r2inc2",
	"r2dec",
	"r2dec2",
	"r2drfpost",
	"r2drfpre",
	"r3refc",
	"r3ref",
	"r3storec",
	"r3store",
	"r3derefc",
	"r3deref",
	"r3inc1",
	"r3inc2",
	"r3dec",
	"r3dec2",
	"r3drfpost",
	"r3drfpre",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
