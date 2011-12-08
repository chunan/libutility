#ifndef UTILITY_H
#define UTILITY_H

#include <sys/times.h>
#include <cstdio>

using namespace std;

template <class _Tp>
class mem_op {/*{{{*/
	public:
		static _Tp ** new_2d_array( int s_m, int s_n ) {
			_Tp **ret;
			ret = new _Tp*[s_m];
			ret[0] = new _Tp[s_m*s_n];
			for( int i = 1; i < s_m; i++ ) ret[i] = ret[i-1] + s_n;
			return ret;
		}
		static bool delete_2d_array( _Tp **&ptr ) {
			if (ptr == NULL) return false;
			delete [] ptr[0];
			delete [] ptr;
			ptr = NULL;
			return true;
		}
		static void reallocate_2d_array( _Tp **&ptr, const int s_m, const int s_n, int &old_s_m, int &old_s_n, int &array_size){
			/* Not enough space */
			if(s_m * s_n > array_size ){
				mem_op<_Tp>::delete_2d_array(ptr);
				ptr = mem_op<_Tp>::new_2d_array(s_m,s_n);
				old_s_m = s_m;
				old_s_n = s_n;
				array_size = s_m * s_n;
			}
			/* Enough space but s_n differ or more s_m required */
			else if(s_m > old_s_m || s_n != old_s_n){
				_Tp *space_ptr = ptr[0];
				delete [] ptr;
				ptr = new _Tp*[s_m];
				ptr[0] = space_ptr;
				for( int i = 1; i < s_m; i++ ) ptr[i] = ptr[i-1] + s_n;
				old_s_m = s_m;
				old_s_n = s_n;
			}
			/* Ohterwise less pointer required && s_n not changed */
		}
};/*}}}*/

template <class _Tp>
bool Free_1d_array(_Tp *&ptr)
{/*{{{*/
	if (ptr == NULL) return false;
	delete [] ptr;
	ptr = NULL;
	return true;
}/*}}}*/

double CalCpuTime(clock_t real, struct tms *tms_start, struct tms * tms_end, bool print);

void TicTimer(clock_t &start_t_i, struct tms &tms_start, bool &timeron);

void TocTimer(clock_t &end_t_i, struct tms &tms_end, bool &timeron);

FILE *FOPEN(const char fname[], char const flag[]);

void ErrorExit(const char file[], int line, int exit_value, const char *format, ...);

#endif
