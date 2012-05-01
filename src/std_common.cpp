#include <cstring>
#include <typeinfo>
#include <set>
#include "ugoc_utility.h"
#include "std_common.h"
#include "thread_util.h"


using std::cout;
using std::cerr;

namespace StdCommonUtil {

int GetDidx(const vector<string>& doc_list, string name) {/*{{{*/
  size_t found;
  unsigned didx;
  for (didx = 0; didx < doc_list.size(); ++didx) {
    if ((found = doc_list[didx].find(name)) != string::npos) break;
  }
  if (didx < doc_list.size())
    return didx;
  else
    return -1;
}/*}}}*/



void ParseList(const char *filename,/*{{{*/
               string directory,
               vector<string> *list,
               QueryProfileList *profile_list) {
  /* Parse filename -> (profile_list and) list */
  FILE *fd = FOPEN(filename, "r");
  char buff[1024];
  while (fgets(buff, 1024, fd)) {
    int idx = -1;
    char *tok = strtok(buff," \t\n");
    if (profile_list != NULL) {
      int qid = atoi(tok);
      if ((idx = profile_list->Find(qid)) != -1) {
        fprintf(stderr, "Warning %s: Repeated query id %d for query"
                "instance, overwrite old one(s)\n",
                __FUNCTION__, qid);
      } else {
        profile_list->push_back(qid);
      }
      tok = strtok(NULL," \t\n");
    }
    /* Store name */
    string filename;
    if (directory.empty()) {
      filename = tok;
    } else {
      filename = directory + '/' + tok;
    }
    if (idx == -1) {
      list->push_back(filename);
    } else {
      (*list)[idx] = filename;
    }
  }
  fclose(fd);
}/*}}}*/

void ParseIgnore(const char* filename,/*{{{*/
                 vector<string>& doc_list,
                 QueryProfileList* profile_list) {
  FILE *fd = FOPEN(filename, "r");
  char buff[1024];
  /* Parse each line: qid doc_name */
  while (fgets(buff, 1024, fd)) {
    /* 1st field, qid */
    char *tok = strtok(buff, " \t\n");
    int qid = atoi(tok);
    int qidx = profile_list->Find(qid);
    if (qidx == -1) { // Unknown qid
      fprintf(stderr, "Warning %s: Query id %d unknown, ignore.\n",
              __FUNCTION__, qid);
      continue;
    }
    /* 2nd field, doc_name */
    string name = strtok(NULL," \t\n");
    int didx = GetDidx(doc_list, name);
    if (didx == -1) { // Unknown didx
      fprintf(stderr, "Warning %s: Doc %s not found, ignore.\n",
              __FUNCTION__, name.c_str());
    }
    if (qidx != -1 && didx != -1) profile_list->PushBackIgnore(qidx, didx);
  }
  fclose(fd);
  /* Sort ignore list */
  profile_list->SortIgnore();
}/*}}}*/



ostream& operator<<(ostream& os, const SnippetProfileList& snippet_list) {/*{{{*/
  os << "qidx didx nth_snippet score\n";
  for (unsigned i = 0; i < snippet_list.size(); ++i) {
    os << std::right
      << setw(4) << snippet_list.GetProfile(i).Qidx()
      << setw(5) << snippet_list.GetProfile(i).Didx()
      << setw(12) << snippet_list.GetProfile(i).NthSnippet()
      << ' ' << snippet_list.GetProfile(i).Score()
      << endl;
  }
  return os;
}/*}}}*/



AnswerList::AnswerList(const string& filename,/*{{{*/
                       const QueryProfileList& query_prof_list,
                       const vector<string>& doc_list) {
  Init(filename, query_prof_list, doc_list);
}/*}}}*/

void AnswerList::Init(const string& filename,/*{{{*/
                      const QueryProfileList& profile_list,
                      const vector<string>& doc_list) {
  is_answer.Resize(profile_list.size(), doc_list.size());
  is_answer.Memfill(false);


  FILE *fd = FOPEN(filename.c_str(), "r");
  char buff[1024];
  /* Parse each line: qid X doc_name X */
  while (fgets(buff, 1024, fd)) {
    /* 1st field, qid */
    char *tok = strtok(buff, " \t");
    int qid = atoi(tok);
    int qidx = profile_list.Find(qid);
    if (qidx == -1) { // Unknown qid
      fprintf(stderr, "Warning %s::%s: Query id %d unknown, ignore.\n",
              typeid(*this).name(), __FUNCTION__, qid);
      continue;
    }
    /* 2nd field, not used */
    tok = strtok(NULL, " \t");
    /* 3rd field, doc_name */
    string name = strtok(NULL," \t\n");
    int didx = GetDidx(doc_list, name);
    if (didx == -1) {// Unknown didx
      fprintf(stderr, "Warning %s::%s: Doc %s not found, ignore.\n",
              typeid(*this).name(), __FUNCTION__, name.c_str());
      continue;
    }
    is_answer[qidx][didx] = true;
  }
  fclose(fd);

}/*}}}*/

ostream& operator<<(ostream& os, const AnswerList& ans_list) {/*{{{*/
  for (unsigned qidx = 0; qidx < ans_list.NumQ(); ++qidx) {
    for (unsigned didx = 0; didx < ans_list.NumD(); ++didx) {
      if(ans_list.IsAnswer(qidx, didx))
        os << "(qidx = " << qidx << ", didx = " << didx << ")\n";
    }
  }
  return os;
}/*}}}*/


inline void _InitDispatcherRange(Dispatcher<UPair> *disp, /*{{{*/
                                 unsigned qidx,
                                 unsigned didx_start,
                                 unsigned didx_end) {
  for (unsigned didx = didx_start; didx < didx_end; ++didx) {
    disp->Push(UPair(qidx, didx));
  }
}/*}}}*/

void InitDispatcher(Dispatcher<UPair>* disp, /*{{{*/
                    const QueryProfileList& profile_list,
                    const vector<string>& doc_list) {

  disp->Clear();

  for (unsigned qidx = 0; qidx < profile_list.size(); ++ qidx) {
    const QueryProfile& query = profile_list.QP(qidx);
    unsigned didx_start = 0;
    /* For each ignore */
    for (unsigned i = 0; i < query.ignore.size(); ++i) {
      _InitDispatcherRange(disp, qidx, didx_start, query.ignore[i]);
      didx_start = query.ignore[i] + 1;
    }
    _InitDispatcherRange(disp, qidx, didx_start, doc_list.size());
  }
}/*}}}*/

void InitDispatcher(Dispatcher<UPair>* disp,/*{{{*/
                    const vector<SnippetProfileList>& snippet_lists) {
  disp->Clear();
  for (unsigned qidx = 0; qidx < snippet_lists.size(); ++qidx) {
    std::set<unsigned> docs;
    const SnippetProfileList& qidx_snippets = snippet_lists[qidx];
    for (unsigned i = 0; i < qidx_snippets.size(); ++i) // collect docs
      docs.insert(qidx_snippets.GetProfile(i).Didx());
    for (typeof(docs.begin()) itr = docs.begin(); itr != docs.end(); ++itr)
      disp->Push(UPair(qidx, *itr));
  }
}/*}}}*/


void _DumpDist(FILE* fp,/*{{{*/
               vector<vector<float> >& snippet_dist_qidx,
               int qid, unsigned didx_start, unsigned didx_end,
               const vector<string>& doc_list) {
  for (unsigned didx = didx_start; didx < didx_end; ++didx) {
    vector<float>& dist = snippet_dist_qidx[didx];
    if (dist.size() >= 1) {
      fprintf(fp,"%d %d %s %d %.6f %d\n",
              qid, 0, doc_list[didx].c_str(), 0, dist[0], didx);
    } else {
      cerr << "Warning " << __FUNCTION__
        << ": (qid, didx) = (" << qid << ", " << didx
        << ") not found.\n";
    }
  }
}/*}}}*/

void DumpResult(const char* fname,/*{{{*/
                const QueryProfileList& profile_list,
                QDArray<vector<float> >& snippet_dist,
                const vector<string>& doc_list) {
  FILE* fp = FOPEN(fname, "w");
  /* For each query id */
  for (unsigned qidx = 0; qidx < profile_list.size(); ++qidx) {
    const QueryProfile& query = profile_list.QP(qidx);
    unsigned didx_start = 0;
    /* For each ignore */
    for (unsigned i = 0; i < query.ignore.size(); ++i) {
      _DumpDist(fp, snippet_dist[qidx], query.qid,
                didx_start, query.ignore[i], doc_list);
      didx_start = query.ignore[i] + 1;
    }
    _DumpDist(fp, snippet_dist[qidx], query.qid,
              didx_start, doc_list.size(), doc_list);
  }
  fclose(fp);
}/*}}}*/

void DumpResult(FILE* fp,/*{{{*/
                int qid,
                const SnippetProfileList& snippet_list,
                const vector<string>& doc_list,
                const AnswerList* ans_list) {
  for (unsigned i = 0; i < snippet_list.size(); ++i) {
    const SnippetProfile& snippet = snippet_list.GetProfile(i);
    int qidx = snippet.Qidx();
    int didx = snippet.Didx();
    int answer = -(i + 1);
    if (ans_list != NULL && ans_list->IsAnswer(qidx, didx)) {
      answer = -answer;
    }
    fprintf(fp,"%d %d %s %d %.6f %d\n",
            qid, 0, doc_list[didx].c_str(), 0, snippet.Score(), answer);
  }
}/*}}}*/

void DumpResult(string filename,/*{{{*/
                const QueryProfileList& profile_list,
                const vector<SnippetProfileList>& snippet_lists,
                const vector<string>& doc_list,
                const AnswerList* ans_list) {
  FILE* fp = FOPEN(filename.c_str(), "w");
  /* For each query id */
  for (unsigned qidx = 0; qidx < profile_list.size(); ++qidx) {
    const QueryProfile& query = profile_list.QP(qidx);
    DumpResult(fp, query.qid, snippet_lists[qidx], doc_list, ans_list);
  }
  fclose(fp);
}/*}}}*/

} //namespace StdCommonUtil
