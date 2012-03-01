#include <check.h>
#include <stdlib.h>
#include "../src/prog.h"

START_TEST (test_check_age_old)
{
	char *day = "11.10.1993";
	fail_unless(check_age(day) == 0,
			"Correct date falsely rejected");
}END_TEST

START_TEST (test_check_age_young)
{
	char *day = "02.10.2011";
	fail_unless(check_age(day) == 1,
			"Too young falsely approved");
}END_TEST

START_TEST (test_check_age_future)
{
	char *day = "22.12.2012";
	fail_unless(check_age(day) == 1,
			"This guy is not even born yet");
}END_TEST

START_TEST (test_check_age_incorrect_format)
{
	char *day = "10.1.2005";
	fail_unless(check_age(day) == 1,
			"Wrong date format");
}END_TEST

START_TEST (test_check_age_null_argument)
{
	char *day = NULL;
	fail_unless(check_age(day) == 1,
			"Should not fall with null argument");
}END_TEST

START_TEST (test_check_age_random_string)
{
	char *day = "1a2s3d4d5f";
	fail_unless(check_age(day) == 1,
			"Random text string is not a date");
}END_TEST

Suite *check_age_suite (void)
{
	Suite *s = suite_create ("Age");

	/* Logic test case */
	TCase *tc_logic = tcase_create ("Logic");
	tcase_add_test (tc_logic, test_check_age_old);
	tcase_add_test (tc_logic, test_check_age_young);
	tcase_add_test (tc_logic, test_check_age_future);

	/* Input format test case*/
	TCase *tc_format = tcase_create ("Format");
	tcase_add_test (tc_format, test_check_age_incorrect_format);
	tcase_add_test (tc_format, test_check_age_null_argument);
	tcase_add_test (tc_format, test_check_age_random_string);

	suite_add_tcase (s, tc_logic);
	suite_add_tcase (s, tc_format);

	return s;
}


int main (void)
{
	int number_failed;
	Suite *s = check_age_suite ();
	SRunner *sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


