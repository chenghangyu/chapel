#ifndef _chplcomm_h_
#define _chplcomm_h_

#include "chpltypes.h"
#include "chpl_cfg_comm.h"

extern _int32 _localeID;   // unique ID for each locale: 0, 1, 2, ...
extern _int32 _numLocales; // number of locales

typedef void (*func_p)(void*);

typedef struct {
  _int32  locale; // locale that owns the data
  _int32  size;   // size in bytes
  void   *addr;   // local address of data
} _chpl_comm_ptr_t;

#define _chpl_comm_set_fat(fat, ilocale, isize, iaddr) \
  (fat).locale = ilocale;                              \
  (fat).size = isize;                                  \
  (fat).addr = iaddr

#define _chpl_comm_get_off(addr, locale, raddr, size, stype, sfield)   \
  _chpl_comm_get(addr, locale,                                         \
                  (char*)raddr + ((char*)(&(((stype*)addr)->sfield))-(char*)addr), \
                  sizeof(((stype*)addr)->sfield))

#define _chpl_comm_put_off(addr, locale, raddr, size, stype, sfield)  \
  _chpl_comm_put(addr, locale,                                        \
                   (char*)raddr + ((char*)(&(((stype*)addr)->sfield))-(char*)addr), \
                   sizeof(((stype*)addr)->sfield))

//
// given the program arguments, returns whether the invocation of
// main() is the original invocation by the user (1) or a recursive
// invocation made by the comm layer to get the program started (0).
//
int _chpl_comm_user_invocation(int argc, char* argv[]);

//
// Returns the default number of locales to use for this comm layer if
// the user does not specify a number.  For most comm layers, this
// should probably print a helpful error and exit rather than
// defaulting to anything.  For comm layer "none" a default of 1
// locale makes sense which is why this routine exists.  If the
// routine returns a value, that value needs to be consistent across
// multiple calls to the routine.
//
int _chpl_comm_default_num_locales(void);

//
// This routine allows a comm layer to screen the number of locales to
// be used.  In particular, if a number exceeding some sort of maximum
// was provided, an error should be reported.
//
void _chpl_comm_verify_num_locales(_int32 proposedNumLocales);

//
// This routine allows a comm layer to process the argc/argv calls
// provided by main(), and optionally to create a modified version of
// argc/argv for use with _chpl_comm_init().  It is called in the case
// of a user invocation of main() after Chapel has parsed the command
// line arguments to determine the number of locales.
//
// This routine takes as its input arguments:
//
// - execNumLocales: indicating the number of locales the program will
//                   be executed on;
//
// - argc/argv as provided to main().  
//
// It specifies the *commArgc value as an output argument and returns
// a corresponding commArgv vector of size commArgc -- these values
// are then passed into _chpl_comm_init() in the case of a user
// invocation of main().  If the comm layer does not need to modify
// argc/argv, it can just pass them through to commArgc/commArgv.
//
char** _chpl_comm_create_argcv(_int32 execNumLocales, int argc, char* argv[], 
                               int* commArgc);

//
// initializes the communications package
//   set _localeID and _numLocales
// notes:
//   * For a user invocation of main(), this routine is called with
//     the argc/argv pair generated by _chpl_comm_create_argcv
//   * For a comm-layer invocation of main(), it is called with the
//     argc/argv pair passed to main()
//
void _chpl_comm_init(int *argc_p, char ***argv_p);

//
// a final comm layer stub before barrier synching and calling into
// the user code.  It is recommended that a debugging message be
// printed here indicating that each locale has started using
// _chpl_msg() and a verbosity level of 2 (which will cause it to be
// displayed using the -v flag).
//
void _chpl_comm_rollcall(void);

//
// barrier for synchronization between all processes; currently only
// used for startup and teardown.  msg is a string that can be used
// for debugging to determine where the barrier is being called.
//
void _chpl_comm_barrier(const char *msg);

//
// terminates communication package at the end of a normal run of the
// chapel program -- assumes all processes are calling into the
// routine.  If the communication layer likes to call exit, the exit
// code is provided using the "status" argument; if it doesn't, it
// can simply return and the Chapel program will call exit().
//
// notes:
//   this function is called last
//   a barrier is invoked before calling into this function
//   Chapel's program termination is not yet implemented correctly
//
void _chpl_comm_exit_all(int status);


//
// this routine should terminate the communication package when called
// by any thread, and should clean up the communication package's
// resources as best possible.  This routine is called whenever a user
// thread calls halt or exit and we have no guarantees that all threads
// are calling into the halt or exit.  Otherwise, it is much like the
// _chpl_comm_exit_all() routine.
//
void _chpl_comm_exit_any(int status);

//
// put 'size' bytes of local data at 'addr' to remote data at
// 'raddr' on locale 'locale'
// notes:
//   address is arbitrary
//   size and locale are part of p
//
void  _chpl_comm_put(void* addr, _int32 locale, void* raddr, _int32 size);

//
// get 'size' bytes of remote data at 'raddr' on locale 'locale' to
// local data at 'addr'
// notes:
//   address is arbitrary
//   size and locale are part of p
//
void  _chpl_comm_get(void *addr, _int32 locale, void* raddr, _int32 size);

//
// remote fork should launch a thread on locale that runs function f
// passing it arg where the size of arg is stored in arg_size
// notes:
//   multiple forks to the same locale should be handled concurrently
//
void  _chpl_comm_fork(int locale, func_p f, void *arg, int arg_size);

//
// non-blocking fork (not yet used)
//
void  _chpl_comm_fork_nb(int locale, func_p f, void *arg, int arg_size);


#endif
