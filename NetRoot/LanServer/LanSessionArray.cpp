
#include "LanSessionArray.h"

namespace server_baby
{


    LanSessionArray::LanSessionArray(const long interval)
        : stamp_(1), sessionIndexStack_(nullptr), lastTimeoutTime_(GetTickCount64()), timeoutInterval_(0)
    {
        timeoutInterval_ = interval;
        sessionIndexStack_ = new LockFreeStack<int>();

        Validate();
    }

    LanSessionArray::~LanSessionArray()
    {
        Invalidate();
    }

    void LanSessionArray::Timeout()
    {
        if (!timeoutInterval_)
            return;

        ULONGLONG nowTime = GetTickCount64();
        if (nowTime < (lastTimeoutTime_ + timeoutInterval_))
            return;

        ULONGLONG disconnectTime = nowTime - timeoutInterval_;
        for (int i = eINDEX_START; i < eLAN_SESSION_MAX_COUNT; i++)
        {
            LanSession* session = &sessionArray_[i];
            if (session->lastRecvTime_ <= disconnectTime && session->lastRecvTime_ != 0)
            {
                SystemLogger::GetInstance()->LogText(L"Timeout", LEVEL_DEBUG, L"Session ID : %d", session->GetTotalID().element_.unique_);
                session->Disconnect();
            }

        }

        lastTimeoutTime_ = nowTime;
        return;
    }

    void LanSessionArray::Validate()
    {
        for (int i = (eLAN_SESSION_MAX_COUNT - 1); i >= eINDEX_START; i--)
        {
            sessionIndexStack_->Push(i);
        }
    }

    void LanSessionArray::Invalidate()
    {
        for (int i = eINDEX_START; i < eLAN_SESSION_MAX_COUNT; i++)
        {
            LanSession* session = &sessionArray_[i];
            if(!session->isDead())
                session->Disconnect();
        }
    }
 
    LanSession* LanSessionArray::AllocSession(const SOCKET sock, SOCKADDR_IN* const addr)
    {
        // ���ǿ� �ε��� �� ID �Ҵ�
        int index = -1;
        if (!sessionIndexStack_->Pop(&index))
        {
            ErrorDisplay(L"CreateSession : Session MAX");
            return nullptr;
        }

        LanSessionID newID = MakeLanSessionID(index);
        sessionArray_[index].Initialize(sock, addr, newID);

        return &sessionArray_[index];
    }

}
