
#ifndef __NET__SESSION__
#define __NET__SESSION__

#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Winmm.lib")
#include <WinSock2.h>
#include <BaseTsd.h>
#include <timeapi.h>
#include "../Common/LockFreeEnqQueue.h"
#include "NetLargePacket.h"
#include "../Common/SystemLogger.h"
#include "NetSessionID.h"
#include "../Common/Queue.h"
#include "NetEnums.h"
#include "../Common/QueueWithLock.h"

using namespace std;

namespace server_baby
{
    class NetPacketSet;
    class NetPacket;
    class NetPipe;
    constexpr unsigned short DELETE_FLAG_MASK = 0x7FFF;
    constexpr unsigned short DELETE_FLAG_BIT = 0x8000;

    enum ErrorCode
    {
        NET_PACKET_ERROR = -1,
        SUCCESS = 1,
        SUCCESS_ZERO_PACKET_COUNT = 2,
        SUCCESS_IO_PENDING = 3,
        FAILURE_RECV_PACKET_FULL = 4,
        FAILURE_PACKET_ERROR = 5,
        SEND_FAILURE_NOT_OKAY_TO_SEND = 6,
        SEND_FAILURE_SENDQ_EMPTY = 7, //�����ڵ� ǥ �ٽ� �����
        FAILURE_IN_IO_REQUEST = 8,
        FAILURE_IN_IO_CANCEL_FLAG = 9,
    };

    //===============================
    //���� ���� ������ ���� Ŭ����
    //===============================
    class NetSession final
    {
        enum Setting
        {
            PACKET_ATTACK = -1,
            SEND_Q_CODE = 0x1004,
        };

    public:
        explicit NetSession();
        ~NetSession();

        char RecvPost();       
        char CompleteRecvCheck_PacketQ(NetPacketSet** const packetQBuf, int* const packetCount, const DWORD transferred);
        void SaveOverlappedError(const int transferred, const int errorCode, const LPOVERLAPPED overlapped);

        char SendPost(long* const oldSendCount);
        char SendAndDisconnectPost(long* const oldSendCount);
        bool EnqueueSendQ(NetPacket* const packet);
        void SetSendFlagFalse();

        bool isIOCanceled() const;
        bool isDead() const;
        bool isRecvOverlapped(const LPOVERLAPPED overlapped) const;
        bool isSendOverlapped(const LPOVERLAPPED overlapped) const;

        void GetOverlappedResult(const int transferred, LPOVERLAPPED overlapped);

        short GetIOCount() const;
        NetSessionID GetTotalID() const;
        short GetSendCount() const;
        LPOVERLAPPED GetRecvOverlapped();
        LPOVERLAPPED GetSendOverlapped();

        unsigned short AddIOCount(const unsigned short value);
        unsigned short SubIOCount(const unsigned short value);
        unsigned short IncrementIOCount();
        unsigned short DecrementIOCount();

        void Disconnect();
        bool SetDeleteFlag();
        bool isSending();
        bool AcquireSession(const NetSessionID ID);

        void ClearJobQ();

        static long GetSendQPoolCount();
        static long GetSendQPoolCapacity();
        static void DeleteSendQPoolTLS();
        static void GarbageCollectSendQPool();

        bool Destroy();

    private:
        bool Initialize(const SOCKET sock, SOCKADDR_IN* const addr, const NetSessionID ID);

        void FreeSentPacket();
        bool isSendFlagFalse();
        bool DequeueSendQ(NetPacket** const packet);

        void CancelIO() const;

        void ErrorCheck(const errno_t err, const WCHAR* const msg);
        void ErrorQuit(const WCHAR* const msg);
        void ErrorDisplay(const WCHAR* const msg); 

    public:
        QueueWithoutCount<NetPacketSet*>* jobQ_;
        NetPipe* curPipe_;
        NetPipe* destPipe_;

    private:
        //====================================
        // ���� �Ϸ� ť (WSASend�� �ְ�, ���� ����ȭ���ۿ��� Packet ����)
        //====================================
        bool isSending_;
        WSABUF sentQ_[eNET_PACKET_SEND_Q_MAX_SIZE] = { 0 };
        unsigned short IOcount_;

        //====================================
        // ������ ����ü
        // Each size : 32
        // Total Size : 32
        WSAOVERLAPPED recvOverlapped_;
        WSAOVERLAPPED sendOverlapped_;
        //====================================

        LockFreeEnqQueue<NetPacket*, SEND_Q_CODE, eNET_PACKET_SEND_Q_MAX_SIZE> sendQ_;
        NetLargePacket* recvPacket_;
        ULONGLONG lastRecvTime_;
        short sentCount_;
        bool isIOCanceled_;
        
        alignas(64) SOCKADDR_IN clientAddr_;
        NetSessionID ID_;
        SOCKET sock_;


        friend class NetSessionArray;
    };

    inline short NetSession::GetIOCount() const
    {
        return IOcount_;
    }

    inline NetSessionID NetSession::GetTotalID() const
    {
        return ID_;
    }

    inline short NetSession::GetSendCount() const
    {
        return sentCount_;
    }

    inline LPOVERLAPPED NetSession::GetRecvOverlapped()
    {
        ZeroMemory(&recvOverlapped_, sizeof(recvOverlapped_));
        return &recvOverlapped_;
    }

