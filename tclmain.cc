/*
  @copyright Russell Standish 2000
  @author Russell Standish
  This file is a modified version of a similarly named file in EcoLab.

  It is released as public domain.
*/
#include "ecolab.h"
#include "timer.h"
#include "object.h"
#include "eco_hashmap.h"
#include "pack_stream.h"

#include "init.h"

#include <ecolab_epilogue.h>
#include <fstream>

#include <boost/filesystem.hpp>
using boost::filesystem::path;

extern "C"
{
  typedef void (*__sighandler_t) (int);
  extern __sighandler_t signal (int __sig, __sighandler_t __handler);
}

#ifndef SIG_DFL
#define SIG_DFL	((__sighandler_t) 0)		/* Default action.  */
#endif

#define	SIGILL		4	/* Illegal instruction (ANSI).  */

#ifndef SIGABRT
#define	SIGABRT		6	/* Abort (ANSI).  */
#endif

#define	SIGBUS		7	/* BUS error (4.2 BSD).  */
#define	SIGSEGV		11	/* Segmentation violation (ANSI).  */


using namespace std;
using namespace ecolab;
using namespace classdesc;
using namespace minsky;

#include "version.h"
NEWCMD(minsky_version,0)
{
  tclreturn r;
  r<<VERSION;
}

namespace ecolab
{
  Tk_Window mainWin=0;
}

void printTimersAtEnd()
{
  stop_timer("main"); 
  print_timers();
}

#if defined(_WIN32)
// for GetConsoleWindow()
#define _WIN32_WINNT 0x0501
#include <windows.h>
#endif

int main(int argc, char* argv[])
{
  Timer timer("main");
  //  atexit(printTimersAtEnd);

#if defined(NDEBUG) && defined(_WIN32)
  ShowWindow(GetConsoleWindow(),SW_HIDE);
#endif

  Tcl_FindExecutable(argv[0]);
#ifndef MXE
  // For MXE builds, override tcl_library and tk_library to the self-contained versions
  if (Tcl_Init(interp())!=TCL_OK)
#endif
    // one possible reason is that it failed to locate the TCL library, we we set tcl_library and try again
    {
      path exeDir=path(Tcl_GetNameOfExecutable()).parent_path();
      tclvar tcl_library("tcl_library", (exeDir/"library"/"tcl").string().c_str());
      tclvar tk_library("tk_library", (exeDir/"library"/"tk").string().c_str());
      if (Tcl_Init(interp())!=TCL_OK)
        {
          fprintf(stderr,"%s\n",Tcl_GetStringResult(interp()));
          fprintf(stderr,"%s\n",Tcl_GetVar(interp(),"errorInfo",0)); /* print out trace */
          return 1;  /* not a clean execution */
        }
    }

  /* set the TCL variables argc and argv to contain the
     arguments. */
  tclvar tcl_argc("argc"), tcl_argv("argv");
  tcl_argc=argc;
  for (int i=0; i<argc; i++) tcl_argv[i]=argv[i];

  path minskydir=path(Tcl_GetNameOfExecutable()).parent_path();
  // if Minsky's libraries are not located with the executable, look in the lib
  // directory (unix style installation)
  if (!exists(minskydir/"minsky.tcl"))
    minskydir/="../lib/minsky";
  if (!exists(minskydir/"minsky.tcl"))
    {
      fprintf(stderr,"Minsky library not found!\n");
      return 1;
    }
  tclvar minskyHome("minskyHome",minskydir.string().c_str());

  if (Tcl_EvalFile(interp(), (minskydir/"minsky.tcl").string().c_str())!=TCL_OK)
    {
      fprintf(stderr,"%s\n",Tcl_GetStringResult(interp()));
      fprintf(stderr,"%s\n",Tcl_GetVar(interp(),"errorInfo",0)); /* print out trace */
      return 1;  /* not a clean execution */
    }

  while (mainWin) /* we are running GUI mode */
    Tcl_DoOneEvent(0); /*Tk_MainLoop();*/
}

