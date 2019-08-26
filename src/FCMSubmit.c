/*
 * FCMSubmit
 * 	Submit Firebase Cloud Messaging notification
 * 	Script shell version
 *
 * 	Copyright 2019 Laurent Faillie
 *
 * 		FCMSubmit is covered by
 *		Creative Commons Attribution-NonCommercial 3.0 License
 *		(http://creativecommons.org/licenses/by-nc/3.0/) 
 *		Consequently, you're free to use if for personal or non-profit usage,
 *		professional or commercial usage REQUIRES a commercial licence.
 *
 *		Majordome is distributed in the hope that it will be useful,
 *		but WITHOUT ANY WARRANTY; without even the implied warranty of
 *		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	26/08/2019 - LF - Start of development
 */

#include <stdlib.h>		/* exit(), ... */
#include <unistd.h>		/* getopt(), ... */
#include <libgen.h>		/* basename(), ... */
#include <stdio.h>

	/***
	 * Default configuration
	 ***/
#define DEFAULT_CONFIGURATION_FILE	"/usr/local/etc/FCMSubmit.conf"
#define VERSION "0.1"

int main( int ac, char ** av){
	const char *conf_file = DEFAULT_CONFIGURATION_FILE;

	int c;
	while((c = getopt(ac, av, "vhf:t")) != EOF) switch(c){
	case 'h':
		fprintf(stderr, "%s (%s)\n"
			"Submit Firebase Cloud Messaging notification\n"
			"Known options are :\n"
			"\t-h : this online help\n"
			"\t-v : enable verbose messages\n"
			"\t-f<file> : read <file> for configuration\n"
			"\t\t(default is '%s')\n" ,
			basename(av[0]), VERSION, DEFAULT_CONFIGURATION_FILE
		);
		exit(EXIT_FAILURE);
		break;	
	}

}
