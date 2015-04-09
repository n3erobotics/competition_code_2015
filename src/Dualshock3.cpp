#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <cstdlib>
#include <iostream>
#include <signal.h>

#include "Dualshock3.h"

int Dualshock3::connect() {
	
	axis=NULL;
	num_of_axis=0;
	num_of_buttons=0;
	button=NULL;
	
	cout << "Waiting for joystick... ";

	while(( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 ){
	}

	cout << "Got it!" << endl;

	ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
	ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
	ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

	axis = (int *) calloc( num_of_axis, sizeof( int ) );
	button = (char *) calloc( num_of_buttons, sizeof( char ) );

	printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n"
		, name_of_joystick
		, num_of_axis
		, num_of_buttons );

	fcntl( joy_fd, F_SETFL, O_NONBLOCK );	/* use non-blocking mode */

	cout << "Ready!\n" << endl;

	return 0;
}

void Dualshock3::getData(){
	/* read the joystick state */
	read(joy_fd, (void*) &js, sizeof(struct js_event));
	
	/* see what to do with the event */
	switch (js.type & ~JS_EVENT_INIT){
		case JS_EVENT_AXIS:
			axis   [ js.number ] = js.value;
			break;		
		case JS_EVENT_BUTTON:
			button [ js.number ] = js.value;
			break;
	}
}

void Dualshock3::disconnect() {
	close( joy_fd );
}
