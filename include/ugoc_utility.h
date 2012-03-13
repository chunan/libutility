#ifndef UTILITY_H
#define UTILITY_H

#include <sys/times.h>
#include <cassert>
#include <cstdio>
#include <deque>
#include <vector>
#include <string>
#include <iomanip>
#include <iostream>

using std::cout;
using std::endl;
using std::setw;
using std::deque;
using std::vector;
using std::string;

char *Strcpy(char *&dest, const char src[]);

template <class _Tp>
inline _Tp Min(const _Tp a, const _Tp b)/*{{{*/
{
	if(a <= b) return a;
	else return b;
}
/*}}}*/

template <typename _Tp>
typename deque<_Tp>::iterator Max(deque<_Tp> &q, int size) {/*{{{*/
	typename deque<_Tp>::iterator maxItr = q.begin();
	for(typename deque<_Tp>::iterator it = q.begin()+1; it != q.end(); it++){
		size--;
		if(size == 0) break;
		if(*it > *maxItr){
			maxItr = it;
		}
	}
	return maxItr;
}/*}}}*/

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

/* Maitain a 2D array stored as an 1D array (continuous space)
 * R(): current number of rows
 * C(): current number of columns
 * Resize(nr, nc): resize to nr-by-nc array
 * Memfill(val): fill the array with val
 * (r, c): return the (r, c) entry (reference to entry)
 * [r]: return the [r] vector (pointer of (r, 0))
 */
template<class _Tp>
class TwoDimArray {
  public:
    TwoDimArray() { Init(); }
    TwoDimArray(int nr, int nc) {
      Init();
      Resize(nr, nc);
    }
    ~TwoDimArray() {
      mem_op<_Tp>::delete_2d_array(&data_);
    }
    void Resize(const int nr, const int nc) {
      nr_ = nr;
      nc_ = nc;
      mem_op<_Tp>::reallocate_2d_array(&data_, nr, nc,
                                       &nr_max_, &nc_max_, &size_);
    }
    void Memfill(const _Tp& val) {
      if (data_ != NULL) {
        for (int i = 0; i < nr_ * nc_; ++i) {
          data_[0][i] = val;
        }
      }
    }
    _Tp& operator()(const int r, const int c) {
      assert(data_ != NULL);
      assert(r >= 0 && r < nr_);
      assert(c >= 0 && c < nc_);
      return data_[r][c];
    }
    _Tp Entry(const int r, const int c) const {
      assert(data_ != NULL);
      assert(r >= 0 && r < nr_);
      assert(c >= 0 && c < nc_);
      return data_[r][c];
    }
    _Tp* operator[](const int r) {
      if (data_ == NULL) {
        return NULL;
      } else {
        assert(r >= 0 && r < nr_);
        return data_[r];
      }
    }
    const _Tp* Vec(const int r) const {
      if (data_ == NULL) {
        return NULL;
      } else {
        assert(r >= 0 && r < nr_);
        return data_[r];
      }
    }
    int R() const { return nr_; }
    int C() const { return nc_; }
    int R_max() const { return nr_max_; }
    int C_max() const { return nc_max_; }
    void DumpData() const {}
  private:
    void Init() {
      data_ = NULL;
      nr_ = nc_ = nr_max_ = nc_max_ = size_ = 0;
    }
    _Tp** data_;
    int nr_;
    int nc_;
    int nr_max_;
    int nc_max_;
    int size_;
};

void CalCpuTime(clock_t real, struct tms *tms_start, struct tms * tms_end,
                double *rtime, double *utime, double *stime,
                bool print);

clock_t TimeStamp(struct tms *tms_stamp, bool *timeron);

FILE *FOPEN(const char fname[], char const flag[]);

void ErrorExit(const char file[], int line, int exit_value, const char *format, ...);

class Timer {
  public:
    Timer() { is_working = true; }
    void Print();
    unsigned Tic(const char* msg);
    void Toc(unsigned i);
  private:
    vector<string> log;
    vector<struct tms> s_stamp, e_stamp;
    vector<clock_t> s_real, e_real;
    vector<bool> tic_not_toc;
    bool is_working;
};

void StripExtension(string* input);

void KeepBasename(string* input);

void KeepBasename(vector<string>* list);

#endif
