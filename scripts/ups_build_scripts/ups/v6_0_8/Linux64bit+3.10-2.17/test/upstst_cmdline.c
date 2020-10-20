/*****************************************************************************
**
** ABSTRACT:
**	This file contains routines used to support command line entry,
**      and editing.  
**
**	AUTHOR'S DISCLAIMER: Hey, look out !!! This is admittedly a hastily
**	written, non-generic line editor.  It was written to suit our immediate
**	needs at the time (for another project) and no effort was made to
**	support the variety of terminals available.  It is hoped in the future
**	we can do something to remedy the situation and remove this disclaimer.
**	Until then, beware of this software.
**
**   ENTRY POINT	SCOPE	DESCRIPTION
**   -------------------------------------------------------------------------
**   upstst_linestart	public	Initialize, setup to read command line and 
**				print prompt.
**   upstst_cmdreset	public	Restore original terminal characteristics.
**   upstst_getchar	public	Get next character from command line.
**   upstst_procchar	public	Process single character input from command
**				line.
**
**   upstst_cmdint	local	Signal handler called for Ctrl C interrupts.
**   upstst_cmdtstp	local	Signal handler called for Ctrl Z interrupts. 
**   upstst_cmdcont	local	Signal handler called for continuation.
**   upstst_interrupt_dec	public	Declare interrupt handler.
**   upstst_interrupt_chk	public	Check if interrupt occurred.
**
** ENVIRONMENT: ANSI C.
**
** AUTHOR:      Creation date: Aug. 17, 1992
**              Gary Sergey
**
******************************************************************************
******************************************************************************
*/
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#ifdef __MACH__
#include <term.h>
#include <termios.h>
#else
#include <termio.h>
#endif
#include <signal.h>     /* Needed for signal function */
#include <sys/types.h>

#include "upstst_cmdline.h"

extern int	upstst_istty;


/*============================================================================
**============================================================================
**
** LOCAL PROTOTYPES, DEFINITIONS, MACROS, ETC.
** (NOTE: NONE OF THESE WILL BE VISIBLE OUTSIDE THIS MODULE)
**
**============================================================================
*/

/*---------------------------------------------------------------------
**
** LOCAL FUNCTION PROTOTYPES
*/
static void upstst_cmdint(void);
static void upstst_cmdtstp(void);

/*---------------------------------------------------------------------
**
** LOCAL DEFINITIONS, MACROS
*/

/*
** LOCALLY GLOBAL LINE EDITOR "ORIGINAL" TERMINAL CHARACTERISTICS
*/
#ifdef __TERMIOS_H__
static	struct termios	g_ed_savearg;
#else
static	struct termio	g_ed_savearg;
#endif

/*
** LOCALLY GLOBAL INTERRUPT SIGNAL FLAGS
*/
static	pid_t		g_exit_pid;
static  int		g_exit_flag=0;
static	int		g_sigint_flag=0;
static	int		g_sigint_flag_max=0;
struct sigaction	g_sigint_old;
struct sigaction	g_sigtstp_old;
struct sigaction	g_sigcont_old;
struct sigaction	g_sigterm_old;

/*---------------------------------------------------------------------
**
** PRINTLINE, CLRTOEOL : FTCLups_cmd_t_ LOCAL MACRO DEFINITONS
*/
#define PRINTLINE fputs(a_hl->line, stdout)
#define CLRTOEOL \
   { \
   int i; \
   fputc('\r',stdout); \
   for(i=0; i < a_hl->lndx; i++) {fputc(' ', stdout);} \
   }

#define MOVECRSR \
   { \
   int i; \
   if (a_hl->cndx < a_hl->lndx) \
      { \
      for (i = 0; i < (a_hl->lndx - a_hl->cndx); i++) {fputc(L_BS, stdout);} \
      } \
   }


/*---------------------------------------------------------------------
**
** ED_ON : MACRO TO SET TERMINAL CHARACTERISTICS
*/
#ifdef __TERMIOS_H__

