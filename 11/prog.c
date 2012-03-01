#include <locale.h>
#include <stdio.h>
#include <libintl.h>

#define _(STRING) gettext(STRING)

int main()
{

	setlocale(LC_ALL, "");
	bindtextdomain("prog", "/usr/share/locale");
	textdomain("prog"); // domain name == *.mo file name
	printf(_("Hello world\n"));
	
	
	return 0;
}
