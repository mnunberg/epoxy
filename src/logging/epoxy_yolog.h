
/*
This file was automatically generated from '../../contrib/yolog/srcutil/genyolog.pl'. It contains the macro
wrappers and function definitions.
*/
/**
 *  Initial version August 2010
 *  Second revision June 2012
 *  Copyright 2010-2012 M. Nunberg
 *  See the included LICENSE file for distribution terms
 *
 *
 *  Yolog is a simple logging library.
 *
 *  It has several goals
 *
 *  1) Make initial usage and learning curve as *easy* as possible. It should be
 *      simple enough to generate loging output for most cases
 *
 *  2) While simplicity is good, flexibility is good too. From my own
 *      experience, programs will tend to accumulate a great deal of logging
 *      info. In common situations they are just commented out or removed in
 *      production deployments. This is probably not the right way to go.
 *      Logging statements at times can function as comments, and should be
 *      enabled when required.
 *
 *      Thus output control and performance should be flexible, but not at the
 *      cost of reduced simplicity.
 *
 *  3) Reduction of boilerplate. Logging should be more about what's being
 *      logged, and less about which particular output control or context is
 *      being used. Through the use of preprocessor macros, this information
 *      should be implicit using a simple identifier which is the logging
 *      entry point.
 *
 *
 *  As such, the architecture is designed as follows:
 *
 *  Logging Context
 *      This is the main component. A logging context represents a class or
 *      subsystem within the application which emits messages. This logging
 *      context is created or initialized by the application developer
 *      appropriate to his or her application.
 *
 *      For example, an HTTP client may contain the following systems
 *          htparse: Subsystem which handles HTTP parsing
 *          sockpool: Subsystem maintaining connection pools
 *          srvconn:  Subsystem which intializes new connections to the server
 *          io: Reading/writing and I/O readiness notifications
 *
 *  Logging Levels:
 *      Each subsystem has various degrees of information it may wish to emit.
 *      Some information is relevant only while debugging an application, while
 *      other information may be relevant to its general maintenance.
 *
 *      For example, an HTTP parser system may wish to output the tokens it
 *      encounters during its processing of the TCP stream, but would also
 *      like to notify about bad messages, or such which may exceed the maximum
 *      acceptable header size (which may hint at an attack or security risk).
 *
 *      Logging messages of various degrees concern various aspects of the
 *      application's development and maintenance, and therefore need individual
 *      output control.
 *
 *  Configuration:
 *      Because of the varying degrees of flexibility required in a logging
 *      library, there are multiple configuration stages.
 *
 *      1) Project configuration.
 *          As the project author, you are aware of the subsystems which your
 *          application provides, and should be able to specify this as a
 *          compile/build time parameter. This includes the types of subsystems
 *          as well as the identifier/macro names which your application will
 *          use to emit logging messages. The burden should not be placed
 *          on the application developer to actually perform the boilerplate
 *          of writing logging wrappers, as this can be generally abstracted
 *
 *      2) Runtime/Initialization configuration.
 *          This is performed by users who are interested in different aspects
 *          of your application's logging systems. Thus, a method of providing
 *          configuration files and environment variables should be used in
 *          order to allow users to modify logging output from outside the code.
 *
 *          Additionally, you as the application developer may be required to
 *          trigger this bootstrap process (usually a few function calls from
 *          your program's main() function).
 *
 *      3) There is no logging configuration or setup!
 *          Logging messages themselves should be entirely about the message
 *          being emitted, with the metadata/context information being implicit
 *          and only the information itself being explicit.
 */

#ifndef EPOXY_YOLOG_H_
#define EPOXY_YOLOG_H_

#include <stdio.h>
#include <stdarg.h>

