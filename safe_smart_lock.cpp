// Online C++ compiler to run C++ program online
#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

// this class is reusable
template <typename T>
class MutexSafe{
  private:
    T* data;
    mutex _m;

  public:
    MutexSafe(T* data): data(data){}
    ~MutexSafe(){
      delete data;
    }

    void lock(){
      _m.lock();
    }

    void unlock(){
      _m.unlock();
    }

    bool try_lock(){
      return _m.try_lock();
    }

    mutex& get_mutex(){
      return _m;
    }


    template <class safeT>
    T& aquire(unique_lock<safeT>& lock){  // unique_lock的类型可以是一个template class
      return *data;
    }
};

typedef MutexSafe<int> Safe;

void add(Safe& safe)
{
    unique_lock<Safe> lock(safe); // Safe is to be used as a mutex
    int & var = safe.aquire(lock);
    for(int i = 0;i < 10000000; i++){
        var++;
    }
}

void add_(Safe& safe){
    unique_lock<Safe> lock(safe);
    int & var = safe.aquire(lock);
    for(int i = 0;i < 10000000; i++){
        var++;
    }
}

int main(){

    Safe safe = Safe(new int(0));
    thread t1(add, ref(safe));
    thread t2(add, ref(safe));
    t1.join();
    t2.join(); // wait until the thread finishes

    unique_lock<Safe> lock(safe);
    int var = safe.aquire(lock);
    cout << var<<endl;
    return 0;
}

