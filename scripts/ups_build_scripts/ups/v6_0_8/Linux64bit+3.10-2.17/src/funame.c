/* @(#) funame.c 1.7 Delta: 96/05/30 08:56:19 Extraction: 98/01/05 13:49:47 @(#) */
/* funame - Fermi version of uname "system call". This library routine is
	very similiar to the uname system call, but enforces Fermi specific
	requirements. Originally funame was only shell callable. This code
	is a modification of that program.
 

	Matt Wicks - 4/25/91
*/

/*
	Modification History -
**
** 21-May-96	CJD	Fixed the AIX release value to look like 
**			the other flavors.
**
** 30-May-96	CJD	Added support for HP-UX.  HP-UX uname -r
**			returns a string composed of Machine-code,
**			release-number, and version-number.  Funame
**			for HP-UX will return
**			
**			release-number.version-number
**
**			like the "other" systems.
**
**  25-Sep-2000  MWM	use upsuname.c routines...
*/
		
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifdef USE_GETOPT_H
#include <getopt.h>
#else
#include <stdlib.h>
#endif
#include "upsuname.h"

#ifdef VMS

struct utsname {
	char	sysname[32];
	char	nodename[32];
	char	release[32];
	char	version[32];
	char	machine[32];
};

#define MAXHOSTNAMELEN 64

#else	/* VMS */
#include <sys/utsname.h>
#include <sys/param.h>
#endif	/* VMS */

#define DOMAIN ".fnal.gov"

int
funame_f(baseuname,hostname,ipaddress)
struct utsname *baseuname;
char *hostname;
char *ipaddress;

{
	int c,i;
	int length;
	struct hostent *hostentry;
	char *tpoint;
	char thostname[MAXHOSTNAMELEN];


		if ( (c = uname(baseuname)) == -1)
			return(-1);

                baseuname->sysname[0] = 0;
                ups_append_OS(baseuname->sysname);
                baseuname->release[0] = 0;
		ups_append_release(baseuname->release);

		if( (strncmp(baseuname->sysname, "AIX", 3) == 0 )) {
			baseuname->version[1] = '\0';
		}

		tpoint = strchr(baseuname->nodename,'.');
		if ( tpoint != NULL)
			tpoint[0] = '\0';

		(void) strcpy(hostname,baseuname->nodename);
		tpoint = strchr(hostname,'.');
		if ( tpoint != NULL)
			tpoint[0] = '\0';
		(void) strcat(hostname,DOMAIN);
		(void) strcpy(thostname,baseuname->nodename);
		tpoint = strchr(thostname,'.');
		if ( tpoint != NULL)
			tpoint[0] = '\0';
		if ( (hostentry = gethostbyname(thostname) ) == (struct hostent *) 0)
		{	/* didn't find match with short name, try to
			   find it with fully qualified name*/
			(void) strcat(thostname,DOMAIN);
			hostentry = gethostbyname(thostname);
		}
		ipaddress[0] = 0;
		if ( hostentry != (struct hostent *) 0 )
		{	/*Entry found*/
			for(i=0;i<hostentry->h_length;i++)
			{    length=strlen(ipaddress);
			     sprintf(&ipaddress[length],"%d.",
			        ((short)hostentry->h_addr_list[0][i] & 0xff));
			}
			length=strlen(ipaddress);
			ipaddress[length-1] = 0;
		}
return(0);
}

#ifdef VMS
/* uname for VMS so that funame works on a VAX */
/* uses LIB$GETSYI Programming vol 5B which calls*/
/*      SYS$GETSYI Programming vol 4B */
/* jms 2-jan-91 */

#include <ssdef.h>
#include <descrip.h>
#include <syidef.h>

int uname(name)

struct utsname *name;

{
  int			status;		/* return status */
  long			code;		/* for syidef */
  unsigned long		value;		/* from syidef */
  char			string[32];	/* for dsc_str */
  $DESCRIPTOR(dsc_str, string);		/* for syidef */
  unsigned short	length;		/* from syidef */
  unsigned long		csid;		/* for syidef */


  /* set the cluster system id */
  csid = 0;

  /* first set the system name */
  strcpy(name->sysname,"VMS");
  
  /* get the nodename */
  code = SYI$_NODENAME;
  dsc_str.dsc$w_length  = 15;
  status = lib$getsyi(&code,&value,&dsc_str,&length,&csid);
  if(status == SS$_NORMAL){
     strncpy(name->nodename,string,length);
     }
  else{
     fprintf(stderr," uname: unable to get nodename on VAX %d \n",status);
     return status;
     }

  /* get the release */
  code = SYI$_VERSION;
  dsc_str.dsc$w_length  = 8;
  status = lib$getsyi(&code,&value,&dsc_str,&length,&csid);
  if(status == SS$_NORMAL){
     strncpy(name->release,string,length);
     }
  else{
     fprintf(stderr," uname: unable to get release on VAX %d \n",status);
     return status;
     }

  /* get the version */
  code = SYI$_NODE_SWVERS;
  dsc_str.dsc$w_length  = 4;
  status = lib$getsyi(&code,&value,&dsc_str,&length,&csid);
  if(status == SS$_NORMAL){
     strncpy(name->version,string,length);
     }
  else{
     fprintf(stderr," uname: unable to get release on VAX %d \n",status);
     return status;
     }