#define ED_ON(a_savearg) 			\
   { 						\
   struct termios ed_arg; 			\
   tcgetattr(1, a_savearg); 	\
   tcgetattr(1, &ed_arg); 	\
   ed_arg.c_lflag &= (unsigned long) ~(ICANON | ECHO); 		\
   ed_arg.c_iflag &= (unsigned long) ~(INLCR | ICRNL); 		\
   ed_arg.c_cc[VMIN] = 1;			\
   ed_arg.c_cc[VTIME] = 0;			\
   tcsetattr(1,TCSANOW,&ed_arg); 		\
   }

#else

#define ED_ON(a_savearg) 			\
   { 						\
   struct termio ed_arg; 			\
   ioctl(1, TCGETA, (char *)(a_savearg)); 	\
   ed_arg = *(a_savearg); 			\
   ed_arg.c_lflag &= (unsigned long) ~(ICANON | ECHO); 		\
   ed_arg.c_iflag &= (unsigned long) ~(INLCR | ICRNL); 		\
   ed_arg.c_cc[VMIN] = 1;			\
   ed_arg.c_cc[VTIME] = 0;			\
   ioctl(1, TCSETA, (char*)&ed_arg); 		\
   }

#endif

/*---------------------------------------------------------------------

**
** ED_OFF : MACRO TO RESET TERMINAL CHARACTERISTICS
*/
#ifdef __TERMIOS_H__
#define ED_OFF(a_savearg) \
   { \
   tcsetattr(1, TCSANOW, a_savearg); \
   }

#else

#define ED_OFF(a_savearg) \
   { \
   ioctl(1, TCSETA, (char*)(a_savearg)); \
   }

#endif


static void upstst_exithandler(void);	/* forward declaration */


/*============================================================================  

ROUTINE: upstst_linestart

DESCRIPTION:
	Initialize the structure (handle) used by ftclCmd_ routines,
	setup to read characters from standard input, and print the line
	prompt to standard output.  This routine is to be used in conjunction
	with upstst_getchar and upstst_procchar, and must be called
	prior to line entry, for each line that is entered.

GLOBALS REFERENCED
	g_ed_savearg
	g_sigint_flag 

============================================================================ */
void upstst_linestart ( ups_cmd_edithndl_t * const a_hl, const char * const a_prompt)   
{

/* SETUP EXIT HANDLER
   ------------------ */

if (g_exit_flag == 0)
   {
   g_exit_flag++;
   g_exit_pid = getpid();
   atexit(upstst_exithandler);
   }

/* SETUP THE BEGINNING OF THE LINE
   ------------------------------- */

a_hl->line[0] = '\r';
strcpy(&a_hl->line[1], a_prompt);
strcpy(a_hl->saveline, a_hl->line);
strcpy(a_hl->savehdr, a_hl->line);
a_hl->firstcol = (int) strlen(a_prompt) + 1;

/* SET TERMINAL CHARACTERISTICS
  ----------------------------- */

ED_ON(&g_ed_savearg); 

/* RESET THE GLOBAL INTERRUPT FLAG
   ------------------------------- */

g_sigint_flag  = 0; /* This flag is incremented whenever an interrupt occurs */

/* SETUP TO READ CHARACTERS UNTIL A CARRIAGE RETURN IS HIT
   ------------------------------------------------------- */

a_hl->escmode = 0;  /* This flag is set when an escape character is entered */
a_hl->linemodified = 0;  /* This flag is set when a line has been modified */
a_hl->lndx = a_hl->firstcol;
a_hl->cndx = a_hl->firstcol;
a_hl->line[a_hl->firstcol] = (char)0;
a_hl->nextchar = 0;

/* PRINT PROMPT TO STANDARD OUTPUT
   -------------------------------- */

if (upstst_istty)
   PRINTLINE;
}


/*============================================================================  

ROUTINE: upstst_cmdreset

DESCRIPTION:
	Restore the original terminal characteristics.

GLOBALS REFERENCED
	g_ed_savearg

============================================================================ */
void upstst_cmdreset (void)   
{
/* Reset terminal characteristics */
ED_OFF(&g_ed_savearg);
}

