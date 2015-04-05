/*
 * SerialPort.h
 *
 *  Created on: Jun 20, 2011
 *      Author: jose
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include <stdio.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <fcntl.h>
#include <unistd.h>

using namespace std;


class SerialPort {
private:
	int fileDescriptor;

public:
	int connect (const char device[]);
	void disconnect(void);

	void sendArray(string buffer, int len);
	int getArray (string *buffer, int len);

	int bytesToRead();
	void clear();
};


#endif /* SERIALPORT_H_ */
