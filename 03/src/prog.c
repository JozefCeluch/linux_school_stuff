#include "prog.h"
#include "debug_msgs.h"

int check_age(char *birth_date)
{

	if (!birth_date) {
		printf("%s\n", DATE_FORMAT_MSG);
		return FAILURE;
	}
	if (strlen(birth_date) != 10) {
		printf("%s\n", DATE_FORMAT_MSG);
		return FAILURE;
	}
	int error_count = 0;
	int i = 0;

	for (i = 0; i < strlen(birth_date); i++) {

		if (i == 2 || i == 5) {
			if (birth_date[i] != '.') {
				error_count++;
			}
		} else if (!isdigit(birth_date[i])) {
			error_count++;
		}
	}
	if (error_count) {
		printf("%s\n", DATE_FORMAT_MSG);
		return FAILURE;
	}

	time_t now;
	time(&now);
	struct tm *time_now_struct = (struct tm*) malloc(sizeof(struct tm));
	time_now_struct = localtime(&now);

	char year_now[5];
	year_now[4] = '\0';
	strncpy(year_now, ctime(&now) + 20, 4);

	char birth_day[3] = "\0\0\0";
	char birth_month[3] = "\0\0\0";
	char birth_year[5] = "\0\0\0\0\0";
	strncpy(birth_day, birth_date, 2);
	strncpy(birth_month, birth_date + 3, 2);
	strncpy(birth_year, birth_date + 6, 4);

	if ((atoi(year_now) - atoi(birth_year)) > 18) {
		DBG(STDERR, "%d\n",(atoi(year_now) - atoi(birth_year)));
		return SUCCESS;
	} else if ((atoi(year_now) - atoi(birth_year)) == 18) {
		if ((*time_now_struct).tm_mon + 1 > atoi(birth_month)) {
			DBG(STDERR, "OK\n");
			return SUCCESS;
		} else if (((*time_now_struct).tm_mon + 1 == atoi(birth_month))
				&& ((*time_now_struct).tm_mday >= atoi(birth_day))) {
			DBG(STDERR, "OK\n");
			return SUCCESS;
		} else {
			DBG(STDERR, "no OK\n");
			return FAILURE;
		}

		DBG(STDERR, "no OK\n");
		return FAILURE;
	} else {
		DBG(STDERR, "no OK\n");
		return FAILURE;
	}

}

//int main(int argc, char *argv[])
//{
//	int a = check_age(argv[1]);
//	return a;
//}