/*============================================================================  

ROUTINE: upstst_getchar

DESCRIPTION:
	Get next character from command line (stdin).  The returned value
	is for information purposes only - the caller may ignore it.

RETURN VALUES:
	An integer value representing the character just obtained is returned.
	Note that this value is for information purposes only - the caller may
	ignore it.  If an error occurred, then the subsequent call to 
	upstst_procchar will reflect the error, so the user need only check
	the return status from upstst_procchar.

============================================================================ */
int upstst_getchar (ups_cmd_edithndl_t  * const a_hl)   
{
errno = 0;
a_hl->nextchar = getchar();

if (a_hl->nextchar == -1)
   {
   /* If there was an interrupted system call, then try one more time.  We
      do this since when we resume from a Ctrl Z (stopping) interrupt, an
      interrupted system call error is expected. */
   if (errno == EINTR)
      {a_hl->nextchar = getchar();}

   /* Clear the EOF indicator.  This is required because under SunOS, getchar
      will always return an EOF on subsequent calls once an actual EOF 
      occurs (until it is cleared). */
   clearerr(stdin);
   }
return(a_hl->nextchar);
}



/*============================================================================  

ROUTINE: upstst_procchar

DESCRIPTION
      Processes a single character input from the command line (stdin).
	This routine must be preceded by a call to upstst_linestart (to
	initialize) and upstst_getchar (to get the next character
	from the command line).  upstst_getchar and this routine
	should be called repeatedly until command line entry is complete
	(return value = 1), at which time the output parameter a_line is
	set.  Once line entry is complete, upstst_linestart must be invoked
	again, if additional lines are expected.
      There are lots of things that we need to do if stdin is a tty for
	command line editing. These thing don't need to be done if stdin
	is a file.

RETURN VALUES:
	 0 - Character entry processed successfully.
	 1 - Command line entry is complete.
	-1 - End-of-file detected (all processing is complete).

============================================================================ */
int upstst_procchar (ups_cmd_edithndl_t * const a_hl,char * const a_line)   
{
ups_cmd_t 	tmpline;
int 		rstatus, i, foundSpace, foundChar;
int 		entry, entry3;

char escseq[5];
escseq[0] = L_ESC;
escseq[1] = '[';
escseq[2] = 'C';
escseq[3] = '\0';



/*---------------------------------------------------------------------
**
** PROCESS CHARACTER 
*/
rstatus = 0;		/* Default return status */
entry = a_hl->nextchar;	/* Get local copy of entered character */

/*------------------------------------------------------------------------
**
** IF ESCAPE SEQUENCE STARTED - CATCH THE WHOLE THING
*/
if (upstst_istty)
   {
   if (a_hl->escmode)
      {
      /* STILL IN ESCAPE MODE, MAKE SURE ENTIRE SEQUENCE IS COLLECTED
         ------------------------------------------------------------ */
      a_hl->escmode++;

      /* Second pass - make sure we got a '[' */
      if ((a_hl->escmode == 2) && (entry != (int)'['))
         {
         fputc(L_BELL, stdout);	/* Invalid sequence - yell at the user */
         a_hl->escmode = 0;	/* No longer in escape mode */
         fflush(stdout);
         return(rstatus);	/* Go fish */
         }
      /* Third pass - sequence complete, so continue */
      else if (a_hl->escmode == 3)
         {
         entry3 = entry;	/* Save last character of sequence */
         entry  = L_ESC;	/* Setup to process sequence */
         a_hl->escmode = 0;	/* No longer in escape mode - complete */
         }
      else
         {
         return(rstatus);	/* Vamoose */
         }
      }
   else if (entry == L_ESC)
      {
      /* 
      ** ESCAPE SEQUENCE DETECTED, PUT IN ESCAPE MODE, THEN RETURN
      */
      a_hl->escmode = 1;
      return(rstatus);		/* Scadoodle */
      }
   }

/* these are the characters that work for both stt and files
   ---------------------------------------------------------  */

switch(entry)
   {

   /*------------------------------------------------------------------
   **
   ** IF A CARRIAGE RETURN WAS HIT, THE LINE IS COMPLETE
   */
   case L_CR:
   case L_LF:

      if (upstst_istty)
         {
         PRINTLINE;
         fputc('\n', stdout);
         }
/* copy the line to the user's argument
   ------------------------------------ */

      ED_OFF(&g_ed_savearg);	
      strcpy(a_line, &a_hl->line[a_hl->firstcol]);
      return (1); 		/* line entry is complete */

   /*------------------------------------------------------------------
   **
   ** IF END OF FILE THEN GET OUT
   */
   case -1:
   case  0:

      ED_OFF(&g_ed_savearg);	
      return (-1);  		/* end-of-file was detected */

   default:

      break;
   }

/* if we made it this far, we still haven't processed the char yet.
   let's do things that are unique for ttys.
   ---------------------------------------------------------------- */

if (upstst_istty)
   {
   switch(entry)
      {
      /*------------------------------------------------------------------
      **
      ** IF AN ESCAPE SEQUENCE IS DETECTED - DEAL WITH IT
      */
      case L_ESC:
         switch(entry3)
            {

            /*
            ** RIGHT ARROW WAS PRESSED - MOVE CURSOR TO THE RIGHT
            */
            case L_RIGHT:
               if (a_hl->cndx < a_hl->lndx)
                  {
                  a_hl->cndx++;
                  fputs(escseq, stdout);
                  }
               return(rstatus);

            /*
            ** LEFT ARROW WAS PRESSED - MOVE CURSOR TO THE LEFT
            */
            case L_LEFT:
               if (a_hl->cndx > a_hl->firstcol)
                  {
                  a_hl->cndx--;
                  fputc(L_BS, stdout);
                  }
               return(rstatus);

            /*
            ** DEFAULT - UNKNOWN ESCAPE SEQUENCE
            */
            default:
               fputc(L_BELL, stdout);	/* Bitch at the user */
               return(rstatus);
            }  /* switch */

      /*------------------------------------------------------------------
      **
      ** IF A BACKSPACE WAS ENTERED, GO TO THE START OF THE LINE
      */
      case L_BS:
         a_hl->cndx = a_hl->firstcol;
         PRINTLINE;
         MOVECRSR;
         return(rstatus);

      /*------------------------------------------------------------------
      **
      ** IF A RUBOUT ('^U') WAS ENTERED, CLEAR ALL TO LEFT OF CURSOR
      */
      case L_RUB:
         CLRTOEOL;
         if (a_hl->cndx < a_hl->lndx)
            {
            strcpy(tmpline, &a_hl->line[a_hl->cndx]);
            strcpy(&a_hl->line[a_hl->firstcol], tmpline);
            a_hl->lndx = (int) strlen(tmpline) + (int) a_hl->firstcol;
            }
         else
            {
            a_hl->line[a_hl->firstcol] = (char)0;
            a_hl->lndx = a_hl->firstcol;
            }
         a_hl->cndx = a_hl->firstcol;
         PRINTLINE;
         MOVECRSR;
         return(rstatus);

      /*------------------------------------------------------------------
      **
      ** IF A '^F' WAS ENTERED, MOVE CURSOR TO NEXT WORD
      */
      case L_FWD:
         foundSpace = 0;
         for (i=a_hl->cndx; i<=a_hl->lndx; i++)
            {
            if (a_hl->line[i] == (char)' ')
               {
               foundSpace = 1;
               }
            else if (foundSpace)
               {
               a_hl->cndx = i;
               PRINTLINE;
               MOVECRSR;
               break; 
               }
            }
         return(rstatus);

      /*------------------------------------------------------------------
      **
      ** IF A '^B' WAS ENTERED, MOVE CURSOR TO PREVIOUS WORD
      */
      case L_BWD:
         foundChar = 0;
         for (i=a_hl->cndx-1; i>=a_hl->firstcol; i--)
            {
            if (a_hl->line[i] == (char)' ')
               {
               if (foundChar == 1)
                  {
                  a_hl->cndx = i+1;
                  PRINTLINE;
                  MOVECRSR;
                  break;
                  }
               }
            else if (i == a_hl->firstcol)
               {
               a_hl->cndx = i;
               PRINTLINE;
               MOVECRSR;
               break;
               }
            else
               {
               foundChar = 1;
               }
            }
         return(rstatus);

      /*------------------------------------------------------------------
      **
      ** IF '^E' WAS ENTERED, GO TO THE END OF THE LINE
      */
      case L_EOL:
         a_hl->cndx = a_hl->lndx;
         PRINTLINE;
         return(rstatus);

      /*------------------------------------------------------------------
      **
      ** IF '^R' WAS ENTERED, REDRAW THE LINE
      */
      case L_REDRAW:
         CLRTOEOL;
         PRINTLINE;
         return(rstatus);
   
      /*------------------------------------------------------------------
      **
      ** IF THE DELETE KEY WAS HIT, REMOVE LAST CHARACTER FROM LINE
      */
      case L_DEL:
         if (a_hl->cndx > a_hl->firstcol)
            {
            if (a_hl->cndx < a_hl->lndx)
               {
               strcpy(tmpline, &a_hl->line[a_hl->cndx]);
               strcpy(&a_hl->line[a_hl->cndx - 1], tmpline);
               }
            else
               {
               a_hl->line[a_hl->cndx - 1] = (char)0;
               }
            a_hl->cndx--;
            a_hl->lndx--;
            a_hl->linemodified = 1;
            CLRTOEOL;
            PRINTLINE;
            MOVECRSR;
            }
         return(rstatus);
      default:
         break;
      }
   }

/* we made it this far - do this for all input types
  -------------------------------------------------- */

/*------------------------------------------------------------------
**
** ADD PRINTABLE CHARACTERS TO THE LINE
*/

/* Only add printable characters, ignore anything else */
if (isprint((char)entry))
   {

   /* Make sure the line is not full */
   if ((a_hl->lndx + 1) < UPSTST_MAXLINE)
      {
      if (a_hl->cndx < a_hl->lndx)
         {
         /* We're not at the end of the line, so we need to copy */
         strcpy(tmpline, &a_hl->line[a_hl->cndx]);
         a_hl->line[a_hl->cndx] = (char)entry;
         a_hl->cndx++;
         strcpy(&a_hl->line[a_hl->cndx], tmpline);
         a_hl->lndx++;
         a_hl->line[a_hl->lndx]   = (char)0;	/* Null terminate */
         a_hl->linemodified = 1;
	 if (upstst_istty)
 	    {
            PRINTLINE;
            MOVECRSR;
   	    }
         }
      else
         {
         /* We're at the end of the line, so just add it */
         a_hl->line[a_hl->cndx] = (char)entry;
         a_hl->cndx++;
         a_hl->lndx++;
         a_hl->line[a_hl->lndx]   = (char)0;	/* Null terminate */
         a_hl->linemodified = 1;
         if (upstst_istty) fputc((int)entry, stdout);
         }
      }

   /* Uh oh, the line is full - scream at the user */
   else
      {
      if (upstst_istty) fputc(L_BELL, stdout);
      else fprintf (stderr, "line is full! \n");
      }
   }

return(rstatus);

} /* upstst_procchar */