typedef enum {
    /* make at least one signed number here */
    _EPOXY_YOLOG_LEVEL_MAKE_COMPILER_HAPPY = -1,

#define EPOXY_YOLOG_XLVL(X) \
    X(DEFAULT,  0) \
    /* really transient messages */ \
    X(RANT,     1) \
    /* function enter/leave events */ \
    X(TRACE,    2) \
    /* state change events */ \
    X(STATE,    3) \
    /* generic application debugging events */ \
    X(DEBUG,    4) \
    /* informational messages */ \
    X(INFO,     5) \
    /* warning messages */ \
    X(WARN,     6) \
    /* error messages */ \
    X(ERROR,    7) \
    /* critical messages */ \
    X(CRIT,     8)

#define X(lvl, i) \
    EPOXY_YOLOG_##lvl = i,
    EPOXY_YOLOG_XLVL(X)
#undef X
    EPOXY_YOLOG_LEVEL_MAX
} epoxy_yolog_level_t;

#define EPOXY_YOLOG_LEVEL_INVALID -1
#define EPOXY_YOLOG_LEVEL_UNSET 0

enum {
    /* screen output */
    EPOXY_YOLOG_OUTPUT_SCREEN = 0,

    /* global file */
    EPOXY_YOLOG_OUTPUT_GFILE,

    /* specific file */
    EPOXY_YOLOG_OUTPUT_PFILE,

    EPOXY_YOLOG_OUTPUT_COUNT
};

#define EPOXY_YOLOG_OUTPUT_ALL EPOXY_YOLOG_OUTPUT_COUNT

#ifndef EPOXY_YOLOG_API
#define EPOXY_YOLOG_API
#endif

struct epoxy_yolog_context;
struct epoxy_yolog_fmt_st;

/**
 * Callback to be invoked when a logging message arrives.
 * This is passed the logging context, the level, and message passed.
 */
typedef void
        (*epoxy_yolog_callback)(
                struct epoxy_yolog_context*,
                epoxy_yolog_level_t,
                va_list ap);


enum {
    EPOXY_YOLOG_LINFO_DEFAULT_ONLY = 0,
    EPOXY_YOLOG_LINFO_DEFAULT_ALSO = 1
};

typedef enum {

    #define EPOXY_YOLOG_XFLAGS(X) \
    X(NOGLOG, 0x1) \
    X(NOFLOG, 0x2) \
    X(COLOR, 0x10)
    #define X(c,v) EPOXY_YOLOG_F_##c = v,
    EPOXY_YOLOG_XFLAGS(X)
    #undef X
    EPOXY_YOLOG_F_MAX = 0x200
} epoxy_yolog_flags_t;

/* maximum size of heading/trailing user data between format specifiers */
#define EPOXY_YOLOG_FMT_USTR_MAX 16

/* Default format string */
#define EPOXY_YOLOG_FORMAT_DEFAULT \
    "[%(prefix)] %(filename):%(line) %(color)(%(func)) "


enum {
    EPOXY_YOLOG_FMT_LISTEND = 0,
    EPOXY_YOLOG_FMT_USTRING,
    EPOXY_YOLOG_FMT_EPOCH,
    EPOXY_YOLOG_FMT_PID,
    EPOXY_YOLOG_FMT_TID,
    EPOXY_YOLOG_FMT_LVL,
    EPOXY_YOLOG_FMT_TITLE,
    EPOXY_YOLOG_FMT_FILENAME,
    EPOXY_YOLOG_FMT_LINE,
    EPOXY_YOLOG_FMT_FUNC,
    EPOXY_YOLOG_FMT_COLOR
};


/* structure representing a single compiled format specifier */
struct epoxy_yolog_fmt_st {
    /* format type, opaque, derived from format string */
    int type;
    /* user string, heading or trailing padding, depending on the type */
    char ustr[EPOXY_YOLOG_FMT_USTR_MAX];
};

struct epoxy_yolog_msginfo_st {
    const char *co_line;
    const char *co_title;
    const char *co_reset;

    const char *m_func;
    const char *m_file;
    const char *m_prefix;

    int m_level;
    int m_line;
    unsigned long m_time;
};

struct epoxy_yolog_output_st {
    FILE *fp;
    struct epoxy_yolog_fmt_st *fmtv;
    int use_color;
    int level;
};

