#pragma once

#include <synchapi.h>
using namespace std;

namespace server_baby
{
    class CrashDump;
    class SystemLogger;

    template <class DATA>
    class QueueWithLock
    {
        enum QueueSetting
        {
            QUEUE_MAX_SIZE = 1000
        };

        DATA arr_[QUEUE_MAX_SIZE] = { 0 };         // Queue 요소를 저장할 어레이
        int front_;      // 앞은 Queue의 앞 요소를 가리킵니다(있는 경우).
        int rear_;       // 후면은 Queue의 마지막 요소를 가리킵니다.
        int count_;  // Queue의 현재 크기
        RTL_SRWLOCK lock_;

    public:
        explicit QueueWithLock() : front_(0), rear_(0), count_(0) {}

        ~QueueWithLock() {}

        void Initialize()
        {
            InitializeSRWLock(&lock_);
            front_ = 0;
            rear_ = 0;
            count_ = 0;
        }

        bool Dequeue(DATA* buf)
        {
            Lock();
            if (isEmpty())
            {
                Unlock();
                ErrorDisplay(L"Array Queue Empty");
                return false;
            }

            count_--;
            *buf = arr_[front_];
            front_ = (front_ + 1) % QUEUE_MAX_SIZE;
            Unlock();
            return true;
        }

        bool Enqueue(DATA item)
        {
            Lock();
            if (isFull())
            {
                Unlock();
                ErrorDisplay(L"Array Queue Full");
                return false;
            }

            arr_[rear_] = item;
            rear_ = (rear_ + 1) % QUEUE_MAX_SIZE;
            count_++;
            Unlock();
            return true;
        }

        int Size()
        {
            return count_;
        }


    private:
        void Lock()
        {
            AcquireSRWLockExclusive(&lock_);
        }

        void Unlock()
        {
            ReleaseSRWLockExclusive(&lock_);
        }

        bool isEmpty()
        {
            return (rear_ == front_);
        }

        bool isFull()
        {
            return ((rear_ + 1) % QUEUE_MAX_SIZE == front_);
        }

        void ErrorQuit(const WCHAR* message)
        {
            SystemLogger::GetInstance()->LogText(L"QueueWithLock", 2, message);
            CrashDump::Crash();
        }

        void ErrorDisplay(const WCHAR* message)
        {
            SystemLogger::GetInstance()->LogText(L"QueueWithLock", 2, message);
        }

    };
}
