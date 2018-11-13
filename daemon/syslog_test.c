#include <syslog.h>
#include <stdio.h>

int main() {
    openlog("hello world", LOG_PID, LOG_USER);

    for (int i = 0; i < 100; ++i) {
        syslog(LOG_NOTICE, "this is the %d test log message. LOG_NOTICE\n", i);
    }

    for (int i = 0; i < 100; ++i) {
        syslog(LOG_WARNING, "this is the %d test log message. LOG_WARNING\n", i);
    }

    for (int i = 0; i < 100; ++i) {
        syslog(LOG_ERR, "this is the %d test log message. LOG_ERR\n", i);
    }

    setlogmask(LOG_MASK(LOG_NOTICE) | LOG_MASK(LOG_WARNING));

    closelog();
}