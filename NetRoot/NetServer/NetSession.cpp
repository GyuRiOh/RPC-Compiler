
#include "NetSession.h"
#include "NetPacketSet.h"
#include "NetPacket.h"
using namespace std;

namespace server_baby
{
    NetSession::NetSession() : sock_(INVALID_SOCKET), sentCount_(0),
        recvPacket_(nullptr), lastRecvTime_(0), isSending_(false), IOcount_(DELETE_FLAG_BIT), 
        isIOCanceled_(false), curPipe_(nullptr), destPipe_(nullptr)
    {
        ZeroMemory(&recvOverlapped_, sizeof(recvOverlapped_));
        ZeroMemory(&sendOverlapped_, sizeof(sendOverlapped_));
        ZeroMemory(&clientAddr_, sizeof(clientAddr_));
        ZeroMemory(&sentQ_, sizeof(sentQ_));

        jobQ_ = new QueueWithoutCount<NetPacketSet*>;
    }

    NetSession::~NetSession()
    {
        Destroy();
        delete jobQ_;
    }

    void NetSession::ErrorQuit(const WCHAR* const msg)
    {
        SystemLogger::GetInstance()->Console(L"Session", LEVEL_SYSTEM, msg);
        SystemLogger::GetInstance()->LogText(L"Session", LEVEL_SYSTEM, msg);

        CrashDump::Crash();
    }

    void NetSession::ErrorDisplay(const WCHAR* const msg)
    {
        SystemLogger::GetInstance()->Console(L"Session", LEVEL_SYSTEM, msg);
        SystemLogger::GetInstance()->LogText(L"Session", LEVEL_SYSTEM, msg);
    }

    bool NetSession::Initialize(const SOCKET sock, SOCKADDR_IN* addr, const NetSessionID ID)
    {
        IncrementIOCount();

        ID_ = ID;
        sock_ = sock;
        recvPacket_ = NetLargePacket::Alloc();
        lastRecvTime_ = GetTickCount64();
        isIOCanceled_ = false;
        clientAddr_ = *addr;

        InterlockedAnd16((short*)&IOcount_, DELETE_FLAG_MASK);

        return true;
    }


    bool NetSession::Destroy()
    {
        FreeSentPacket();
        SetSendFlagFalse();

        if(recvPacket_)
            NetLargePacket::Free(recvPacket_);
        
        while (sendQ_.Size() > 0)
        {
            NetPacket* packet = nullptr;
            if (!sendQ_.Dequeue(&packet))
                ErrorQuit(L"Destoy - sendQ size is more than 0 - dequeue failed");

            NetPacket::Free(packet);
        }

        if (sendQ_.Size() != 0)
            ErrorQuit(L"Destoy - SendQ Size Not Zero");
        
        lastRecvTime_ = 0;
        closesocket(sock_);
        if (InterlockedExchange64((LONG64*)&sock_, (LONG64)INVALID_SOCKET) == INVALID_SOCKET)
            ErrorQuit(L"Destroy - Socket Closed Twice");

        curPipe_ = nullptr;
        destPipe_ = nullptr;
        ClearJobQ();

        return true;

    }

    char NetSession::RecvPost()
    {
        IncrementIOCount();

        DWORD flags = 0;
        WSABUF recvBuf[1];

        recvBuf->buf = recvPacket_->GetWritePos();
        recvBuf->len = (ULONG)recvPacket_->GetEmptySize() - 1;
        

        int retval = WSARecv(sock_,
            recvBuf,
            1,
            NULL,
            &flags,
            GetRecvOverlapped(),
            NULL);

        if (retval == SOCKET_ERROR)
        {
            errno_t err = WSAGetLastError();
            if (err != WSA_IO_PENDING)
            {
                ErrorCheck(err, L"WSARecv()");
                return FAILURE_IN_IO_REQUEST;
            }

            if (isIOCanceled_)
            {
                CancelIO();
                return FAILURE_IN_IO_CANCEL_FLAG;
            }

            return SUCCESS_IO_PENDING;
        }

        return SUCCESS;

    }


