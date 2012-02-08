#ifndef UTILITY_H
#define UTILITY_H

#include <sys/times.h>
#include <cstdio>

using namespace std;

char *Strcpy(char *&dest, const char src[]);

template <class _Tp>
class mem_op {/*{{{*/
	public:
		static _Tp ** new_2d_array(int s_m, int s_n) {
			_Tp **ret;
			ret = new _Tp*[s_m];
			ret[0] = new _Tp[s_m*s_n];
			for( int i = 1; i < s_m; i++ ) ret[i] = ret[i-1] + s_n;
			return ret;
		}
		static bool delete_2d_array(_Tp ***ptr) {
			if (*ptr == NULL) return false;
			delete [] (*ptr)[0];
			delete [] *ptr;
			*ptr = NULL;
			return true;
		}
		static void reallocate_2d_array(_Tp ***ptr,
                                    const int m, const int n,
                                    int *m_size, int *n_size,
                                    int *array_size) {
			/* Not enough space */
			if (m * n > *array_size ) {
				mem_op<_Tp>::delete_2d_array(ptr);
				*ptr = mem_op<_Tp>::new_2d_array(m, n);
				*m_size = m;
				*n_size = n;
				*array_size = m * n;
			} else if (m > *m_size) {
        /* Enough space but m > *m_size */
				_Tp *space_ptr = (*ptr)[0];
				delete [] *ptr;
				*ptr = new _Tp*[m];
				(*ptr)[0] = space_ptr;
				for(int i = 1; i < m; ++i) {
          (*ptr)[i] = (*ptr)[i-1] + n;
        }
				*m_size = m;
				*n_size = n;
			} else if (n != *n_size) {
        /* Enough space, m < m_size, but n != n_size */
        *n_size = n;
				for(int i = 1; i < *m_size; ++i) {
          (*ptr)[i] = (*ptr)[i-1] + n;
        }
      }
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
