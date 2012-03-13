#include <cstring>
#include "ugoc_utility.h"
#include "std_common.h"



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