    inline LPOVERLAPPED NetSession::GetSendOverlapped()
    {
        ZeroMemory(&sendOverlapped_, sizeof(sendOverlapped_));
        return &sendOverlapped_;
    }

    inline bool NetSession::isIOCanceled() const
    {
        return isIOCanceled_;
    }

    inline bool NetSession::isDead() const
    {
        return ((short)IOcount_ < 0);
    }

    inline bool NetSession::isRecvOverlapped(const LPOVERLAPPED overlapped) const
    {
        return (&recvOverlapped_ == overlapped);
    }

    inline bool NetSession::isSendOverlapped(const LPOVERLAPPED overlapped) const
    {
        return (&sendOverlapped_ == overlapped);
    }

    inline void NetSession::GetOverlappedResult(const int transferred, LPOVERLAPPED overlapped)
    {
        DWORD transferredBuf = transferred;
        DWORD flags = 0;
        bool retval = WSAGetOverlappedResult(sock_, overlapped, &transferredBuf, false, &flags);
        if (!retval)
            ErrorCheck(WSAGetLastError(), L"GetOverlappedResult");
    }

    inline void NetSession::SetSendFlagFalse()
    {
        isSending_ = false;
    }

    inline long server_baby::NetSession::GetSendQPoolCount()
    {
        return LockFreeEnqQueue<NetPacket*, SEND_Q_CODE, eNET_PACKET_SEND_Q_MAX_SIZE>::GetPoolCount();
    }

    inline long server_baby::NetSession::GetSendQPoolCapacity()
    {
        return LockFreeEnqQueue<NetPacket*, SEND_Q_CODE, eNET_PACKET_SEND_Q_MAX_SIZE>::GetPoolCapacity();
    }

    inline void server_baby::NetSession::DeleteSendQPoolTLS()
    {
        LockFreeEnqQueue<NetPacket*, SEND_Q_CODE, eNET_PACKET_SEND_Q_MAX_SIZE>::DeleteTLS();
    }

    inline void NetSession::GarbageCollectSendQPool()
    {
        LockFreeEnqQueue<NetPacket*, SEND_Q_CODE, eNET_PACKET_SEND_Q_MAX_SIZE>::GarbageCollect();
    }

    inline bool NetSession::EnqueueSendQ(NetPacket* const packet)
    {
        if (isIOCanceled())
            return false;

        bool ret = sendQ_.Enqueue(packet);
        if (!ret)
            SystemLogger::GetInstance()->LogText(L"SendQ", LEVEL_DEBUG, L"SendQ Q Enq Failed - FULL");
        return ret;
    }

    inline void NetSession::Disconnect()
    {
        isIOCanceled_ = true;
        CancelIO();
    }

    inline void server_baby::NetSession::CancelIO() const
    {
        CancelIoEx((HANDLE)sock_, NULL);
    }


    inline bool NetSession::AcquireSession(const NetSessionID oldID)
    {
        //DeleteFlag�� �����ִ� �������� Ȯ��
        if (isDead())
            return false;

        //���� �Ǿ����� Ȯ��
        if (ID_.element_.unique_ != oldID.element_.unique_)
            return false;

        return true;
    }

    inline unsigned short NetSession::AddIOCount(const unsigned short value)
    {
        return static_cast<unsigned short>(InterlockedAdd((LONG*)&IOcount_, value));
    }

    inline unsigned short NetSession::SubIOCount(const unsigned short value)
    {
        return static_cast<unsigned short>(InterlockedAdd((LONG*)&IOcount_, value));
    }

    inline bool NetSession::DequeueSendQ(NetPacket** const packet)
    {
        if (!sendQ_.Dequeue(packet))
        {
            ErrorDisplay(L"SendQ Deq Failed");
            return false;
        }

        return true;
    }

    inline unsigned short NetSession::IncrementIOCount()
    {
        return InterlockedIncrement16((SHORT*)&IOcount_);
    }

    inline unsigned short NetSession::DecrementIOCount()
    {
        return InterlockedDecrement16((SHORT*)&IOcount_);
    }

    inline bool NetSession::isSendFlagFalse()
    {
        return (InterlockedExchange8((CHAR*)&isSending_, true) == false);
    }

    inline bool NetSession::SetDeleteFlag()
    {
        return static_cast<bool>(InterlockedCompareExchange16
        ((SHORT*)&IOcount_,
            DELETE_FLAG_BIT,
            NULL));
    }

    inline bool NetSession::isSending()
    {
        return isSending_;
    }

    inline void NetSession::SaveOverlappedError(const int transferred, const int errorCode, const LPOVERLAPPED overlapped)
    {
        if (isRecvOverlapped(overlapped))
            SystemLogger::GetInstance()->LogText(L"Recv GQCS false", LEVEL_SYSTEM, L"Error Code : %d, Transferred : %d", errorCode, transferred);
        else if (isSendOverlapped(overlapped))
            SystemLogger::GetInstance()->LogText(L"Send GQCS false", LEVEL_SYSTEM, L"Error Code : %d, Transferred : %d", errorCode, transferred);

        if (overlapped)
            GetOverlappedResult(transferred, overlapped);
    }

}

#endif