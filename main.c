#include <gc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef __MINGW32__
	#include <readline/readline.h>
	#include <readline/history.h>
#endif

#include "node.h"
#include "libhorns.h"
#include "parse.h"
#include "test.h"

extern FILE *yyin;

extern node *NODE_RESULT;

mode_type MODE;

int SUCCESSES;
int FAILURES;

static void usage(char *program) {
	printf("Usage: %s [script]\n", program);
	printf("-e <exp> evaluate\n");
	printf("-h help\n");
	printf("-t test\n");
	printf("-v version\n");
	exit(0);
}

static char *banner() {
	int size = (int) (strlen(VERSION)+strlen(ARCH)+strlen(COPYRIGHT)+6);

	char *result=alloc_string(size);
	(void) snprintf(result, (size_t) size, "Horns v%s %s %s %s", VERSION, PLATFORM, ARCH, COPYRIGHT);
	return result;
}

int main(int argc, char **argv) {
	char *line, *script_name, *exp;

	char c;
	int i;

	FILE *script;

	node *ARGV;

	horns_init();

	NOTHING_TO_DO=0;

	MODE=INTERACTIVE_MODE;

	SUCCESSES=0;
	FAILURES=0;

	line=alloc_string(MAX_LINE);

	script_name="";
	exp="";

	opterr=0;

	while ((int) (c= (char) getopt(argc, argv, "htve:")) != -1) {
		switch(c) {
			case 'h':
				MODE=HELP_MODE;
				break;
			case 't':
				MODE=TEST_MODE;
				break;
			case 'v':
				MODE=VERSION_MODE;
				break;
			case 'e':
				MODE=EXP_MODE;
				exp=optarg;
				break;
			case '?':
				if ((char) optopt == 'e') usage(argv[0]);
		}
	}

	if (optind < argc) {
		MODE=SCRIPT_MODE;
		script_name=argv[optind++];
	}

	ARGV=node_empty_list();

	for (i=optind; i < argc; i++) {
		ARGV=node_append(ARGV, node_str(argv[i]));
	}

	(void) node_set(node_sym("\'ARGV"), ARGV);

	switch(MODE) {
		case HELP_MODE:
			usage(argv[0]);
		case VERSION_MODE:
			printf("%s\n", banner());
			exit(0);
		case TEST_MODE:
			test();
			exit(0);
		case EXP_MODE:
			(void) yy_scan_string(exp);
			parse();

			if (!NOTHING_TO_DO) printf("%s\n", node_string(NODE_RESULT, 1));

			break;
		case INTERACTIVE_MODE:
			printf("%s\n", banner());
			printf("%s\n", HOW_TO_EXIT);

			#ifdef __MINGW32__
			printf("%s", PROMPT);
			while ((line=fgets(line, MAX_LINE, stdin)) != NULL) {
			#else
			while ((line=readline(PROMPT)) != NULL) {
				if (strlen(line)>0) {
					(void) add_history(line);
				}
			#endif

				NOTHING_TO_DO=0;

				if (!(bool) streq(line, "\n")) {
					(void) yy_scan_string(line);
					parse();

					if (!NOTHING_TO_DO) printf("=>%s\n", node_string(NODE_RESULT, 1));
				}

				#ifdef __MINGW32__
				printf("%s", PROMPT);
				#endif
			}

			break;
		case SCRIPT_MODE:
			script=fopen(script_name, "r");
			if (!script) {
				printf("Could not open %s\n", script_name);
				exit(0);
			}

			yyin=script;
			parse();
			(void) fclose(yyin);
	}

	// free(ARGV);
	// free(line);

	return 0;
}