struct epoxy_yolog_context;

typedef struct epoxy_yolog_context_group {
    struct epoxy_yolog_context *contexts;
    int ncontexts;
    epoxy_yolog_callback cb;
    struct epoxy_yolog_output_st o_file;
    struct epoxy_yolog_output_st o_screen;
} epoxy_yolog_context_group;

typedef struct epoxy_yolog_context {
    /**
     * The minimum allowable logging level.
     * Performance number so we don't have to iterate over the entire
     * olevels array each time. This should be kept in sync with sync_levels
     * after any modification to olevels
     */
    epoxy_yolog_level_t level;

    struct epoxy_yolog_context_group *parent;

    /**
     * Array of per-output-type levels
     */
    epoxy_yolog_level_t olevels[EPOXY_YOLOG_OUTPUT_COUNT];

    /**
     * This is a human-readable name for the context. This is the 'prefix'.
     */
    const char *prefix;

    /**
     * If this subsystem logs to its own file, then it is set here
     */
    struct epoxy_yolog_output_st *o_alt;
} epoxy_yolog_context;


/**
 * These two functions log an actual message.
 *
 * @param ctx - The context. Can be NULL to use the default/global context.
 * @param level - The severity of this message
 * @param file - The filename of this message (e.g. __FILE__)
 * @param line - The line of this message (e.g. __LINE__)
 * @param fn - The function name (e.g. __func__)
 * @param fmt - User-defined format (printf-style)
 * @param args .. extra arguments to format
 */
EPOXY_YOLOG_API
void
epoxy_yolog_logger(epoxy_yolog_context *ctx,
             epoxy_yolog_level_t level,
             const char *file,
             int line,
             const char *fn,
             const char *fmt,
             ...);

/**
 * Same thing as epoxy_yolog_logger except it takes a va_list instead.
 */
EPOXY_YOLOG_API
void
epoxy_yolog_vlogger(epoxy_yolog_context *ctx,
              epoxy_yolog_level_t level,
              const char *file,
              int line,
              const char *fn,
              const char *fmt,
              va_list ap);

/**
 * Initialize the default logging settings. This function *must* be called
 * some time before any logging messages are invoked, or disaster may ensue.
 * (Or not).
 */
EPOXY_YOLOG_API
void
epoxy_yolog_init_defaults(epoxy_yolog_context_group *grp,
                    epoxy_yolog_level_t default_level,
                    const char *color_env,
                    const char *level_env);


/**
 * Compile a format string into a format structure.
 *
 *
 * Format strings can have specifiers as such: %(spec). This was taken
 * from Python's logging module which is probably one of the easiest logging
 * libraries to use aside from Yolog, of course!
 *
 * The following specifiers are available:
 *
 * %(epoch) - time(NULL) result
 *
 * %(pid) - The process ID
 *
 * %(tid) - The thread ID. On Linux this is gettid(), on other POSIX systems
 *  this does a byte-for-byte representation of the returned pthread_t from
 *  pthread_self(). On non-POSIX systems this does nothing.
 *
 * %(level) - A level string, e.g. 'DEBUG', 'ERROR'
 *
 * %(filename) - The source file
 *
 * %(line) - The line number at which the logging function was invoked
 *
 * %(func) - The function from which the function was invoked
 *
 * %(color) - This is a special specifier and indicates that normal
 *  severity color coding should begin here.
 *
 *
 * @param fmt the format string
 * @return a list of allocated format structures, or NULL on error. The
 * format structure list may be freed by free()
 */
EPOXY_YOLOG_API
struct epoxy_yolog_fmt_st *
epoxy_yolog_fmt_compile(const char *fmt);


/**
 * Sets the format string of a context.
 *
 * Internally this calls fmt_compile and then sets the format's context,
 * freeing any existing context.
 *
 * @param ctx the context which should utilize the format
 * @param fmt a format string.
 * @param replace whether to replace the existing string (if any)
 *
 * @return 0 on success, -1 if there was an error setting the format.
 *
 *
 */
