#include "confmodule.h"
#include "commands.h"
#include "frontend.h"
#include "database.h"
#include "question.h"
#include "strutl.h"
#include "debconfclient.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

static commands_t commands[] = {
    { "input",	    command_input },
    { "clear",	    command_clear },
    { "version",	command_version },
    { "capb",	    command_capb },
    { "title",	    command_title },
    { "beginblock",	command_beginblock },
    { "endblock", 	command_endblock },
    { "go",		    command_go },
    { "get",	    command_get },
    { "set",	    command_set },
    { "reset",	    command_reset },
    { "subst",	    command_subst },
    { "register",	command_register },
    { "unregister",	command_unregister },
    { "purge",	    command_purge },
    { "metaget",	command_metaget },
    { "fget",	    command_fget },
    { "fset",	    command_fset },
    { "exist",	    command_exist },
    { "stop",	    command_stop },
    { "progress",   command_progress },
    { "x_loadtemplatefile", command_x_loadtemplatefile },
    { "settitle",   command_settitle },
    { 0, 0 }
};

/* private functions */
/*
 * @brief helper function to process incoming commands
 * @param struct confmodule *mod - confmodule object
 * @param char *in - input command
 * @param char *out - reply buffer
 * @param size_t outsize - reply buffer length
 * @return int - DC_OK, DC_NOTOK, DC_NOTIMPL
 */
static char *_confmodule_process(struct confmodule *mod, char *in)
{
	int i;
	char *argv[2] = { "", "" };

	if (*in == '#') return NULL;

	strcmdsplit(in, argv, DIM(argv));

	for (i = 0; commands[i].command != 0; i++)
	{
		if (strcasecmp(argv[0], commands[i].command) == 0) {
			return (*commands[i].handler)(mod, argv[1]);
		}
	}
	return NULL;
}

/* public functions */
static int confmodule_communicate(struct confmodule *mod)
{
    char buf[1023];
    char *in;
    size_t insize = 1024;
    char *out;
//    size_t outsize = 4096;
    char *inp;
    int ret = 0;

    in = malloc(insize);
    if (!in)
        return DC_NOTOK;
    memset(in, 0, insize);

//    out = malloc(outsize);
//    if (!out)
//        return DC_NOTOK;
//    memset(out, 0, outsize);

    while (1) {
        buf[0] = 0;
        in[0] = 0;
        while (strchr(buf, '\n') == NULL) {
            ret = read(mod->infd, buf, sizeof(buf));
            if (ret <= 0)
                return DC_NOTOK;
            buf[ret] = 0;
            if (strlen(in) + ret + 1 > insize) {
                insize += sizeof(buf);
                in = realloc(in, insize);
            }
            strcat(in, buf);
        }

        inp = strstrip(in);
        INFO(INFO_DEBUG, "--> %s\n", inp);
        out = _confmodule_process(mod, inp);
        if (out == NULL) {
            fprintf(stderr, "E: Unimplemented function\n");
            continue;
        }
        /*		if (out[0] == 0) break; // STOP called*/
        INFO(INFO_DEBUG, "<-- %s\n", out);
        write(mod->outfd, out, strlen(out));
        write(mod->outfd, "\n", 1);
        free(out);
    }
    return ret;
}

static char *confmodule_process_command(struct confmodule *mod, char *cmd)
{
    char *inp;
    char *out;

    inp = strstrip(cmd);

    INFO(INFO_DEBUG, "--> %s\n", inp);
    out = _confmodule_process(mod, inp);
    if (out == NULL) {
        fprintf(stderr, "E: Unimplemented function\n");
        asprintf(&out, "%u Not implemented", DC_NOTOK);
    }
    /*		if (out[0] == 0) break; // STOP called*/
    INFO(INFO_DEBUG, "<-- %s\n", out);

    return out;
}

static int confmodule_shutdown(struct confmodule *mod)
{
    int status;

    mod->exitcode = 0;

    waitpid(mod->frontend->pid, &status, 0);

    if (WIFEXITED(status))
        mod->exitcode = WEXITSTATUS(status);

    return mod->exitcode;
}

static int confmodule_run(struct confmodule *mod, int argc, char **argv)
{
	int pid;
	int i;
	char **args;
	int oldstdin = -1, oldstdout = -1;
	int toconfig[2], fromconfig[2]; /* 0=read, 1=write */
	pipe(toconfig);
	pipe(fromconfig);
	switch ((pid = fork()))
	{
	case -1:
		mod->frontend->methods.shutdown(mod->frontend);
		DIE("Cannot execute client config script");
		break;
	case 0:
		mod->frontend->pid = pid;
		oldstdout = dup(1);
		if (toconfig[0] != 0) { /* if stdin is closed initially */
			oldstdin = dup(0);
			dup2(toconfig[0], 0); close(toconfig[0]);
		}
		close(fromconfig[0]); close(toconfig[1]);
		dup2(fromconfig[1], 1); close(fromconfig[1]);
		if (oldstdin != -1 && oldstdin != DEBCONF_OLD_STDIN_FD) {
			dup2(oldstdin, DEBCONF_OLD_STDIN_FD);
			close(oldstdin);
		}
		if (oldstdout != -1 && oldstdout != DEBCONF_OLD_STDOUT_FD) {
			dup2(oldstdout, DEBCONF_OLD_STDOUT_FD);
			close(oldstdout);
		}

		args = (char **)malloc(sizeof(char *) * argc);
		for (i = 1; i < argc; i++)
			args[i-1] = argv[i];
		args[argc-1] = NULL;
		if (execv(argv[1], args) != 0)
			perror("execv");
		/* should never reach here, otherwise execv failed :( */
		DIE("Cannot execute client config script");
	default:
		close(fromconfig[1]); close(toconfig[0]);
		mod->infd = fromconfig[0];
		mod->outfd = toconfig[1];
	}

	return pid;
}

struct confmodule *confmodule_new(struct configuration *config,
	struct template_db *templates, struct question_db *questions, 
    struct frontend *frontend)
{
	struct confmodule *mod = NEW(struct confmodule);
	memset(mod, 0, sizeof(struct confmodule));

	mod->config = config;
	mod->templates = templates;
	mod->questions = questions;
	mod->frontend = frontend;
	mod->run = confmodule_run;
	mod->communicate = confmodule_communicate;
	mod->process_command = confmodule_process_command;
	mod->shutdown = confmodule_shutdown;

	/* TODO: I wish we don't need gross hacks like this.... */
	setenv("DEBIAN_HAS_FRONTEND", "1", 1);

	return mod;
}

void confmodule_delete(struct confmodule *mod)
{
	DELETE(mod);
}

