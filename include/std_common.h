#ifndef __STD_COMMON_H__
#define __STD_COMMON_H__

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>

using std::vector;
using std::string;
using std::cout;
using std::setw;

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

#endif
