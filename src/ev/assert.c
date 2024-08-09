
EV_LOCAL void ev__assertion_failure(const char* exp, const char* file, int line, const char* fmt, ...)
{
    fprintf(stderr, "%s:%d: Assertion failed: %s", file, line, exp);
    if (fmt != NULL)
    {
        fprintf(stderr, ": ");

        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    fprintf(stderr, "\n");
    abort();
}
