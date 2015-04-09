#ifndef DUALSHOCK_H_
#define DUALSHOCK_H_

#include <stdio.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <cstdlib>
#include <signal.h>

#define JOY_DEV "/dev/input/js0"

using namespace std;

class Dualshock3 {
private:
	int num_of_axis, num_of_buttons, x, joy_fd;
	char name_of_joystick[80];
	struct js_event js;

public:
	int *axis;
	char *button;

	int connect ();
	void getData();
	void disconnect();
};


#endif
