#include <iostream>
#include <limits>
#include <memory>
#include <sched.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <queue>
#include <random>
#include <optional>

using namespace std::chrono_literals;

const auto c_tick = 2000ms;
// final product should use a config loaded value for this
const size_t g_max_queuable_items = 0xFFFF;

class Item
{
public:
  static constexpr int invalid = std::numeric_limits<int>::max();

  Item(){}
  Item(int value)
    : m_value(value)
  {
    // nothing to do
  }

  int m_value{0};
};

class Reporter
{
public:
  // business logic is fine here. it just logs added data
  void report(const std::string &data)
  {
    m_mantiConcurency.lock(); 
    std::cout << data << std::endl;
    m_mantiConcurency.unlock();
  }
  std::mutex m_mantiConcurency;
};

class Data
{
public:
  Data(){

  }

  bool add(const Item &item) {
    const std::lock_guard<std::mutex> lock(m_mantiConcurency);

    if ( m_items.size() >= g_max_queuable_items ) {
      return false;
    
    }
    m_items.push(item);
    ++m_items_produced;

    return true;
  }

  std::optional<Item> get() {
    m_mantiConcurency.lock(); 
    if(m_items.empty()){
      m_mantiConcurency.unlock();
      return std::nullopt;
    }
    Item item = m_items.front();
    m_items.pop();
    m_mantiConcurency.unlock();
    return item;
  }

  std::size_t get_produced_items() const
  {
    return m_items_produced;
  }

  std::size_t m_items_produced{0};
  std::queue<Item> m_items;
  std::mutex m_mantiConcurency;
};

class Producer
{
public:
  Producer(Data *data, Reporter *reporter)
  {
    m_pData = data;
    m_pReporter = reporter;
    // nothing to do
  }

  void run()
  {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1, 10);

    while (m_running) {
      std::this_thread::sleep_for(c_tick);
      int dTmpRdNum = dist(rd);
      Item item(dTmpRdNum);
      m_pReporter->report("produced " + std::to_string(item.m_value));
      m_pData->add(item);
    }
    m_pReporter->report("producer stopping");
  }

  void stop()
  {
    m_running = false;
  }

  Data *m_pData;
  Reporter *m_pReporter;
  bool m_running{true};
};

class Consumer
{
public:
  Consumer(Data *data, Reporter *reporter) 
    {
    m_pReporter = reporter;
    // nothing to do
    m_pData = data;
  }

  void run()
  {
    while (m_running) {
      std::this_thread::sleep_for(c_tick);
      std::optional<Item> item = m_pData->get();
      if( item.has_value() ){
        m_pReporter->report("consumed " + std::to_string(item.value().m_value));
      }
    }
    m_pReporter->report("consumer stopping");
  }

  void stop()
  {
    m_running = false;
  }

  Data *m_pData;
  Reporter *m_pReporter;
  bool m_running{true};
};

class Scheduler
{
public:
// constructor
  Scheduler(Data *data, std::vector<Consumer *> *pvConsumers, std::vector<Producer *> *pvProducers, Reporter *pReporter)
  {
    m_pReporter = pReporter;
    m_pvConsumers = pvConsumers;
    m_pvProducers = pvProducers;
    // nothing to do
    m_pData = data;
  }
// the actual thread func
  void run()
  {
    while(m_running)
    {
      std::this_thread::sleep_for(c_tick);
      // as exected program exists if queue is too large
      if (m_pData->get_produced_items() > 10) {
        m_running = false;
        m_pReporter->report("scheduler stopping");
        break;
      }
      // this is the actual code that periodcally runs
      m_pReporter->report("scheduler continue");
    }

    for(auto itr = m_pvConsumers->begin(); itr != m_pvConsumers->end(); itr++)
      (*itr)->stop();

    for(auto itr = m_pvProducers->begin(); itr != m_pvProducers->end(); itr++)
      (*itr)->stop();
  }

// member variables
  Data *m_pData;
  Reporter *m_pReporter;
  std::vector<Producer *> *m_pvProducers;
  std::vector<Consumer *> *m_pvConsumers;
  bool m_running{true};
};

int main() {
  static const std::size_t n_consumers = 100;
  static const std::size_t n_producers = 100;
  
  Data *pData = new Data();
  if( pData == NULL)
  {
    return -1;
  }

  Reporter *pReporter = new Reporter();
  if( pReporter == NULL)
  {
    return -1;
  }
  
  // store all the consumers and producers
  std::vector<Producer *> vProducers;
  std::vector<Consumer *> vConsumers;

  // manage our producers and consumers
  Scheduler scheduler(pData, &vConsumers, &vProducers, pReporter);

  // set up first the consumers

  std::thread tConsumers[n_consumers];
  for( size_t index=0; index<n_consumers; index++){
    Consumer *pConsumer = new Consumer(pData, pReporter);
    if( pConsumer == NULL)
    {
      return -1;
    }
    vConsumers.push_back(pConsumer);
    tConsumers[index] = std::thread(&Consumer::run, pConsumer);
  }

  // after that set up the producers
  std::thread tProduers[n_producers];  
  for( size_t index=0; index<n_producers; index++){
    Producer *pProducer = new Producer(pData, pReporter);
    if( pProducer == NULL)
    {
      return -1;
    }
    vProducers.push_back(pProducer);
    tProduers[index] = std::thread(&Producer::run, pProducer);
  }

  // scheduler to watch producers and consumers
  std::thread t2(&Scheduler::run, &scheduler);
 
  // in theory these are all stopping right now
  for( size_t index=0; index<n_producers; index++){
    tProduers[index].join();
  }

  // in theory these are all stopping right now
  for( size_t index=0; index<n_consumers; index++){
    tConsumers[index].join();
  }

  // as a last thing we shut down our scheduler
  t2.join();

  // !! todo : instead of manually handle objet liffetime add auto poitners
  for( size_t index=0; index<n_producers; index++){
      delete vProducers[index];
  }
  for( size_t index=0; index<n_consumers; index++){
      delete vConsumers[index];
  }
  delete pData;
  delete pReporter;

  return 0;
}