/*============================================================================  

ROUTINE: upstst_cmdint

DESCRIPTION:
	Signal handler routine that is called to cleanup when an interrupt
	signal (e.g., Ctrl C) occurs.  This routine restores the original
	terminal characteristics and then sets a global flag (g_sigint_flag) 
	indicating the interrupt occurred.  This flag can be checked
	(via upstst_interrupt_chk) by the user to determine when an interrupt
	occurred.

GLOBALS REFERENCED
	g_ed_savearg
	g_sigint_flag 
	g_sigint_flag_max
	g_sigint_old

============================================================================ */
static void upstst_cmdint(void)
{
struct sigaction    act;

/*
** INCREMENT THE GLOBAL INTERRUPT FLAG
*/
g_sigint_flag++;

/*
** TELL THE USER AN INTERRUPT WAS DETECTED
*/
fputc(L_BELL, stdout);
fputs("\n-- INTERRUPT --\n", stdout);
if ((g_sigint_flag + 1) == g_sigint_flag_max)
   {fputs("-- Hit ^C again to exit program --\n", stdout);}

/*
** IF WE'VE EXCEEDED THE MAX NUMBER OF INTERRUPTS COUNT, THEN EXIT THE PROGRAM
*/
if (g_sigint_flag >= g_sigint_flag_max)
   {
   /*
   ** FIRST RESTORE THE ORIGINAL TERMINAL CHARACTERISTICS
   */
   ED_OFF(&g_ed_savearg);

   /*
   ** CALL THE OLD HANDLER, THEN RESTORE DEFAULT HANDLER AND RE-SEND INTERRUPT
   ** SO DEFAULT ACTION TAKES PLACE
   */
   if ((g_sigint_old.sa_handler != SIG_IGN) && 
       (g_sigint_old.sa_handler != SIG_DFL) &&
       (g_sigint_flag == g_sigint_flag_max))
      {
      g_sigint_old.sa_handler(0);
      }

   /* If we're still here, then restore the default handler so we exit */
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   act.sa_handler = (void (*)(int))SIG_DFL;
   sigaction(SIGINT, &act, NULL);
   raise(SIGINT);
   }
}



