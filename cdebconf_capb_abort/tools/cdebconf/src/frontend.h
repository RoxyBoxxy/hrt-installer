/**
 * @file frontend.h
 * @brief debconf frontend interface
 */
#ifndef _FRONTEND_H_
#define _FRONTEND_H_

#undef _
#define _(x) (x)

struct configuration;
struct template_db;
struct question_db;
struct question;
struct frontend;

#define DCF_CAPB_BACKUP		(1UL << 0)
#define DCF_CAPB_ABORT		(1UL << 1)

/*
 *  For full backup sort, questions are not flagged as being seen
 *  as soon as they are displayed, but only when session is over.
 *  The list of displayed questions is managed by a stack, which
 *  is the seen_questions member of the frontend structure.
 */
enum seen_action {
        STACK_SEEN_ADD,       /*  Add a question to the stack       */
        STACK_SEEN_REMOVE,    /*  Remove a question from the stack  */
        STACK_SEEN_SAVE,      /*  Questions are flagged as seen and
                                  removed from the etack */
        STACK_SEEN_CLEAR      /*  Clear the entire stack            */
};

struct frontend_module {
    int (*initialize)(struct frontend *, struct configuration *);
    int (*shutdown)(struct frontend *);
    unsigned long (*query_capability)(struct frontend *);
    void (*set_title)(struct frontend *, const char *title);
    int (*add)(struct frontend *, struct question *q);
    int (*go)(struct frontend *);
    int (*update_seen_questions)(struct frontend *, enum seen_action);
    void (*clear)(struct frontend *);
    int (*can_go_back)(struct frontend *, struct question *);
    int (*can_go_forward)(struct frontend *, struct question *);
    int (*can_abort)(struct frontend *);

    void (*progress_start)(struct frontend *fe, int min, int max, const char *title);
    void (*progress_set) (struct frontend *fe, int val);
    /* You do not have to implement _step, it will call _set by default */
    void (*progress_step)(struct frontend *fe, int step);
    void (*progress_info)(struct frontend *fe, const char *info);
    void (*progress_stop)(struct frontend *fe);
};

struct frontend {
	/* module handle */
	void *handle;
	/* configuration data */
	struct configuration *config;
    /* config path - base of instance configuration */
    char configpath[DEBCONF_MAX_CONFIGPATH_LEN];
	/* database handle for templates and config */
	struct template_db *tdb;
	struct question_db *qdb;
	/* frontend capabilities */
	unsigned long capability;
	/* private data */
	void *data;

	/* class variables */
	struct question *questions;
	char **seen_questions;
	int interactive;
	char *capb;
	char *title;
	char *progress_title;
    int progress_min, progress_max, progress_cur;
	pid_t pid;
	
	/* methods */
    struct frontend_module methods;
};

struct frontend *frontend_new(struct configuration *, struct template_db *, struct question_db *);
void frontend_delete(struct frontend *);

#endif
