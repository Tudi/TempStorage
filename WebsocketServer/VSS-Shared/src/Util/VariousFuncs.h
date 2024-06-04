#pragma once

#include <vector>
#include <iterator>
#include <stdexcept>
#include <type_traits>

#define ENUM_FLAG_OPERATORS(T)                                                                                                                                            \
    constexpr inline T operator~ (T a) { return static_cast<T>( ~static_cast<std::underlying_type<T>::type>(a) ); }                                                                       \
    constexpr inline T operator| (T a, T b) { return static_cast<T>( static_cast<std::underlying_type<T>::type>(a) | static_cast<std::underlying_type<T>::type>(b) ); }                   \
    constexpr inline T operator& (T a, T b) { return static_cast<T>( static_cast<std::underlying_type<T>::type>(a) & static_cast<std::underlying_type<T>::type>(b) ); }                   \
    constexpr inline T operator^ (T a, T b) { return static_cast<T>( static_cast<std::underlying_type<T>::type>(a) ^ static_cast<std::underlying_type<T>::type>(b) ); }                   \
    constexpr inline T& operator|= (T& a, T b) { return reinterpret_cast<T&>( reinterpret_cast<std::underlying_type<T>::type&>(a) |= static_cast<std::underlying_type<T>::type>(b) ); }   \
    constexpr inline T& operator&= (T& a, T b) { return reinterpret_cast<T&>( reinterpret_cast<std::underlying_type<T>::type&>(a) &= static_cast<std::underlying_type<T>::type>(b) ); }   \
    constexpr inline T& operator^= (T& a, T b) { return reinterpret_cast<T&>( reinterpret_cast<std::underlying_type<T>::type&>(a) ^= static_cast<std::underlying_type<T>::type>(b) ); }

#define GetNanoStamp() (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count())
__int64 GetNanoStampWindows();

// convert a string to base64encoded version of it
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length);
unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length);

// own implementation of CRC to be able to port it to other languages 
unsigned int crc31_hash(int seed, const void* data, size_t size);
unsigned __int64 crc64(unsigned __int64 crc, const void* buffer, unsigned __int64 size);
unsigned __int64 fnv1a_hash(unsigned __int64 hash, const void* data, size_t size);

void GetTaskbarPosAndSize(int& x, int& y, int& width, int& height);

template <typename T>
class CircularBuffer {
public:
    CircularBuffer() : buffer(), head(0), tail(0), full(false) {}

    inline void push(const T& item) {
        buffer[head] = item;
        if (full) {
            tail = (tail + 1) % buffer.size();
        }
        head = (head + 1) % buffer.size();
        full = (head == tail);
    }

    // not thread safe !
    inline T& push() {
        T& toret = buffer[head];
        if (full) {
            tail = (tail + 1) % buffer.size();
        }
        head = (head + 1) % buffer.size();
        full = (head == tail);
        return toret;
    }

    inline T pop() {
        if (empty()) {
            T item;
            memset(&item, 0, sizeof(T));
            return item;
        }

        full = false;
        T item = buffer[tail];
        tail = (tail + 1) % buffer.size();
        return item;
    }

    inline T& pop_ref() {
        full = false;
        T& toret = buffer[tail];
        tail = (tail + 1) % buffer.size();
        return toret;
    }

    inline bool isFull() const
    {
        return full;
    }

    inline bool empty() const {
        return (!full && (head == tail));
    }

    size_t size() const {
        if (full) {
            return buffer.size();
        }
        else if (head >= tail) {
            return head - tail;
        }
        else {
            return buffer.size() + head - tail;
        }
    }

    // Iterator support
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    iterator begin() {
        return buffer.begin() + tail;
    }

    iterator end() {
        return (full) ? buffer.end() : (buffer.begin() + head);
    }

    const_iterator cbegin() const {
        return buffer.cbegin() + tail;
    }

    const_iterator cend() const {
        return (full) ? buffer.cend() : (buffer.cbegin() + head);
    }

    // Indexing support
    T& operator[](size_t index) {
        if (index >= size()) {
            throw std::out_of_range("Index out of range");
        }
        return buffer[(tail + index) % buffer.size()];
    }

    const T& operator[](size_t index) const {
        if (index >= size()) {
            throw std::out_of_range("Index out of range");
        }
        return buffer[(tail + index) % buffer.size()];
    }

    void clear() {
        head = 0;
        tail = 0;
        full = false;
//        buffer.clear();
    }

    void reserve(size_t size){
        if (size > buffer.size()) {
            full = false;
        }
        buffer.resize(size);
    }
private:
    std::vector<T> buffer;
    size_t head;
    size_t tail;
    bool full;
};

#ifndef MIN
    #define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
    #define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef M_PI
    #define M_PI 3.14159265f
#endif

// for a console application, make the console window width larger
void AutoSizeConsoleWindow(int ApplicationIndex = 0, int ExpectedApplications = 1);

#define EnumHasAnyFlag(EVal,Mask) (((unsigned __int64)EVal & ((unsigned __int64)Mask)) != (unsigned __int64)0)

#define EnumHasAllFlags(EVal,Mask) (((unsigned __int64)EVal & ((unsigned __int64)Mask)) == (unsigned __int64)Mask)

#define EnumSetFlags(EVal,Mask) ((unsigned __int64)EVal | (unsigned __int64)Mask)

#define EnumClearFlags(EVal,Mask) ((unsigned __int64)EVal & (~(unsigned __int64)Mask))