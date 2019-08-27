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
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

	/***
	 * Default configuration
	 ***/

#define DEFAULT_CONFIGURATION_FILE	"/usr/local/etc/FCMSubmit.conf"
#define VERSION "0.1"


	/***
	 * Configuration
	 ***/

#define MAXLINE 1024

bool verbose = false;
const char *token = NULL;
const char *senderID = NULL;


	/***
	 * Helpers
	 ***/

char *removeLF(char *s){
	size_t l=strlen(s);
	if(l && s[--l] == '\n')
		s[l] = 0;
	return s;
}

char *striKWcmp( char *s, const char *kw ){
/* compare string s against kw
 * Return :
 * 	- remaining string if the keyword matches
 * 	- NULL if the keyword is not found
 */
	size_t klen = strlen(kw);
	if( strncasecmp(s,kw,klen) )
		return NULL;
	else
		return s+klen;
}

	/***
	 * Let's go ...
	 ***/

void read_configuration( const char *fch){
	char l[MAXLINE];
	FILE *f;
	char *arg;

	if(verbose)
		printf("\nReading configuration file '%s'\n---------------------------\n", fch);

	if(!(f=fopen(fch, "r"))){
		perror( fch );
		exit(EXIT_FAILURE);
	}

	while(fgets(l, MAXLINE, f)){
		if(*l == '#' || *l == '\n')
			continue;

		if((arg = striKWcmp(l,"token="))){
			assert( token = strdup( removeLF(arg) ) );
			if(verbose)
				printf("API token : '%s'\n", token);
		} else if((arg = striKWcmp(l,"senderID="))){
			assert( senderID = strdup( removeLF(arg) ) );
			if(verbose)
				printf("senderID : '%s'\n", senderID);		}
	}

	if( !token || !senderID ){
		fputs("Missing a mandatory parameter in the configuration file\n", stderr);
		exit(EXIT_FAILURE);
	}
}

int main( int ac, char ** av){
	const char *conf_file = DEFAULT_CONFIGURATION_FILE;

	int c;
	while((c = getopt(ac, av, "vhf:")) != EOF) switch(c){
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
	case 'v':
		verbose = true;
		break;
	case 'f':
		conf_file = optarg;
		break;
	default:
		fprintf(stderr, "Unknown option\n%s -h\n\tfor some help\n", av[0] );
		exit(EXIT_FAILURE);
	}
	read_configuration( conf_file );

	exit(EXIT_SUCCESS);
}
