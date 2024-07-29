#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

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

struct StockBlackboard{
    float price;
    const string name;
    StockBlackboard(const string name, float price = 0): name(name), price(price){}
};

typedef MutexSafe<StockBlackboard> StockSafe;

void peterUpdateStockPrice(StockSafe& safe, condition_variable& cv)
{
  for(int i = 0; i < 5; i++)
  {
    {
      unique_lock<StockSafe> lock(safe);
      StockBlackboard& stock = safe.aquire(lock);
      if(i == 2)
      {
        stock.price = 100;
      }
      else
      {
        stock.price = abs(rand()) % 100;
      }
      cout<< "Peter updated stock price to " << stock.price << endl;
      if(stock.price > 90)
      {
        lock.unlock();  // unlock the mutex before notify
        cout<< "peter notify danny to sell the stock" << endl;
        cv.notify_one();
        this_thread::sleep_for(std::chrono::milliseconds(10));
        lock.lock();
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
  }
}

void DannyReadStock(StockSafe& safe, condition_variable& cv){
  {
      unique_lock<mutex> lock(safe.get_mutex());
      cout<< "Danny waiting for stock price" << endl;
      cv.wait(lock);  // unlock the mutex and go to sleep waiting for the notification
      StockBlackboard& stock = safe.aquire(lock);
      cout<< "Danny sell stock at price " << stock.price << endl;

  }
  std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));

}

int main(){
  StockSafe safe = StockSafe(new StockBlackboard("APPLE", 0));
  condition_variable cv;
  thread t1(peterUpdateStockPrice, ref(safe), ref(cv));
  thread t2(DannyReadStock, ref(safe), ref(cv));
  t1.join();
  t2.join();
  return 0;
}