extern "C" int Tktable_Init(Tcl_Interp *interp);

// useful for debugging X11 errors
//int minskyXErrorHandler(Display *d, XErrorEvent* ev)
//{
//  if (ev->error_code==BadWindow) return 0;
//  char msg[1024];
//  XGetErrorText(d,ev->error_code,msg,1024);
//  puts(msg);
//  //  throw error("Xlib error %s",msg);
//}


NEWCMD(GUI,0)
{
  if (Tk_Init(interp())==TCL_ERROR || Tktable_Init(interp())==TCL_ERROR)
    {
      fprintf(stderr,"Error initialising Tk: %s",Tcl_GetStringResult(interp()));
      fprintf(stderr,"%s\n",Tcl_GetVar(interp(),"errorInfo",0));
      /* If Tk_Init fails, it is not necessarily a fatal error. For
         example, on unpatched macs, we get an error from the attempt
         to create a console, yet Minsky works just fine without
         one. */
      //      return 1;
    }

  // call initialisers
  vector<Fun>& init=initVec();
  for (vector<Fun>::iterator i=init.begin(); i!=init.end(); ++i)
    (*i)();

  mainWin = Tk_MainWindow(interp());
  //XSetErrorHandler(minskyXErrorHandler);
}

//this seems to be needed to get Tkinit to function correctly!!
NEWCMD(tcl_findLibrary,-1) {}

NEWCMD(exit_ecolab,0)
{
mainWin=0;
}


namespace TCLcmd
{

  namespace trap
  {
    eco_string sigcmd[32];
    void sighand(int s) {Tcl_Eval(interp(),sigcmd[s].c_str());}
    hash_map<eco_string,int> signum;   /* signal name to number table */
    struct init_t
    {
      init_t()
      {
        signum["HUP"]=	1;	/* Hangup (POSIX).  */
        signum["INT"]=	2;	/* Interrupt (ANSI).  */
        signum["QUIT"]=	3;	/* Quit (POSIX).  */
        signum["ILL"]=	4;	/* Illegal instruction (ANSI).  */
        signum["TRAP"]=	5;	/* Trace trap (POSIX).  */
        signum["ABRT"]=	6;	/* Abort (ANSI).  */
        signum["IOT"]=	6;	/* IOT trap (4.2 BSD).  */
        signum["BUS"]=	7;	/* BUS error (4.2 BSD).  */
        signum["FPE"]=	8;	/* Floating-point exception (ANSI).  */
        signum["KILL"]=	9;	/* Kill, unblockable (POSIX).  */
        signum["USR1"]=	10;	/* User-defined signal 1 (POSIX).  */
        signum["SEGV"]=	11;	/* Segmentation violation (ANSI).  */
        signum["USR2"]=	12;	/* User-defined signal 2 (POSIX).  */
        signum["PIPE"]=	13;	/* Broken pipe (POSIX).  */
        signum["ALRM"]=	14;	/* Alarm clock (POSIX).  */
        signum["TERM"]=	15;	/* Termination (ANSI).  */
        signum["STKFLT"]=	16;	/* Stack fault.  */
        signum["CLD"]=	17;     /* Same as SIGCHLD (System V).  */
        signum["CHLD"]=   17;	/* Child status has changed (POSIX).  */
        signum["CONT"]=	18;	/* Continue (POSIX).  */
        signum["STOP"]=	19;	/* Stop, unblockable (POSIX).  */
        signum["TSTP"]=	20;	/* Keyboard stop (POSIX).  */
        signum["TTIN"]=	21;	/* Background read from tty (POSIX).  */
        signum["TTOU"]=	22;	/* Background write to tty (POSIX).  */
        signum["URG"]=	23;	/* Urgent condition on socket (4.2 BSD).  */
        signum["XCPU"]=  	24;	/* CPU limit exceeded (4.2 BSD).  */
        signum["XFSZ"]=	25;	/* File size limit exceeded (4.2 BSD).  */
        signum["VTALRM"]= 26;	/* Virtual alarm clock (4.2 BSD).  */
        signum["PROF"]=	27;	/* Profiling alarm clock (4.2 BSD).  */
        signum["WINCH"]=  28;	/* Window size change (4.3 BSD, Sun).  */
        signum["POLL"]=	29;/* Pollable event occurred (System V).  */
        signum["IO"]=	29;	/* I/O now possible (4.2 BSD).  */
        signum["PWR"]=	30;	/* Power failure restart (System V).  */
        signum["SYS"]=	31;	/* Bad system call.  */
        signum["UNUSED"]=	31;
      }
    } init;

