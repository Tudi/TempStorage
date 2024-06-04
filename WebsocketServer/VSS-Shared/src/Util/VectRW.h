#pragma once

#include <mutex>
#include <vector>
#include <condition_variable>
#include <assert.h>

template <typename T>
class RWLockedContainer 
{
public:
    RWLockedContainer() : readCount(0), writeCount(0) {}
	
    void ReadLock() 
	{
        std::unique_lock<std::mutex> lock(mutex_);
        readCondition.wait(lock, [this] { return writeCount == 0; });
        ++readCount;
    }

    void ReadUnlock() 
	{
        std::unique_lock<std::mutex> lock(mutex_);
        assert(readCount >= 1);
        --readCount;
        if (readCount == 0) {
            writeCondition.notify_one();
        }
    }

    void WriteLock() 
	{
        std::unique_lock<std::mutex> lock(mutex_);
        ++writeCount;
        writeCondition.wait(lock, [this] { return readCount == 0 && writeCount == 1; });
    }

    void WriteUnlock() 
	{
        std::unique_lock<std::mutex> lock(mutex_);
        assert(writeCount >= 1);
        --writeCount;
        readCondition.notify_all();
        writeCondition.notify_all();
    }

    T& GetVect() { return m_dataContainer; }
private:
    T m_dataContainer;
	
    std::mutex mutex_;
    std::condition_variable readCondition;
    std::condition_variable writeCondition;
    size_t readCount;
    size_t writeCount;	
};

#define MAX_EXPECTED_CONCURENT_THREADS 100

template <typename T>
class RWLockedVector
{
public:
    RWLockedVector() : readCount(0), writeCount(0) {}

    void ReadLock()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        readCondition.wait(lock, [this] { return writeCount == 0; });
        assert(readCount < MAX_EXPECTED_CONCURENT_THREADS);
        ++readCount;
    }

    void ReadUnlock()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        assert(readCount >= 1);
        --readCount;
        if (readCount == 0) {
            writeCondition.notify_one();
        }
    }

    void WriteLock()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        assert(writeCount < MAX_EXPECTED_CONCURENT_THREADS);
        ++writeCount;
        writeCondition.wait(lock, [this] { return readCount == 0 && writeCount == 1; });
    }

    void WriteUnlock()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        assert(writeCount >= 1);
        --writeCount;
        readCondition.notify_all();
        writeCondition.notify_all();
    }

    std::vector<T>& GetVect() { return m_dataVector; }
private:
    std::vector<T> m_dataVector;

    std::mutex mutex_;
    std::condition_variable readCondition;
    std::condition_variable writeCondition;
    size_t readCount;
    size_t writeCount;
};
