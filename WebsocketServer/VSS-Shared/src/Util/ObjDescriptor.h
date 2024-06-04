#pragma once

//#define USE_OBJECT_TYPE_INFO_IN_RELEASE
#if defined(_DEBUG) || defined(USE_OBJECT_TYPE_INFO_IN_RELEASE)
// Primary template for compile-time counter
template <int N>
struct Counter {
    static const int value = N;
};

// Partial specialization to increment the counter
template <int N>
struct IncrementCounter : Counter<N + 1> {};

// Define a macro to associate a type with a name and a compile-time incremented ID
#define REFLECT_TYPE(type) \
    static constexpr const char* type_name() { return #type; } \
    static constexpr int type_id() { return IncrementCounter<__COUNTER__>::value; } \
    const char *instance_type_name; \
    int instance_type_id; \
    inline int GetInstanceTypeId() { return instance_type_id; } \
    inline const char *GetInstanceTypeName() { return instance_type_name; } \
    void InitTypeInfo() \
    { \
        instance_type_name = type_name(); \
        instance_type_id = type_id(); \
    }

// only valid if structure class implementes the REFLECT_TYPE and type info gets initialized
template <typename T>
bool CheckObjectTypeCast(void* p)
{
    if (p == NULL)
    {
        return false;
    }
    T* castedP = (T*)p;
    return ((castedP->GetInstanceTypeId() == T::type_id()) && 
        (castedP->GetInstanceTypeName() == T::type_name()));
}

template <typename T>
T* typecheck_cast(void* p)
{
    if (CheckObjectTypeCast<T>(p))
    {
        return (T*)p;
    }
    return (T*)NULL;
}

#define typecheck_castL(pointertype, pointer) typecheck_cast<pointertype>(pointer); \
    if(pointer != NULL && CheckObjectTypeCast<pointertype>(pointer) == false){ \
	AddLogEntry(LogDestinationFlags::LDF_ALL, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceObjectCast, 0, 0, \
        "LogSourceObjectCast:Bad pointer cast. Was expecting %d, got %d. Expected type name %s. Maybe typename %s", \
        pointertype::type_id(), ((pointertype*)pointer)->GetInstanceTypeId(), pointertype::type_name(), ((pointertype*)pointer)->GetInstanceTypeName()); \
    int *crash = (int*)1; *crash = 1;}

#else
    #define REFLECT_TYPE(type) constexpr bool CheckObjectTypeCast() { return true; } \
        static constexpr int type_id() { return 0; } \
        constexpr int GetInstanceTypeId() { return 0; } \
        inline const char *GetInstanceTypeName() { return "";} \
        inline void InitTypeInfo() {}

        #define typecheck_castL(type,pointer) (type*)(pointer);
#endif