EPOXY_YOLOG_API
int
epoxy_yolog_set_fmtstr(struct epoxy_yolog_output_st *output,
                 const char *fmt,
                 int replace);


EPOXY_YOLOG_API
void
epoxy_yolog_set_screen_format(epoxy_yolog_context_group *grp,
                        const char *format);

/**
 * Yolog maintains a global object for messages which have no context.
 * This function gets this object.
 */
EPOXY_YOLOG_API
epoxy_yolog_context *
epoxy_yolog_get_global(void);

/**
 * This will read a file and apply debug settings from there..
 *
 * @param contexts an aray of logging contexts
 * @param ncontext the count of contexts
 * @param filename the filename containing the settings
 * @param cb a callback to invoke when an unrecognized line is found
 * @param error a pointer to a string which shall contain an error
 *
 * @return true on success, false on failure. error will be set on failure as
 * well. Should be freed with 'free()'
 */
EPOXY_YOLOG_API
int
epoxy_yolog_parse_file(epoxy_yolog_context_group *grp,
                 const char *filename);


EPOXY_YOLOG_API
void
epoxy_yolog_parse_envstr(epoxy_yolog_context_group *grp,
                const char *envstr);

/**
 * These functions are mainly private
 */

/**
 * This is a hack for C89 compilers which don't support variadic macros.
 * In this case we maintain a global context which is initialized and locked.
 *
 * This function tries to lock the context (it checks to see if this level can
 * be logged, locks the global structure, and returns true. If the level
 * cannot be logged, false is retured).
 *
 * The functions implicit_logger() and implicit_end() should only be called
 * if implicit_begin() returns true.
 */
int
epoxy_yolog_implicit_begin(epoxy_yolog_context *ctx,
                     int level,
                     const char *file,
                     int line,
                     const char *fn);

/**
 * printf-compatible wrapper which operates on the implicit structure
 * set in implicit_begin()
 */
void
epoxy_yolog_implicit_logger(const char *fmt, ...);

/**
 * Unlocks the implicit structure
 */
void
epoxy_yolog_implicit_end(void);


void
epoxy_yolog_fmt_write(struct epoxy_yolog_fmt_st *fmts,
                FILE *fp,
                const struct epoxy_yolog_msginfo_st *minfo);


void
epoxy_yolog_sync_levels(epoxy_yolog_context *ctx);

/**
 * These are the convenience macros. They are disabled by default because I've
 * made some effort to make epoxy_yolog more embed-friendly and not clobber a project
 * with generic functions.
 *
 * The default macros are C99 and employ __VA_ARGS__/variadic macros.
 *
 *
 *
 */
#ifdef EPOXY_YOLOG_ENABLE_MACROS

#ifndef EPOXY_YOLOG_C89_MACROS

/**
 * These macros are all invoked with double-parens, so
 * epoxy_yolog_debug(("foo"));
 * This way the 'variation' of the arguments is dispatched to the actual C
 * function instead of the macro..
 */

#define epoxy_yolog_debug(...) \
epoxy_yolog_logger(\
    NULL\
    EPOXY_YOLOG_DEBUG, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)

#define epoxy_yolog_info(...) \
epoxy_yolog_logger(\
    NULL\
    EPOXY_YOLOG_INFO, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)

#define epoxy_yolog_warn(...) \
epoxy_yolog_logger(\
    NULL\
    EPOXY_YOLOG_WARN, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)

#define epoxy_yolog_error(...) \
epoxy_yolog_logger(\
    NULL\
    EPOXY_YOLOG_ERROR, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)

#define epoxy_yolog_crit(...) \
epoxy_yolog_logger(\
    NULL\
    EPOXY_YOLOG_CRIT, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)

#else /* ifdef EPOXY_YOLOG_C89_MACROS */


#define epoxy_yolog_debug(args) \
if (epoxy_yolog_implicit_begin( \
    NULL, \
    EPOXY_YOLOG_DEBUG, \
    __FILE__, \
    __LINE__, \
    __func__)) \
{ \
    epoxy_yolog_implicit_logger args; \
    epoxy_yolog_implicit_end(); \
}

