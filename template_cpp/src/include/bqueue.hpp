#pragma once
#include <iostream>
#include<queue>
#include <mutex>
#include <condition_variable>
// https://cppcodetips.wordpress.com/2018/08/31/thread-pool-class-using-c-threading/
template <class T>
class BlockingQueue
{
public:
    BlockingQueue() { ; }
 
    void push(const T & data)
    {
        std::unique_lock<std::mutex> lock(m_mtxData);
        m_Data.push(data);
        m_Cv.notify_one();
    }
 
    void pop()
    {
        std::unique_lock<std::mutex> lock(m_mtxData);
        if(!m_Data.empty())
            m_Data.pop();
    }
 
    T& front()
    {
        std::unique_lock<std::mutex> lock(m_mtxData);
        m_Cv.wait(lock, [&] { return  !m_Data.empty(); });
        T& data = m_Data.front();
        return data;
    }
 
    T& back()
    {
        std::unique_lock<std::mutex> lock(m_mtxData);
        m_Cv.wait(lock, [&] { return  !m_Data.empty(); });
        T& data = m_Data.back();
        return data;
    }
 
 
    bool empty()
    {
        return m_Data.empty();
    }
 
    size_t size()
    {
        return m_Data.size();
    }
 
private:
    std::queue <T> m_Data;
    std::condition_variable m_Cv;
    std::mutex m_mtxData;
};