#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>

/*define acceptable arguments*/
#define ARGUMENTS "hsEq"

/*define user message*/
#define USR_MESSAGE "To continue using program choose another option, -h for help"

/*loads library and run loaded function*/
int libLoad (char *libName) 
{
	void *handle;	/* pointer to library */
	char *error;	
	void *(*function)();	/* pointer to function */
	handle = dlopen(libName, RTLD_NOW); 

	if (!handle) {
		fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
        }
        dlerror();    /* clear any existing error */

	function = dlsym(handle, "hello_world");

	if ((error = dlerror()) != NULL)  {
		fputs(error, stderr);
		exit(EXIT_FAILURE);
	}
	function();		/* execute loaded function */
	dlclose(handle);	
	return EXIT_SUCCESS;
}

/* Print how to use this program */
void usage(char *progName)
{	
	printf("Usage: %s [-hsEq]\n\n", progName);
	printf(" -h     Show this help\n");
	printf(" -s     Print Hello in slovak\n");
	printf(" -E     Print Hello in English\n");
	printf(" -q     Quit program\n");
}


int main (int argc, char *argv[])
{
	int c;
	char *libName = NULL;
	
	if (argc == 1) {
		usage(argv[0]);
		return 0;
	}

	while ((c = getopt(argc, argv, ARGUMENTS)) != -1) {
		switch (c) {
		case 'h': /* show help */
			usage(argv[0]);
			break;

		case 'E': /* use libhello1 */
			libName = "libhello1.so.0.0.1";
			break;
		case 's': /* use libhello2 */

			libName = "libhello2.so.0.0.1";
			break;
		case 'q': /* quit program */
			return EXIT_SUCCESS;

		default : /* show help, failure */
			usage(argv[0]);
			return EXIT_FAILURE;
		}
	if (libName) {
		libLoad(libName);
		libName = NULL;
	}

	if (optind == argc) {
		printf("\n%s\n",USR_MESSAGE); 
		optind = 1;	/* iterate arguments from the begining */
		argc = 2;		/* set number of arguments */	
		scanf("%s", argv[1]); 
		
	}

	}

	return EXIT_SUCCESS;
}

