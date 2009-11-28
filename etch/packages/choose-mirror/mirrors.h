/*
 * Data structure for representing http mirror information.
 * Contains essentially the same information as Mirrors.masterlist,
 * but only as much information as is necessary.
 */
struct mirror_t {
	char *site;
	char *country;
	char *root;
};

/* This is the codename of the preferred distribution; the one that the
 * current version of d-i is targeted at installing. */
#define PREFERRED_DISTRIBUTION "etch"

/* The two strings defined below must match the strings used in the
 * templates (http and ftp) for these options. */
#define NO_MIRROR    "don't use a network mirror"
#define MANUAL_ENTRY "enter information manually"

#define SUITE_LENGTH 32

/* Stack of suites */
static const char suites[][SUITE_LENGTH] = {
	/* higher preference */
	"oldstable",
	"stable",
	"testing",
	"unstable"
	/* lower preference */
};

#define DEBCONF_BASE "mirror/"

/* Allow for one more release than nr. of suites (list terminator) */
#define MAXRELEASES (sizeof(suites)/SUITE_LENGTH + 1)

/*
 * Data structure containing information on releases supported by a mirror
 */
struct release_t {
	char *name;
	char *suite;
	int status;
};

/* Values for status field in release_t */
#define IS_VALID	0x1
#define IS_DEFAULT	0x2
#define GET_SUITE	0x4
#define GET_CODENAME	0x8