static void upstst_cmdcont(void);	/* forward declaration */
/*============================================================================  
**============================================================================
**
** ROUTINE: upstst_cmdtstp
*/
/*
** DESCRIPTION:
**	Signal handler routine that is called to cleanup when a Ctrl Z
**	interrupt signal (SIGTSTP) occurs.  This routine restores the original
**	terminal characteristics then re-sends the SIGTSTP signal so the 
**	default action will take place (i.e., the process will be stopped).
**	When the process is resumed, this routine will once again be declared
**	as the interupt handler.
**
**	This only reason this handler is required, is to restore the original
**	terminal characteristics.
**
** RETURN VALUES:
**	None.
**
** DEFINED AS SIGNAL HANDLER BY:
**	upstst_interrupt_dec
**	upstst_cmdcont
**
** GLOBALS REFERENCED
**	g_ed_savearg
**	g_sigtstp_old
**	g_sigcont_old
**
**============================================================================
*/
static void upstst_cmdtstp(void)
{
struct sigaction    act;

/*
** RESTORE THE ORIGINAL TERMINAL CHARACTERISTICS
*/
ED_OFF(&g_ed_savearg);

/*
** DECLARE OUR SIGCONT SIGNAL HANDLER
*/
sigemptyset(&act.sa_mask);
act.sa_flags = 0;
act.sa_handler = (void (*)(int))upstst_cmdcont;
sigaction(SIGCONT, &act, &g_sigcont_old);   /* Continue from CTRL Z interrupt */

/*
** RESTORE OLD HANDLER, THEN RE-SEND INTERRUPT SO DEFAULT ACTION TAKES PLACE
*/
sigaction(SIGTSTP, &g_sigtstp_old, NULL);
raise(SIGTSTP);
}



