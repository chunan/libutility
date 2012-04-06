#ifndef __STD_COMMON_H__
#define __STD_COMMON_H__

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include "thread_util.h"

using std::vector;
using std::string;
using std::cout;
using std::setw;

typedef std::pair<unsigned, unsigned> UPair;

/* A num_query by num_doc array of _Tp
 * Implemented by vector<vector<_Tp> >
 */
template<class _Tp>
class QDArray {/*{{{*/
  public:
    QDArray() {}
    QDArray(unsigned nq, unsigned nd) { Resize(nq, nd); }
    void Resize(unsigned nq, unsigned nd) {
      array.resize(nq);
      for (unsigned q = 0; q < nq; ++q) {
        array[q].resize(nd);
      }
    }
    _Tp& operator()(unsigned q, unsigned d) {
      assert(q < array.size() && d < array[q].size());
      return array[q][d];
    }
    vector<_Tp>& operator[](unsigned q) {
      assert(q < array.size());
      return array[q];
    }
  private:
    vector<vector<_Tp> > array;
};/*}}}*/

struct QueryProfile {/*{{{*/
  int qid;
  vector<unsigned> ignore;
};/*}}}*/

class QueryProfileList {/*{{{*/
  public:
    int Find(int qid) const {
      for (unsigned i = 0; i < profiles.size(); ++i) {
        if (profiles[i].qid == qid) return i;
      }
      return -1;
    }
    int push_back(int qid) {
      profiles.resize(profiles.size() + 1);
      profiles.back().qid = qid;
      profiles.back().ignore.clear();
      return profiles.size() - 1;
    }
    void PushBackIgnore(unsigned qidx, unsigned didx) {
      assert(qidx < profiles.size());
      profiles[qidx].ignore.push_back(didx);
    }
    unsigned size() const { return profiles.size(); }
    const QueryProfile& QP(unsigned qidx) const {
      assert(qidx < profiles.size());
      return profiles[qidx];
    }
    void SortIgnore() {
      for (unsigned i = 0; i < profiles.size(); ++i) {
        sort(profiles[i].ignore.begin(), profiles[i].ignore.end());
      }
    }
    void Print() const {
      cout << " qid  ignore_list\n";  
      for (unsigned qidx = 0; qidx < profiles.size(); ++qidx) {
         cout << setw(4) << profiles[qidx].qid << "  {";
         for (unsigned j = 0; j < profiles[qidx].ignore.size(); ++j) {
           cout << " " << profiles[qidx].ignore[j];
         }
         cout << "}\n";
      }
    }
  private:
    vector<QueryProfile> profiles;
};/*}}}*/

void ParseList(const char *filename,
               vector<string> *list,
               QueryProfileList *profile_list = NULL);

void ParseIgnore(const char* filename,
                 vector<string>& D_list,
                 QueryProfileList* profile_list);

void InitDispatcher(Dispatcher<UPair>* disp,
                    const QueryProfileList& profile_list,
                    const vector<string>& D_list);

void DumpResult(const char* fname,
                const QueryProfileList& profile_list,
                QDArray<vector<float> >& snippet_dist,
                const vector<string>& D_list);

#endif
