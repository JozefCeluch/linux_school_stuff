#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include <errno.h>
#include <pwd.h>

/*Program prints same output as unix command id*/

int main(int argc, char *argv[])
{
	if (argc > 1) {
		printf("Usage: %s Program takes no arguments\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	uid_t user_id;
	struct passwd *pwd_result; /*stores entry from /etc/passwd file*/

	struct group *gr; /*used to print groups*/
	gid_t *groups; /*stores all groups of which user is a member*/
	int ngroup_max; /*maximum number of groups*/
	int i; /*counter*/
	int ngroup;

	user_id = getuid();
	pwd_result = (struct passwd *) malloc(sizeof(struct passwd));
	if (pwd_result == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	ngroup_max = sysconf(_SC_NGROUPS_MAX);
	if (ngroup_max == -1) {
		ngroup_max = 65536;
	}
	groups = (gid_t *) malloc(ngroup_max * sizeof(gid_t));
	if (groups == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	errno = 0;
	pwd_result = getpwuid(user_id);
	if (pwd_result == NULL) {
		if (errno == 0) {
			printf("UID not found\n");
		} else {
			perror("getpwnam_r");
		}
		exit(EXIT_FAILURE);
	}

	errno = 0;
	ngroup = getgroups(ngroup_max, groups);
	if (errno != 0) {
		perror("getgroups");
		exit(EXIT_FAILURE);
	}

	printf("uid=%d(%s) gid=%d(%s) groups=%d(%s)", pwd_result->pw_uid,
			pwd_result->pw_name, pwd_result->pw_gid,
			pwd_result->pw_name, pwd_result->pw_gid,
			pwd_result->pw_name);

	for (i = 0; i < ngroup; i++) {
		gr = getgrgid(groups[i]);
		if ((gr != NULL) && (gr->gr_gid != pwd_result->pw_gid)) {
			printf(",%d", groups[i]);
			printf("(%s)", gr->gr_name);
		}
	}
	printf("\n");

	exit(EXIT_SUCCESS);
}