#define epoxy_yolog_info(args) \
if (epoxy_yolog_implicit_begin( \
    NULL, \
    EPOXY_YOLOG_INFO, \
    __FILE__, \
    __LINE__, \
    __func__)) \
{ \
    epoxy_yolog_implicit_logger args; \
    epoxy_yolog_implicit_end(); \
}

#define epoxy_yolog_warn(args) \
if (epoxy_yolog_implicit_begin( \
    NULL, \
    EPOXY_YOLOG_WARN, \
    __FILE__, \
    __LINE__, \
    __func__)) \
{ \
    epoxy_yolog_implicit_logger args; \
    epoxy_yolog_implicit_end(); \
}

#define epoxy_yolog_error(args) \
if (epoxy_yolog_implicit_begin( \
    NULL, \
    EPOXY_YOLOG_ERROR, \
    __FILE__, \
    __LINE__, \
    __func__)) \
{ \
    epoxy_yolog_implicit_logger args; \
    epoxy_yolog_implicit_end(); \
}

#define epoxy_yolog_crit(args) \
if (epoxy_yolog_implicit_begin( \
    NULL, \
    EPOXY_YOLOG_CRIT, \
    __FILE__, \
    __LINE__, \
    __func__)) \
{ \
    epoxy_yolog_implicit_logger args; \
    epoxy_yolog_implicit_end(); \
}

#endif /* EPOXY_YOLOG_C89_MACROS */

#endif /* EPOXY_YOLOG_ENABLE_MACROS */

#endif /* EPOXY_YOLOG_H_ */

/** These macros define the subsystems for logging **/

#define EPOXY_YOLOG_LOGGING_SUBSYS_CLIENT        0
#define EPOXY_YOLOG_LOGGING_SUBSYS_ACCEPT        1
#define EPOXY_YOLOG_LOGGING_SUBSYS_LCBT          2
#define EPOXY_YOLOG_LOGGING_SUBSYS__COUNT        3




/** Array of context object for each of our subsystems **/
extern epoxy_yolog_context* epoxy_yolog_logging_contexts;
extern epoxy_yolog_context_group epoxy_yolog_log_group;

/** Function called to initialize the logging subsystem **/

void
epoxy_yolog_init(const char *configfile);


/** Macro to retrieve information about a specific subsystem **/

#define epoxy_yolog_subsys_ctx(subsys) \
    (epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_ ## subsys)

#define epoxy_yolog_subsys_count() (EPOXY_YOLOG_LOGGING_SUBSYS__COUNT)

#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 0))
#define log_rant(...)
#else
#define log_rant(...) \
epoxy_yolog_logger(\
    NULL,\
    EPOXY_YOLOG_RANT, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 1))
#define log_trace(...)
#else
#define log_trace(...) \
epoxy_yolog_logger(\
    NULL,\
    EPOXY_YOLOG_TRACE, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 2))
#define log_state(...)
#else
#define log_state(...) \
epoxy_yolog_logger(\
    NULL,\
    EPOXY_YOLOG_STATE, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 3))
#define log_debug(...)
#else
#define log_debug(...) \
epoxy_yolog_logger(\
    NULL,\
    EPOXY_YOLOG_DEBUG, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 4))
#define log_info(...)
#else
#define log_info(...) \
epoxy_yolog_logger(\
    NULL,\
    EPOXY_YOLOG_INFO, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 5))
#define log_warn(...)
#else
#define log_warn(...) \
epoxy_yolog_logger(\
    NULL,\
    EPOXY_YOLOG_WARN, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 6))
#define log_error(...)
#else
#define log_error(...) \
epoxy_yolog_logger(\
    NULL,\
    EPOXY_YOLOG_ERROR, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 7))
#define log_crit(...)
#else
#define log_crit(...) \
epoxy_yolog_logger(\
    NULL,\
    EPOXY_YOLOG_CRIT, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 0))
