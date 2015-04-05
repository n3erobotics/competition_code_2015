/*
 * SerialPort.cpp
 *
 *  Created on: Jun 20, 2011
 *      Author: jose
 */

#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "SerialPort.h"

extern pthread_mutex_t access_HUB;

int SerialPort::connect(const char device[]) {
	struct termios terminalAttributes;

	/*
	 * http://linux.die.net/man/2/open
	 *
	 * Open the serial port
	 * read/write
	 * not become the process's controlling terminal
	 * When possible, the file is opened in nonblocking mode
	 *
	 */
	fileDescriptor = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_FSYNC );

	// clear terminalAttributes data
	memset(&terminalAttributes, 0, sizeof(struct termios));

	/*	http://linux.die.net/man/3/termios
	 *
	 *  control modes: c_cflag flag constants:
	 *
	 * 115200 bauds
	 * 8 bits per word
	 * Ignore modem control lines.
	 * Enable receiver.
	 */

	terminalAttributes.c_cflag = B115200 | CS8 | CLOCAL | CREAD;

	/*
	 * input modes: c_iflag flag constants:
	 *
	 * Ignore framing errors and parity errors.
	 * (XSI) Map NL to CR-NL on output.
	 */
	terminalAttributes.c_iflag = IGNPAR |  ONLCR;

	/*
	 * output modes: flag constants defined in POSIX.1
	 *
	 * Enable implementation-defined output processing.
	 */

	terminalAttributes.c_oflag = OPOST;

	/*
	 * Canonical and noncanonical mode
	 *
	 * min time
	 * min bytes to read
	 */

	//terminalAttributes.c_lflag = ICANON;
	terminalAttributes.c_cc[VTIME] = 0;
	terminalAttributes.c_cc[VMIN] = 1;

	/*
	 * http://linux.die.net/man/3/tcsetattr
	 * Set the port to our state
	 *
	 * the change occurs immediately
	 */

	tcsetattr(fileDescriptor, TCSANOW, &terminalAttributes);

	/*
	 * http://linux.die.net/man/3/tcflush
	 *
	 * flushes data written but not transmitted.
	 * flushes data received but not read.
	 */

	tcflush(fileDescriptor, TCOFLUSH);
	tcflush(fileDescriptor, TCIFLUSH);

	return fileDescriptor;
}

void SerialPort::disconnect(void)
{
    close(fileDescriptor);
    printf("\nPort 1 has been CLOSED and %d is the file description\n", fileDescriptor);
}

void SerialPort::sendArray(string buffer, int len) {

	//pthread_mutex_lock(&access_HUB);

	//cout << "Sending.. " << buffer.c_str();
	int n=write(fileDescriptor, buffer.c_str(), len);
	//pthread_mutex_unlock(&access_HUB);

	usleep(5000);
	if(n<=0)
		cout << "ERROR: \"" << buffer.c_str()<< "\"  not sent through SERIALPORT" << endl;
}

int SerialPort::getArray (string *buffer, int len)
{
	int n=0;
	char c_buffer[len];
	if(bytesToRead()>=len){
		n=read(fileDescriptor, c_buffer, len);
		buffer->assign(c_buffer);
	}
	tcflush(fileDescriptor, TCIFLUSH);
	return n;
}

void SerialPort::clear()
{
	tcflush(fileDescriptor, TCIFLUSH);
	tcflush(fileDescriptor, TCOFLUSH);
}

int SerialPort::bytesToRead()
{
	int bytes=0;
	ioctl(fileDescriptor, FIONREAD, &bytes);

	return bytes;
}
