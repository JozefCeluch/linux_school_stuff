#include <time.h>
#include <stdlib.h>

struct tm *localtime(const time_t *timep){

	struct tm *locTime=(struct tm*)malloc(sizeof(struct tm));
	(*locTime).tm_sec = 59;
	(*locTime).tm_min = 59;
	(*locTime).tm_hour = 26;
	(*locTime).tm_mday = 1;
	(*locTime).tm_mon = 1;
	(*locTime).tm_year = 1000;
	(*locTime).tm_wday = 1;
	(*locTime).tm_yday = 1;
	(*locTime).tm_isdst = 1;
	return locTime;
}
