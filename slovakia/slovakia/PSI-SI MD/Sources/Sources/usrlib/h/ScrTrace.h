#ifndef INC_TRACE_H

#define INC_TRACE_H

// TRACE section - definitions for trace and debug functions

#ifndef myTraceFun
#define	myTraceFun	trace_fun
#endif

#ifdef SCRTRACE
	#define	trace	myTraceFun
#else
	#define trace	1 ? 0 : myTraceFun
#endif

int Z_USERDLL trace_fun( char *s, ... ) ;

#ifndef mysTraceFun
#define	mysTraceFun	strace_fun
#endif

#ifdef SCRTRACE
	#define	strace	mysTraceFun
#else
	#define strace	1 ? 0 : mysTraceFun
#endif

int Z_USERDLL strace_fun( char *s, ... ) ;


#endif