  /* get the machine */
  code = SYI$_HW_NAME;
  dsc_str.dsc$w_length  = 31;
  status = lib$getsyi(&code,&value,&dsc_str,&length,&csid);
  if(status == SS$_NORMAL){
     strncpy(name->machine,string,length);
     }
  else{
     fprintf(stderr," uname: unable to get machine on VAX %d \n",status);
     return status;
     }

  return 0;

}

/* Simulate (hopefully) the UNIX getopt() routine */

/* NOTE: The code is not elegant, but it is READABLE! */

#include <stdio.h>

int optind = 1;
int opterr = 1;

char *optarg = 0;

int getopt(argc, argv, optstring)
int argc;
char *argv[];
char optstring[];
{
    int i;
    char option;

    if (optind >= argc)
	return (-1);

    /* Clear the return parameter */
    optarg = NULL;

    /* Check for an option present, i.e. starts with a hypen */
    if (argv[optind][0] != '-')
	return (-1);

    /* Pick up first letter of option (rest of option is ignored) */
    option = argv[optind][1];

    /* Double hypen means end of options */
    if (option == '-')
    {
	optind++;
	return (-1);
    }

    /* Search for a matching option */
    for (i=0; optstring[i] != 0; i++)
    {
	if (option == optstring[i])
	    break;
    }

    /* No valid option found */
    if (optstring[i] == 0)
    {
	if (opterr)
	    fprintf(stderr, "%s: Illegal option `%s'\r\n",
		argv[0], argv[optind]);
	option = '?';
    }

    /* Option is OK - Check for a parameter */
    else
    {
	/* Parameter required? */
	if (optstring[i+1] == ':')
	{
	    if ( (optind+1 < argc) && (argv[optind+1][0] != '-') )
	    {
		optind++;
		optarg = argv[optind];
	    }
	    else
	    {
		if (opterr)
		    fprintf(stderr,"%s: Argument required for `%s'\r\n",
			argv[0], argv[optind]);
	    }
	}
    }

    /* Bump counter to next argument */
    optind++;

    /* Return the letter we found */
    return (option);

}

#endif	/* VMS */

/* @(#) funame.c 1.10 Delta: 94/01/17 14:09:03 Extraction: 97/05/13 12:37:07 @(#) */
/* funame - Fermi version of uname command. This command is very similiar
	to the standard UNIX uname command but enforces specific Fermi
	standards

	Matt Wicks - 12/27/90
*/

/*
	Modification History -
	  1/3/91 - Jonathon Streets added code to make it work under VMS
	  1/8/91 - Matt Wicks added additional machines types for SGIs
	  1/11/91 - Matt Wicks added -i option to print out IP address
	  4/26/91 - Changed to use funame library call, default action is
	   now node name - Matt Wicks
	  4/26/91 - Changed exit value to -1 in case of error - Matt Wicks
	  4/29/91 - Modified length of ipaddress array - Matt Wicks
*/
		
#include <errno.h>

#define MAXIPLEN 16

int
main(argc,argv)
int argc;
char ** argv;

{
	struct utsname name;
	int c;
	extern char *optarg;
	extern int optind;
	short sflag = 0;
	short nflag = 0;
	short rflag = 0;
	short vflag = 0;
	short mflag = 0;
	short hflag = 0;
	short iflag = 0;
	short errflag = 0;
	short space = 0;
	extern int errno;
	char hostname[MAXHOSTNAMELEN];
	char ipaddress[MAXIPLEN+1];

	if ( argc <= 1)
		nflag++;
	
	while (( c = getopt(argc,argv,"snrvmahi")) != -1)
		switch (c)
		{	case 's':
				sflag++;
			break;
			case 'n':
				nflag++;
			break;
			case 'r':
				rflag++;
			break;
			case 'v':
				vflag++;
			break;
			case 'm':
				mflag++;
			break;
			case 'a':
				sflag++;
				nflag++;
				rflag++;
				vflag++;
				mflag++;
			break;
			case 'h':
				hflag++;
			break;
			case 'i':
				iflag++;
			break;
			case '?':
				errflag++;
			break;
		}
		if (errflag)
		{	(void) fprintf(stderr,"%s: usage: funame [-snrvmahi]\n",
			argv[0]);
			exit(-1);
		}

		if ( (c = funame_f(&name,hostname,ipaddress)) == -1)
		{	perror("funame");
			exit(-1);
		}

		if (sflag)
		{
			printf("%s",name.sysname);
			space=1;
		}
		if (nflag)
		{
			if (space)
				printf(" ");
			printf("%s",name.nodename);
			space=1;
		}
		if (rflag)
		{	if (space)
				printf(" ");
			printf("%s",name.release);
		}
		if (vflag)
		{	if (space)
				printf(" ");
			printf("%s",name.version);
			space=1;
		}
		if (mflag)
		{	if (space)
				printf(" ");
			printf("%s",name.machine);
			space=1;
		}
		if (hflag)
		{	if (space)
				printf(" ");
			printf("%s",hostname);
			space = 1;
		}
		if (iflag)
		{
			if ( ipaddress != NULL)
			{
				if (space)
					printf(" ");
				space = 1;
				printf("%s",ipaddress);
			}
		}
		printf("\n");

				

exit(0);
}
