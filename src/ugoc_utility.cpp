#include <unistd.h>
#include <cstdarg>
#include <cstdlib>
#include <string.h>
#include <iostream>

#include "ugoc_utility.h"

using std::cout;
using std::cerr;
using std::endl;

FILE *FOPEN(const char fname[], char const flag[])
{/*{{{*/
	FILE *fd = fopen(fname, flag);
	if (fd==NULL){
		fprintf(stderr, "Unable to open file %s with flag %s\n", fname, flag);
		exit(-1);
	}
	return fd;
}/*}}}*/

char *Strcpy(char *&dest, const char src[]) {/*{{{*/
	if(dest != NULL) Free_1d_array(dest);
	dest = new char [strlen(src) + 1];
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

void CalCpuTime(clock_t real, struct tms *tms_start, struct tms * tms_end,
                double *rtime, double *utime, double *stime,
                bool print) {
	static long clktck = 0;
	if (clktck == 0){
		if ((clktck = sysconf(_SC_CLK_TCK)) < 0){
			fprintf(stderr,"Warning: line %d: sysconf error\n",__LINE__);
			return;
		}
	}
  *rtime = real/(double) clktck;
	*utime = (tms_end->tms_utime - tms_start->tms_utime) / (double) clktck;
	*stime = (tms_end->tms_stime - tms_start->tms_stime) / (double) clktck;
	if (print) {
		fprintf(stdout," real  %7.2f\n", *rtime);
		fprintf(stdout," user: %7.2f\n", *utime);
		fprintf(stdout," sys:  %7.2f\n", *stime);
	}
}

clock_t TimeStamp(struct tms *tms_stamp, bool *timeron) {
  clock_t real_t;
  if (*timeron == false) {
    return -1;
  } else if ((real_t = times(tms_stamp)) == -1) {
    *timeron = false;
    fprintf(stderr,"Warning: %s, line %d: times error\n",__FILE__,__LINE__);
    return -1;
  }
  return real_t;
}

void Timer::Print() {
  double rtime, utime, stime;
  double sum_rtime = 0, sum_utime = 0, sum_stime = 0;
  cout << "ID    real    user     sys     log\n";
  cout << "-----------------------------------\n";
  for (unsigned i = 0; i < log.size(); ++i) {
    if (tic_not_toc[i]) {
      cout << "Timer is tic but not toc.";
    } else {
      CalCpuTime(e_real[i] - s_real[i],
                 &s_stamp[i], &e_stamp[i],
                 &rtime, &utime, &stime, false);
      cout << setw(2) << i 
        << setw(8) << rtime
        << setw(8) << utime
        << setw(8) << stime;
      sum_rtime += rtime;
      sum_utime += utime;
      sum_stime += stime;
    }
    cout << setw(8) << log[i] << endl;
  }
  cout << "------------------------------------\n";
  cout << "Sum"
    << setw(7) << sum_rtime
    << setw(8) << sum_utime
    << setw(8) << sum_stime
    << setw(8) << sum_utime + sum_stime << "\n";
}

unsigned Timer::Tic(const char* msg) {
  unsigned i = s_stamp.size();
  log.push_back(msg);
  s_stamp.push_back(tms());
  e_stamp.push_back(tms());
  s_real.push_back(TimeStamp(&s_stamp[i], &is_working));
  e_real.push_back(0);
  tic_not_toc.push_back(true);
  return i;
}

void Timer::Toc(unsigned i) {
  if (!tic_not_toc[i]) {
    cerr << "Warning: timer[" << i << "] re-stamped, overwrite.\n";
  }
  e_real[i] = TimeStamp(&e_stamp[i], &is_working);
  tic_not_toc[i] = false;
}
