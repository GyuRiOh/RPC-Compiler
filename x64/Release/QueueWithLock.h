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

        DATA arr_[QUEUE_MAX_SIZE] = { 0 };         // Queue ��Ҹ� ������ ���
        int front_;      // ���� Queue�� �� ��Ҹ� ����ŵ�ϴ�(�ִ� ���).
        int rear_;       // �ĸ��� Queue�� ������ ��Ҹ� ����ŵ�ϴ�.
        int count_;  // Queue�� ���� ũ��
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