#define log_client_rant(...)
#else
#define log_client_rant(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_CLIENT,\
    EPOXY_YOLOG_RANT, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 1))
#define log_client_trace(...)
#else
#define log_client_trace(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_CLIENT,\
    EPOXY_YOLOG_TRACE, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 2))
#define log_client_state(...)
#else
#define log_client_state(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_CLIENT,\
    EPOXY_YOLOG_STATE, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 3))
#define log_client_debug(...)
#else
#define log_client_debug(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_CLIENT,\
    EPOXY_YOLOG_DEBUG, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 4))
#define log_client_info(...)
#else
#define log_client_info(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_CLIENT,\
    EPOXY_YOLOG_INFO, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 5))
#define log_client_warn(...)
#else
#define log_client_warn(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_CLIENT,\
    EPOXY_YOLOG_WARN, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 6))
#define log_client_error(...)
#else
#define log_client_error(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_CLIENT,\
    EPOXY_YOLOG_ERROR, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 7))
#define log_client_crit(...)
#else
#define log_client_crit(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_CLIENT,\
    EPOXY_YOLOG_CRIT, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 0))
#define log_accept_rant(...)
#else
#define log_accept_rant(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_ACCEPT,\
    EPOXY_YOLOG_RANT, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 1))
#define log_accept_trace(...)
#else
#define log_accept_trace(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_ACCEPT,\
    EPOXY_YOLOG_TRACE, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 2))
#define log_accept_state(...)
#else
#define log_accept_state(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_ACCEPT,\
    EPOXY_YOLOG_STATE, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 3))
#define log_accept_debug(...)
#else
#define log_accept_debug(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_ACCEPT,\
    EPOXY_YOLOG_DEBUG, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 4))
#define log_accept_info(...)
#else
#define log_accept_info(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_ACCEPT,\
    EPOXY_YOLOG_INFO, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 5))
#define log_accept_warn(...)
#else
#define log_accept_warn(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_ACCEPT,\
    EPOXY_YOLOG_WARN, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 6))
#define log_accept_error(...)
#else
#define log_accept_error(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_ACCEPT,\
    EPOXY_YOLOG_ERROR, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 7))
#define log_accept_crit(...)
#else
#define log_accept_crit(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_ACCEPT,\
    EPOXY_YOLOG_CRIT, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 0))
#define log_lcbt_rant(...)
#else
#define log_lcbt_rant(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_LCBT,\
    EPOXY_YOLOG_RANT, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 1))
#define log_lcbt_trace(...)
#else
#define log_lcbt_trace(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_LCBT,\
    EPOXY_YOLOG_TRACE, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 2))
#define log_lcbt_state(...)
#else
#define log_lcbt_state(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_LCBT,\
    EPOXY_YOLOG_STATE, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 3))
#define log_lcbt_debug(...)
#else
#define log_lcbt_debug(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_LCBT,\
    EPOXY_YOLOG_DEBUG, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 4))
#define log_lcbt_info(...)
#else
#define log_lcbt_info(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_LCBT,\
    EPOXY_YOLOG_INFO, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 5))
#define log_lcbt_warn(...)
#else
#define log_lcbt_warn(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_LCBT,\
    EPOXY_YOLOG_WARN, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 6))
#define log_lcbt_error(...)
#else
#define log_lcbt_error(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_LCBT,\
    EPOXY_YOLOG_ERROR, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
#if (defined EPOXY_YOLOG_DEBUG_LEVEL \
    && (EPOXY_YOLOG_DEBUG_LEVEL > 7))
#define log_lcbt_crit(...)
#else
#define log_lcbt_crit(...) \
epoxy_yolog_logger(\
    epoxy_yolog_logging_contexts + EPOXY_YOLOG_LOGGING_SUBSYS_LCBT,\
    EPOXY_YOLOG_CRIT, \
    __FILE__, \
    __LINE__, \
    __func__, \
    ## __VA_ARGS__)
#endif /* EPOXY_YOLOG_NDEBUG_LEVEL */