    NEWCMD(trap,2)     /* trap argv[2] to excute argv[1] */
    {
      int signo = (isdigit(argv[1][0]))? atoi(argv[1]):
	signum[const_cast<char*>(argv[1])];
      sigcmd[signo]=argv[2];
      signal(signo,sighand);
    }

    void aborthand(int s) {throw error("Fatal Error: Execution recovered");}

    NEWCMD(trapabort,-1)
    {
      void (*hand)(int);
      if (argc>1 && strcmp(argv[1],"off")==0) hand=SIG_DFL;
      else hand=aborthand;
      signal(SIGABRT,hand);
      signal(SIGSEGV,hand);
      signal(SIGBUS,hand);
      signal(SIGILL,hand);
    }
  }



#ifdef READLINE
  extern "C" char *readline(char *);
  extern "C" void add_history(char *);
#endif

  NEWCMD(cli,0)
  {
    int braces=0, brackets=0;
    bool inString=false;
    tclcmd cmd;
    cmd << "savebgerror\n";
#ifdef READLINE
    char *c;
    eco_string prompt((const char*)tclvar("argv(0)")); prompt+='>';
    while ( (c=readline(const_cast<char*>(prompt.c_str())))!=NULL && strcmp(c,"exit")!=0)
#else
      char c[512];
    while (fgets(c,512,stdin)!=NULL && strcmp(c,"exit\n")!=0)
#endif
      {
        // count up number of braces, brackets etc
        for (char* cc=c; *cc!='\0'; ++cc)
          switch(*cc)
            {
            case '{':
              if (!inString) braces++;
              break;
            case '[':
              if (!inString) brackets++;
              break;
            case '}':
              if (!inString) braces--;
              break;
            case ']':
              if (!inString) brackets--;
              break;
            case '\\':
              if (inString) cc++; //skip next character (is literal)
              break;
            case '"':
              inString = !inString;
              break;
            }
        cmd << chomp(c);
        if (!inString && braces<=0 && brackets <=0)
          { // we have a complete command, so attempt to execute it
            try
              {
                cmd.exec();
              }
            catch (std::exception& e) {fputs(e.what(),stderr);}
            catch (...) {fputs("caught unknown exception",stderr);}
            puts(cmd.result.c_str());
          }
#ifdef READLINE
        if (strlen(c)) add_history(c);
        free(c);
#endif
      }
    cmd << "restorebgerror\n";
  }

#ifdef _WIN32
#include <windef.h>
#include <winbase.h>
#include <shellapi.h>
  NEWCMD(shellOpen,1)
  {
    ShellExecute(NULL,"open",argv[1],NULL,NULL,1);
  }
#endif

  // support double-click opening of files on Macintosh, by adding a
  // handler for the open event
  int tk_mac_OpenDocument(ClientData cd,Tcl_Interp *interp,
                           int argc, const char** argv)
  {
    if (argc>1)
      {
        tclvar("argv")[1]=argv[1];
        tclvar("argc")=2;
      }
    return TCL_OK;
  }

  static int tk_mac_OpenDocument_dummy=
    (Tcl_CreateCommand
     (interp(),"::tk::mac::OpenDocument",(Tcl_CmdProc*)tk_mac_OpenDocument,0,NULL),
     0);


}
