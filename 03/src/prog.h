
#ifndef PROG_H_
#define PROG_H_

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define SUCCESS 0
#define FAILURE 1
#define DATE_FORMAT_MSG "Date must be in format: dd.mm.yyyy"


int check_age (char *birth_date);

#endif /* PROG_H_ */