/*============================================================================  
**============================================================================
**
** ROUTINE: upstst_cmdcont
*/
static void upstst_cmdcont(void)
/*
** DESCRIPTION:
**	Signal handler routine that is called once the process is resumed from
**	a Ctrl Z interrupt signal (SIGTSTP).  This routine simply redeclares
**	upstst_cmdtstp as the Ctrl Z interrupt handler, sets the terminal
**	characteristics back to command line input, then re-issues the SIGCONT
**	signal with the previous handler.
**
** RETURN VALUES:
**	None.
**
** DEFINED AS SIGNAL HANDLER BY:
**	upstst_cmdtstp
**
** GLOBALS REFERENCED
**	g_ed_savearg
**	g_sigcont_old
**	g_sigtstp_old
**
**============================================================================
*/
{
#ifdef __TERMIOS_H__
struct termios	    l_ed_savearg;
#else
struct termio	    l_ed_savearg;
#endif
struct sigaction    act;

/*
** REDECLARE THE SIGTSTP SIGNAL HANDLER
*/
sigemptyset(&act.sa_mask);
act.sa_flags = 0;
act.sa_handler = (void (*)(int))upstst_cmdtstp;
sigaction(SIGTSTP, &act, &g_sigtstp_old);    /* CTRL Z interrupt */

/*
** SET TERMINAL CHARACTERISTICS FOR COMMAND LINE EDITING
*/
l_ed_savearg = g_ed_savearg;
ED_ON(&l_ed_savearg);


/*
** RESTORE OLD HANDLER, THEN RE-SEND INTERRUPT SO DEFAULT ACTION TAKES PLACE
*/
sigaction(SIGCONT, &g_sigcont_old, NULL);
raise(SIGCONT);
}



