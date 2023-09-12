
#ifndef __QUEUE__
#define __QUEUE__

#include "Crash.h"
#include "SystemLogger.h"
using namespace std;

namespace server_baby
{
    template <class DATA>
    class Queue
    {
        enum QueueSetting
        {
            QUEUE_MAX_SIZE_MASK = 511,
            QUEUE_MAX_SIZE = 512
        };

        DATA arr_[QUEUE_MAX_SIZE] = { 0 };         // Queue ��Ҹ� ������ ���
        LONG count_;      // Queue�� ���� ũ��
        unsigned short front_;      // ���� Queue�� �� ��Ҹ� ����ŵ�ϴ�(�ִ� ���).
        unsigned short rear_;       // �ĸ��� Queue�� ������ ��Ҹ� ����ŵ�ϴ�.

    public:
        explicit Queue() : front_(0), rear_(0), count_(0) {}
        ~Queue() = default;

        void Clear()
        {
            front_ = 0;
            rear_ = 0;
            count_ = 0;
        }

        bool Dequeue(DATA* buf)
        {
            // Queue ����÷� Ȯ��
            if (isEmpty())
                return false;

            count_--;
            front_ = (front_ + 1) & QUEUE_MAX_SIZE_MASK;
            *buf = arr_[front_];

            return true;
        }

        bool Enqueue(DATA item)
        {
            // Queue �����÷� Ȯ��
            if (isFull())
            {
                ErrorQuit(L"Array Queue Full");
                return false;
            }

            rear_ = (rear_ + 1) & QUEUE_MAX_SIZE_MASK;
            arr_[rear_] = item;
            count_++;

            return true;
        }

        int Size()
        {
            return count_;
        }

    private:
        bool isEmpty()
        {
            return (count_ == 0);
        }

        bool isFull()
        {
            return (count_ >= QUEUE_MAX_SIZE_MASK);
        }

        void ErrorQuit(const WCHAR* message)
        {
            SystemLogger::GetInstance()->LogText(L"Queue", LEVEL_SYSTEM, message);
            CrashDump::Crash();
        }

        void ErrorDisplay(const WCHAR* message)
        {
            SystemLogger::GetInstance()->LogText(L"Queue", LEVEL_DEBUG, message);
        }
    };

    template <class DATA>
    class QueueWithoutCount
    {
        enum QueueSetting
        {
            QUEUE_MAX_SIZE_MASK = 1023,
            QUEUE_MAX_SIZE = 1024
        };

        DATA arr_[QUEUE_MAX_SIZE] = { 0 };    
        unsigned short front_;    
        unsigned short rear_;     

    public:    
        explicit QueueWithoutCount() : front_(0), rear_(0) {}
        ~QueueWithoutCount() = default;

        void Clear()
        {
            front_ = 0;
            rear_ = 0;
        }

        bool Dequeue(DATA* buf)
        {
            if (isEmpty())
                return false;

            *buf = arr_[front_];
            arr_[front_] = NULL;
            InterlockedExchange16((volatile SHORT*)&front_, (front_ + 1) & QUEUE_MAX_SIZE_MASK);

            return true;
        }

        bool Enqueue(DATA item)
        {
            if (isFull())
            {
                ErrorQuit(L"Array Queue Full");
                return false;
            }

            arr_[rear_] = item;
            InterlockedExchange16((volatile SHORT*)&rear_, (rear_ + 1) & QUEUE_MAX_SIZE_MASK);

            return true;
        }

        bool isEmpty()
        {
            unsigned short localFront = front_;
            unsigned short localRear = rear_;
            return (localFront == localRear);
        }

        bool isFull()
        {
            unsigned short localFront = front_;
            unsigned short localRear = rear_;
            return (((localRear + 1) & QUEUE_MAX_SIZE_MASK) == localFront);
        }

    private:
        void ErrorQuit(const WCHAR* message)
        {
            SystemLogger::GetInstance()->LogText(L"Queue", LEVEL_SYSTEM, message);
            CrashDump::Crash();
        }

        void ErrorDisplay(const WCHAR* message)
        {
            SystemLogger::GetInstance()->LogText(L"Queue", LEVEL_DEBUG, message);
        }
    };

}

#endif

