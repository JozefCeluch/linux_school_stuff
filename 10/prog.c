#include <unistd.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

int main()
{
	access("/dev/cdrom", 0);
	perror("access");

	int cd;

	cd = open("/dev/sr0", 2050);

	ioctl(cd, 21257);
	return 0;
}


