#include <string.h>
#include <stdio.h>
#include <time.h>

#include "wu_log.h"

extern char log_date_now[LOG_DATE_NOW_SIZE];

void
localtime_now(void)
{
	struct tm *tmv;
	time_t now;

	now = time(0);
	tmv = localtime(&now);

	strftime(log_date_now, LOG_DATE_NOW_SIZE, "%d/%b/%Y:%T", tmv);

	return;
}

void
wu_display_minhr(char *minhr)
{
	struct tm *tmv;
	time_t now;

	memset(minhr, 0, 10);
	now = time(0);
	tmv = localtime(&now);

	strftime(minhr, 10, "%H:%M", tmv);

	return;
}