/*============================================================================  
**============================================================================
**
** ROUTINE: upstst_cmdterm
*/
/*
** DESCRIPTION:
**	Signal handler routine that is called when a kill command is issued.
**	This routine simply resets the terminal characteristics, then re-issues
**	the SIGTERM signal with the previous handler.
**
** RETURN VALUES:
**	None.
**
** DEFINED AS SIGNAL HANDLER BY:
**	upstst_interrupt_dec
**
** GLOBALS REFERENCED
**	g_ed_savearg
**	g_sigterm_old
**
**============================================================================
*/
static void upstst_cmdterm(void)
{
/*
** RESTORE THE ORIGINAL TERMINAL CHARACTERISTICS
*/
ED_OFF(&g_ed_savearg);

/*
** RESTORE OLD HANDLER, THEN RE-SEND INTERRUPT SO DEFAULT ACTION TAKES PLACE
*/
sigaction(SIGTERM, &g_sigterm_old, NULL);
raise(SIGTERM);
}



/*============================================================================  
**============================================================================

ROUTINE: upstst_interrupt_dec

DESCRIPTION:
	Declare the upstst_cmdint routine as a signal handler and set the
	number of successive interrupts allowed before we exit the program.

GLOBALS REFERENCED
	g_ed_savearg
	g_sigint_flag
	g_sigint_flag_max
	g_sigint_old
	g_sigtstp_old
	g_sigcont_old
	g_sigterm_old

============================================================================ */
void upstst_interrupt_dec (int a_cnt)   
{
struct sigaction act, curHndlr;

/* Initialize global variables just in case a CTRL C is hit before any of the
   ftclCmd_ routines are called */
memset(&g_ed_savearg,0, sizeof(g_ed_savearg));
g_sigint_flag = 0;

if (a_cnt <= 0) {a_cnt = 1;}
g_sigint_flag_max = a_cnt;

/* Set SIGINT handler (if we're not already the handler) */
sigaction(SIGINT, NULL, &curHndlr);		/* Get current handler */
if (curHndlr.sa_handler != (void (*)(int))upstst_cmdint)
   {
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   act.sa_handler = (void (*)(int))upstst_cmdint;
   sigaction(SIGINT, &act, &g_sigint_old);	/* CTRL C interrupt */
   }

/* Set SIGTSTP handler (if we're not already the handler) */
sigaction(SIGTSTP, NULL, &curHndlr);		/* Get current handler */
if (curHndlr.sa_handler != (void (*)(int))upstst_cmdtstp)
   {
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   act.sa_handler = (void (*)(int))upstst_cmdtstp;
   sigaction(SIGTSTP, &act, &g_sigtstp_old);   /* CTRL Z interrupt */
   }

/* Set SIGTERM handler (if we're not already the handler) */
sigaction(SIGTERM, NULL, &curHndlr);		/* Get current handler */
if (curHndlr.sa_handler != (void (*)(int))upstst_cmdterm)
   {
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   act.sa_handler = (void (*)(int))upstst_cmdterm;
   sigaction(SIGTERM, &act, &g_sigterm_old);   /* KILL */
   }
}



/*============================================================================  

ROUTINE: upstst_interrupt_chk

DESCRIPTION:
	Check if a interrupt signal (i.e., Ctrl C) occurred since the last
	call to upstst_linestart.

RETURN VALUES:
	0 - No interrupt occurred.
	1 - Yes, an interrupt occurred.

GLOBALS REFERENCED
	g_sigint_flag 

============================================================================ */
int upstst_interrupt_chk (void)   
{
/* If the global SIGINT flag is non-zero, then an interrupt occurred */
if (g_sigint_flag)
   {return(1);}
else
   {return(0);}
}



/*============================================================================  

 ROUTINE: upstst_exithandler

DESCRIPTION:
	This routine is used as the exit handler for the command line
	editing functions of FTCL.  It is needed to restore the original
	terminal characteristics.

	If we are the process that declared the exit handler, then 
	go ahead and restore the terminal characteristics.  We have to
	be careful since the user may have forked and inherited this 
	exit handler (we don't want to be restoring terminal characteristics
	for all child processes).

DEFINED AS EXIT HANDLER BY:
	upstst_linestart

GLOBALS REFERENCED
	g_exit_pid

**============================================================================
*/
static void upstst_exithandler (void)   
{
if (g_exit_pid == getpid())
   {upstst_cmdreset();}
}

