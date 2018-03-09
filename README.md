# libutility
## General information

- **utility.h**: some common utilities (see file)
- **std_common.h**: handy utilities for Spoken Term Detection (STD) programs (see file)
- **thread_util.h**: handy utilities for pthread (described below)

## thread_util.h functions

Flow & structure at a glance
![alt text](https://github.com/chunan/libutility/img/thread_util.jpg "Flow & structure at a glance")


``void* ThreadEntry(void* arg)`` is the entry point for each thread when calling ``pthread_create(thread, NULL, ThreadEntry, arg)``.
It assume `arg` is a ThreadRunner (see below) type and call `.Run()`.
```cpp
void* ThreadEntry(void* arg) {                                                              
  ThreadRunner *objptr = static_cast<ThreadRunner *>(arg);                                  
  return objptr->Run();    
}
```

``class ThreadRunner`` is a worker interface loaded with necessary parameters
(``ThreadRunner::Run()`` does not accept argument).  You should create a
subclass to load the argument in it. A simple function can be converted into
its subclass by loading all the arguments in the constructor, then call
``Run()``. E.g.
```cpp
class ThreadRunner {
  public:
    ThreadRunner() {}
    virtual ~ThreadRunner() {}                                                              
    virtual void* Run() = 0;
};

class Greet : public ThreadRunner {
  public:
    Greet(const string &name) : guest_name_(name) {}
    void* Run() override { cout << "Hello " << guest_name_ << "!"; }
};
```

To cast multiple threads, we have a handy function --- ```CastThreads()```. Ideally we should take
a vector of ThreadEntry* but it forces the client to create a second vector to hold the list of
pointers to the runner. So we use template instead.
```cpp
template<class _Tp>              
void CastThreads(vector<_Tp>& runner) {
  if (runner.empty()) return;
  vector<pthread_t> threads(runner.size() - 1); // The last runner is used by the main thread
  for (unsigned t = 0; t < threads.size(); ++t) {
    pthread_create(&threads[t], NULL, ThreadEntry, &runner[t]);                             
  }
  runner.back().Run(); // main thread calls the last runner
  void* status;
  for (unsigned t = 0; t < threads.size(); ++t) {
    pthread_join(threads[t], &status);
  }
}
```

A common implementation is to have a centralized dispatcher that assigns tasks
to each runner when requested, which should be mutex-protected.  This is done
by ``class Dispatcher``. Dispatcher::GetObjPtr() will perform mutex locking (if
specified).
```cpp
template<class _Tp> // task ticket type
class Dispatcher {
  public:
    Dispatcher ();
    void Clear();
    _Tp* GetObjPtr(const bool lock = true); // Get a new task ticket (lock mutex)
    void Push(const _Tp &obj); // Add a new task ticket
    void Reset();
    unsigned count() const; // Number of distributed tasks
    unsigned size() const; // Total task tickets
    const _Tp& operator[](unsigned i) const; // Return i-th task ticket
};
```

For example, if I want to perform STD for N queries on M documents, yielding N
x M tasks, we can do it like:
```cpp
  Dispatcher<pair<int, int> > disp; // each ticket indicate the query index and document index
  for (int q = 0; q < M; ++q) {
    for (int d = 0; d < N; ++d) {
      disp.Push(make_pair(q, d));
    }
  }
```
A StdRunner (derived from ThreadRunner) will can fetch the task ticket within Run().
```cpp
class StdRunner : public ThreadRunner {
  virtual void* Run() {
    pair<int, int>* ticket;
    while ((ticket = disp.GetObjPtr()) != NULL) {
      DoStdWithQueryDocPair(ticket);
    }
  }
};
```
