#include <unistd.h>
#include <cstdarg>
#include <cstdlib>
#include <string.h>

#include "ugoc_utility.h"

FILE *FOPEN(const char fname[], char const flag[])
{/*{{{*/
	FILE *fd = fopen(fname, flag);
	if (fd==NULL){
		fprintf(stderr, "Unable to open file %s with flag %s\n", fname, flag);
		exit(-1);
	}
	return fd;
}/*}}}*/

char *Strcpy(char *&dest, const char src[])
{/*{{{*/
	if(dest != NULL) Free_1d_array(dest);
	dest = new char [strlen(src)+1];
	strcpy(dest,src);
	return dest;
}/*}}}*/

void ErrorExit(const char file[], int line, int exit_value, const char *format, ...)
{/*{{{*/
	va_list args;
	char msg[1024];

	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);
	fprintf(stderr, "Error: %s L%d: ",file,line);
	fprintf(stderr, msg);
	exit(exit_value);

}/*}}}*/

double CalCpuTime(clock_t real, struct tms *tms_start, struct tms * tms_end, bool print) {
	static long clktck = 0;
	if (clktck == 0){
		if ((clktck = sysconf(_SC_CLK_TCK)) < 0){
			fprintf(stderr,"Warning: line %d: sysconf error\n",__LINE__);
			return -1;
		}
	}
	double utime = (tms_end->tms_utime - tms_start->tms_utime) / (double) clktck;
	double stime = (tms_end->tms_stime - tms_start->tms_stime) / (double) clktck;
	if (print){
		fprintf(stdout," real  %7.2f\n", real/(double) clktck);
		fprintf(stdout," user: %7.2f\n", utime);
		fprintf(stdout," sys:  %7.2f\n", stime);
	}
	return utime + stime;
}

void TicTimer(clock_t &start_t_i, struct tms &tms_start, bool &timeron) {
  if ((start_t_i = times(&tms_start)) == -1) {
    timeron = false;
    fprintf(stderr,"Warning: %s, line %d: times error\n",__FILE__,__LINE__);
  }
}

void TocTimer(clock_t &end_t_i, struct tms &tms_end, bool &timeron) {
  if (timeron && (end_t_i = times(&tms_end)) == -1) {
    timeron = false;
    fprintf(stderr,"Warning: %s, line %d: times error\n",__FILE__,__LINE__);
  }
}

