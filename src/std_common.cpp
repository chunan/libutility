#include <cstring>
#include "ugoc_utility.h"
#include "std_common.h"
#include "thread_util.h"


using std::cerr;

void ParseList(const char *filename,/*{{{*/
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
        fprintf(stderr, "Warning: Repeated query id %d for query"
                "instance, overwrite old one(s)\n", qid);
      } else {
        profile_list->push_back(qid);
      }
      tok = strtok(NULL," \t\n");
    }
    /* Store name */
    if (idx == -1) {
      list->push_back(tok);
    } else {
      (*list)[idx] = tok;
    }
  }
  fclose(fd);
}/*}}}*/

void ParseIgnore(const char* filename,/*{{{*/
                 vector<string>& D_list,
                 QueryProfileList* profile_list) {
  FILE *fd = FOPEN(filename, "r");
  char buff[1024];
  string name;
  /* Parse each line: qid filename */
  while (fgets(buff, 1024, fd)) {
    char *tok = strtok(buff," \t\n");
    int qid = atoi(tok);
    int qidx = profile_list->Find(qid);
    /* Unknown qid */
    if (qidx == -1) {
      fprintf(stderr, "Warning: Query id %d unknown, ignore.\n", qid);
      continue;
    }
    /* Search for didx */
    name = strtok(NULL," \t\n");
    size_t found;
    unsigned didx;
    for (didx = 0; didx < D_list.size(); ++didx) {
      if ((found = D_list[didx].find(name)) != string::npos) {
        profile_list->PushBackIgnore(qidx, didx);
        break;
      }
    }
    /* Unknown didx */
    if (didx == D_list.size()) {
      fprintf(stderr, "Warning: Doc %s not found, ignore.\n", name.c_str());
    }
  }
  fclose(fd);
  /* Sort ignore list */
  profile_list->SortIgnore();
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
                    const vector<string>& D_list) {

  disp->Clear();

  for (unsigned qidx = 0; qidx < profile_list.size(); ++ qidx) {
    const QueryProfile& query = profile_list.QP(qidx);
    unsigned didx_start = 0;
    /* For each ignore */
    for (unsigned i = 0; i < query.ignore.size(); ++i) {
      _InitDispatcherRange(disp, qidx, didx_start, query.ignore[i]);
      didx_start = query.ignore[i] + 1;
    }
    _InitDispatcherRange(disp, qidx, didx_start, D_list.size());
  }
}/*}}}*/



void _DumpDist(FILE* fp,/*{{{*/
               vector<vector<float> >& snippet_dist_qidx,
               int qid, unsigned didx_start, unsigned didx_end,
               const vector<string>& D_list) {
  for (unsigned didx = didx_start; didx < didx_end; ++didx) {
    vector<float>& dist = snippet_dist_qidx[didx];
    if (dist.size() >= 1) {
      fprintf(fp,"%d %d %s %d %.6f %d\n",
              qid, 0, D_list[didx].c_str(), 0, -dist[0], didx);
    } else {
      cerr << "Warning: (qid, didx) = (" << qid << ", " << didx
        << ") not found.\n";
    }
  }
}/*}}}*/

void DumpResult(const char* fname,/*{{{*/
                const QueryProfileList& profile_list,
                QDArray<vector<float> >& snippet_dist,
                const vector<string>& D_list) {
  FILE* fp = FOPEN(fname, "w");
  /* For each query id */
  for (unsigned qidx = 0; qidx < profile_list.size(); ++qidx) {
    const QueryProfile& query = profile_list.QP(qidx);
    unsigned didx_start = 0;
    /* For each ignore */
    for (unsigned i = 0; i < query.ignore.size(); ++i) {
      _DumpDist(fp, snippet_dist[qidx], query.qid,
                didx_start, query.ignore[i], D_list);
      didx_start = query.ignore[i] + 1;
    }
    _DumpDist(fp, snippet_dist[qidx], query.qid,
              didx_start, D_list.size(), D_list);
  }
  fclose(fp);
}/*}}}*/
