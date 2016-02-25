#ifndef NASIO_LOGGER_H_
#define NASIO_LOGGER_H_

#ifdef NASIO_DEBUG
#define nasio_env_log(env, l, fmt, ...) do{\
    if( env->logger.callback && l>=env->logger.level ){\
        env->logger.callback( l, fmt, __VA_ARGS__ );\
    }\
}while(0)
#else 
#define nasio_env_log(env, l, fmt, ...)
#endif

#define nasio_log_trace(env, fmt, ...) nasio_env_log(env, NASIO_LOG_LEVEL_TRACE, fmt, __VA_ARGS__)
#define nasio_log_info(env, fmt, ...) nasio_env_log(env, NASIO_LOG_LEVEL_INFO, fmt, __VA_ARGS__)
#define nasio_log_debug(env, fmt, ...) nasio_env_log(env, NASIO_LOG_LEVEL_DEBUG, fmt, __VA_ARGS__)
#define nasio_log_warn(env, fmt, ...) nasio_env_log(env, NASIO_LOG_LEVEL_WARN, fmt, __VA_ARGS__)
#define nasio_log_error(env, fmt, ...) nasio_env_log(env, NASIO_LOG_LEVEL_ERROR, fmt, __VA_ARGS__)
#define nasio_log_fatal(env, fmt, ...) nasio_env_log(env, NASIO_LOG_LEVEL_FATAL, fmt, __VA_ARGS__)

#ifdef NASIO_DEBUG
void default_log_cb(int level, const char *fmt, ...);
#endif

#endif