    char NetSession::SendPost(long* const oldSendCount)
    {
        if (!isSendFlagFalse())
            return SEND_FAILURE_NOT_OKAY_TO_SEND;

        *oldSendCount = sentCount_;
        FreeSentPacket();

        short sendCount = 0;
        while (sendQ_.Size() > 0)
        {
            NetPacket* packet = nullptr;
            if (!DequeueSendQ(&packet))
                break;

            sentQ_[sendCount].buf = packet->GetPacketStart();
            sentQ_[sendCount].len = static_cast<ULONG>(packet->GetPacketUsedSize());

            sendCount++;

        }

        if (sendCount > eNET_PACKET_SEND_Q_MAX_SIZE)
            CrashDump::Crash();

        if (sendCount == 0)
        {
            SetSendFlagFalse();
            return SEND_FAILURE_SENDQ_EMPTY;
        }

        sentCount_ = sendCount;

        int retval = WSASend(sock_,
            sentQ_,
            sendCount,
            NULL,
            0,
            GetSendOverlapped(),
            NULL);


        if (retval == SOCKET_ERROR)
        {
            errno_t err = WSAGetLastError();
            if (err != WSA_IO_PENDING)
            {
                ErrorCheck(err, L"WSASend()");
                return FAILURE_IN_IO_REQUEST;
            }

            if (isIOCanceled_)
            {
                CancelIO();
                return FAILURE_IN_IO_CANCEL_FLAG;
            }

            return SUCCESS_IO_PENDING;
        }

        return SUCCESS;


    }

    char NetSession::SendAndDisconnectPost(long* const oldSendCount)
    {
        //보내고 끊기 다시만들기

        while (!isSendFlagFalse()) {
            if (sendQ_.Size() == 0)
            {
                isIOCanceled_ = true;
                return SEND_FAILURE_SENDQ_EMPTY;
            }
        };

        isIOCanceled_ = true;

        *oldSendCount = sentCount_;
        FreeSentPacket();

        short sendCount = 0;
        while (sendQ_.Size() > 0)
        {
            NetPacket* packet = nullptr;
            if (!DequeueSendQ(&packet))
                break;

            sentQ_[sendCount].buf = packet->GetPacketStart();
            sentQ_[sendCount].len = static_cast<ULONG>(packet->GetPacketUsedSize());

            sendCount++;

        }

        if (sendCount == 0)
        {
            SetSendFlagFalse();
            return SEND_FAILURE_SENDQ_EMPTY;
        }

        sentCount_ = sendCount;
        int retval = WSASend(sock_,
                sentQ_,
                sendCount,
                NULL,
                0,
                GetSendOverlapped(),
                NULL);


        if (retval == SOCKET_ERROR)
        {
            errno_t err = WSAGetLastError();
            if (err != WSA_IO_PENDING)
            {
                ErrorCheck(err, L"WSASend()");
                return FAILURE_IN_IO_REQUEST;
            }

            return SUCCESS_IO_PENDING;
        }

        return SUCCESS;
    }


    char server_baby::NetSession::CompleteRecvCheck_PacketQ(NetPacketSet** const packetQBuf, int* const packetCount, const DWORD transferred)
    {
        lastRecvTime_ = GetTickCount64();

        NetLargePacket* packet = recvPacket_;
        if (!packet->MoveWritePos(transferred))
        {
            ErrorDisplay(L"RECV_PACKET_FULL_DISCONNECT");
            return FAILURE_RECV_PACKET_FULL;
        }
     

        NetPacketSet* packetQ = NetPacketSet::Alloc(GetTotalID(), packet);
        int packetCnt = packetQ->RegisterPackets(packet);

        if(packetCnt == NET_PACKET_ERROR)
        {
            NetPacketSet::Free(packetQ);
            return FAILURE_PACKET_ERROR;
        }

        NetLargePacket* newPacket = packet->CopyRemainderToNewPacket(packetCnt);
        recvPacket_ = newPacket;
        *packetCount = packetCnt;
        
        if (packetCnt > 0)
        {
            *packetQBuf = packetQ;
            return SUCCESS;
        }
        else
        {
            NetPacketSet::Free(packetQ);
            return SUCCESS_ZERO_PACKET_COUNT;
        }

    }

    void server_baby::NetSession::ErrorCheck(const errno_t err, const WCHAR* const msg)
    {
        switch (err)
        {
        case 10053:
        case 10054:
            break;
        default:
        {
            SystemLogger::GetInstance()->LogText(L"Session_Disconnect",
                LEVEL_ERROR, L"%ws : error code - %d", msg, err);           
        }
        break;
        } 
        
        Disconnect();

    }

    void NetSession::FreeSentPacket()
    {
        //직렬화버퍼들 릴리즈하자.
        for (int i = 0; i < sentCount_; i++)
        {
            NetPacket::Free((NetPacket*)sentQ_[i].buf);
        }

        sentCount_ = 0;
    }

    void NetSession::ClearJobQ()
    {
        NetPacketSet* set = nullptr;
        while (jobQ_->Dequeue(&set))
        {
            NetPacketSet::Free(set);
        }
        jobQ_->Clear();
    }
}
