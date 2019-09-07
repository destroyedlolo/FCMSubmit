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
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <json-c/json.h>

// #include <openssl/sha.h>
// #include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

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
const char *title = NULL;
const char *message = NULL;
const char *source = NULL;
short int priority = 0;
short int speak = -1;
short int notify = -1;


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
	 * Configuration & parameters
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
		fputs("*F* Missing a mandatory parameter in the configuration file\n", stderr);
		exit(EXIT_FAILURE);
	}
}

bool checkYesNo( const char *arg, short int *val ){
/* Check if the argument is "yes" or "no"
 * <- val : pointer to the result 0:no, 1:yes
 * -> is valid : false if not "yes" or "no"
 */
	if(!strcasecmp(arg, "yes")){
		*val = 1;
		return true;
	} else if(!strcasecmp(arg, "no")){
		*val = 0;
		return true;
	} else /* incorrect argument */
		return false;
}

bool checkState( const char *arg, short int *val ){
/* Check if the argument is "sticky" or "locked"
 * <- val : pointer to the result 1:sticky, 1:locked
 * -> is valid : false if not "sticky" or "no"
 */
	if(!strcasecmp(arg, "sticky")){
		*val = 1;
		return true;
	} else if(!strcasecmp(arg, "locked")){
		*val = 2;
		return true;
	} else /* incorrect argument */
		return false;
}


	/***
	 * Json generation
	 ***/

char *base64Encode(const unsigned char *input, size_t length){
/* encode chunk of memory to base64
 * (from https://stackoverflow.com/questions/22861325/base64-encoding-with-c-and-openssl)
 *
 * -> input, lenght : chunck to encode
 * <- malloc()ed base64 encoding / NULL in case of error
 */
	BIO *bmem = BIO_new(BIO_s_mem()), *b64 = BIO_new(BIO_f_base64());
	BUF_MEM *bptr;

	if(!bmem || !b64)
		return NULL;

	b64 = BIO_push(b64, bmem);
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	char *buff = (char *)malloc(bptr->length+1);
	memcpy(buff, bptr->data, bptr->length);
	buff[bptr->length] = 0;

	BIO_free_all(b64);

	return buff;
}

char *base64EncodeString( const char *msg ){
	if(!msg)
		return NULL;
	return base64Encode( (const unsigned char *)msg, strlen(msg) );
}

void generateFCM(
	const char *token, const char *senderID,
	const char *title, const char *msg,
	short int priority
){
/* generate an FCM payload. */
	char *t;

	char buf[sizeof "AAAA-MM-DDTHH:MM:SSZ"+1];
	time_t now;
	time(&now);
	strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));

	json_object *jobj = json_object_new_object();
	json_object *sub = json_object_new_object();
	assert( jobj && sub );

	assert( !json_object_object_add( jobj, "registration_ids", json_object_new_string(senderID) ) );

	assert( !json_object_object_add( sub, "type", json_object_new_string("ntp_message") ) );
	assert( !json_object_object_add( sub, "timestamp", json_object_new_string(buf) ) );

		/* IMPORTANT NOTE : as the time of writing, data are copied to newly allocated
		 * space so data can be freed just afterward.
		 */
	assert( !json_object_object_add( sub, "priority", json_object_new_int( priority )) );
	assert( !json_object_object_add( sub, "title", json_object_new_string( t = base64EncodeString(title) )) );
	free(t);
	if(msg){
		assert( !json_object_object_add( sub, "message", json_object_new_string( t = base64EncodeString(msg) )) );
		free(t);
	}

	json_object_object_add( jobj, "data", sub );
	printf("=> '%s'\n", json_object_to_json_string(jobj) );

	json_object_put(jobj);
}


	/***
	 * Let's go ...
	 ***/

int main( int ac, char ** av){
	const char *conf_file = DEFAULT_CONFIGURATION_FILE;

	int c;
	while((c = getopt(ac, av, "vhf:t:m:s:p:k:n:a:")) != EOF) switch(c){
	case 'h':
		fprintf(stderr, "%s (%s)\n"
			"Submit Firebase Cloud Messaging notification\n"
			"Known options are :\n"
			"\t-h : this online help\n"
			"\t-v : enable verbose messages\n"
			"\t-f<file> : read <file> for configuration\n"
			"\t\t(default is '%s')\n"
			"\nStandards parameters (mandatory) :\n"
			"\t-t<title> : title to be send\n"
			"\t-m<message> : message to be send\n"
			"\nNewtifryPro specifics (optional)\n"
			"\t-s<source_name>\n"
			"\t-p<num> : priority (-512 to 3)\n"
			"\t-k<Yes|No> : force speaking\n"
			"\t-n<Yes|No> : force notification\n"
			"\t-a<sticky|locked> : behaviour of the message (on top / can't be removed)\n",
			basename(av[0]), VERSION, DEFAULT_CONFIGURATION_FILE
		);
		exit(EXIT_FAILURE);
		break;
	case 'v':
		verbose = true;
		printf("%s (%s)\n-----------------\n", basename(av[0]), VERSION);
		break;
	case 'f':
		assert( conf_file = strdup(optarg) );
		break;
	case 't':
		assert( title = strdup(optarg) );
		if(verbose)
			printf("Title : \"%s\"\n", title);
		break;
	case 'm':
		assert( message = strdup(optarg) );
		if(verbose)
			printf("Message : \"%s\"\n", message);
		break;
	case 's':
		assert( source = strdup(optarg) );
		if(verbose)
			printf("Source : \"%s\"\n", source);
		break;
	case 'p':
		priority = atoi(optarg);
		if(verbose)
			printf("Priority : %d\n", priority);
		break;
	case 'k':
		if(!checkYesNo(optarg,&speak)){
			fputs("*F* Speak accept only \"Yes\" and \"No\"\n", stderr);
			exit(EXIT_FAILURE);
		}
		if(verbose)
			printf("Speak : %s\n", (speak==1) ? "Yes":"No");
		break;
	case 'n':
		if(!checkYesNo(optarg,&notify)){
			fputs("*F* Notify accept only \"Yes\" and \"No\"\n", stderr);
			exit(EXIT_FAILURE);
		}
		if(verbose)
			printf("Notify : %s\n", (notify==1) ? "Yes":"No");
		break;
	case 'a':
		if(!checkState(optarg,&notify)){
			fputs("*F* State accept only \"sticky\" and \"locked\"\n", stderr);
			exit(EXIT_FAILURE);
		}
		if(verbose)
			printf("State : %s\n", (notify==1) ? "Sticky":"Locked");
		break;
	default:
		fprintf(stderr, "Unknown option\n%s -h\n\tfor some help\n", av[0] );
		exit(EXIT_FAILURE);
	}

	if(!title){
		fputs("*F* Title missing\n", stderr);
		exit(EXIT_FAILURE);
	}

	read_configuration( conf_file );

	generateFCM( token, senderID, title, message, priority );

	exit(EXIT_SUCCESS);
}
