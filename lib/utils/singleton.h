#ifndef SINGLETON_H_
#define SINGLETON_H_ 1

#include <aquabotics_defines.h>

#include <cassert>
#include <iostream>

using namespace std;

AQUABOTICS_BEGIN_NAMESPACE

template <typename T>
class Singleton
{
private:
    static T *m_Instance;

public:
    Singleton()
    {
        assert(m_Instance == nullptr);
        m_Instance = static_cast<T *>(this);
    }
    virtual ~Singleton()
    {
        m_Instance = nullptr;
    }
    static T &GetSingleton()
    {
        return *m_Instance;
    }
    static T *GetSingletonPtr()
    {
        return m_Instance;
    }
};

template <typename T>
T *Singleton<T>::m_Instance = nullptr;

AQUABOTICS_END_NAMESPACE

#endif