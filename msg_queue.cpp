#include <thread>
#include <mutex>
#include <iostream>
#include <queue>
#include <condition_variable>

using namespace std;

struct StockPrice{
  public:
    string name;
    float price;
    StockPrice(string name, float price): name(name), price(price){}
};

template <typename MsgType>
class MsgQueue{
  private:
    queue<MsgType> _queue;
    mutex _m;
    condition_variable _enq_cv;
    condition_variable _deq_cv;
    int _limit;

  public:
    MsgQueue(int limit = 100): _limit(limit){}
    ~MsgQueue(){}

    void Enqueue(MsgType& msg){
      unique_lock<mutex> lock(_m);
      if(_queue.size() >= _limit){
        cout<< "Queue is full, waiting for dequeue" << endl;
        _enq_cv.wait(lock, [this](){return _queue.size() < _limit;});
      }
      _queue.push(msg);
      _deq_cv.notify_one();
    }

    MsgType Dequeue(){
      unique_lock<mutex> lock(_m);
      _deq_cv.wait(lock, [this](){return !_queue.empty();});
      MsgType msg = _queue.front();
      _queue.pop();
      _enq_cv.notify_one();
      return msg;
    }
};


typedef MsgQueue<StockPrice> StockMsgQType;

void StockPriceProducer(StockMsgQType& msgQueue)
{
  for(int i = 0; i < 10; i++)
  {
    {
      StockPrice stock("AAPL", abs(rand()%100));
      msgQueue.Enqueue(stock);
      cout<< "stock price: " << stock.price << " is enqueued" << endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
  }
  StockPrice endStock("AAPL", -1);
  msgQueue.Enqueue(endStock);
  cout<< "Announce market is closed" <<endl;
}

void DannyReadStock(StockMsgQType& msgQueue)
{
  while(true)
  {
    StockPrice stock = msgQueue.Dequeue();
    if(stock.price < 0)
    {
      cout<< "Market is closed, Danny exit" << endl;
      break;
    }
    cout<< "Danny read stock price: " << stock.price << endl;
  }
}

void TestStockMsgUpdate()
{
  MsgQueue<StockPrice> msgQ(100);
  thread StockPriceProducerThread(StockPriceProducer, ref(msgQ));
  thread DannyRead(DannyReadStock, ref(msgQ));

  StockPriceProducerThread.join();
  DannyRead.join();
}

int main()
{
  TestStockMsgUpdate();
  return 0;
}