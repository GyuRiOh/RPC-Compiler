
#ifndef __ROOT__LAN__SERVER__
#define __ROOT__LAN__SERVER__

#include "LanSession.h"
#include "../Common/Crash.h"
#include "../Common/SystemLogger.h"
#include "LanDummyPacket.h"
#include "LanSessionArray.h"
#include "LanPacketSet.h"
#include "../Common/MyMultimap.h"
#include "../Common/SizedMemoryPool.h"
#include <process.h>
#include <vector>

using namespace std;

namespace server_baby
{
    union LanPipeID;
    class LanUser;
    class LanPipe;

    class LanRoot
    {
        enum Settings
        {
            eTCPIP_HEADER_SIZE = 40,
            eSEGMENT_SIZE = 1460
        };

        struct TPSInfo 
        {
            long sendTPS_ = 0;
            long recvTPS_ = 0; //���� üũ�غ���
            long recvIOPending_ = 0;
            long sendIOPending_ = 0;
            long recvTraffic_ = 0;
            long sendTraffic_ = 0;

        };
    
    protected:
        struct MonitoringInfo
        {
            __int64 onlineCount_ = NULL;
            int packetCount_ = NULL;
            int packetReadyCount_ = NULL;

            int largePacketCount_ = NULL;
            int largePacketReadyCount_ = NULL;
            int largePacketBufferCount_ = NULL;

            int packetQCount_ = NULL;
            int packetQReadyCount_ = NULL;

            __int64 stackCount_ = NULL;
            __int64 stackReadyCount_ = NULL;
            __int64 stackSize_ = NULL;

            __int64 LFQNodeUsed_ = NULL;
            __int64 LFQNodeCap_ = NULL;

            int sessionSetUsed_ = NULL;
            int sessionSetCap_ = NULL;

            int recvTPS_ = NULL;
            int sendTPS_ = NULL;
            int acceptTPS_ = NULL;
            long long acceptTotal_ = NULL;
            int recvTraffic_ = NULL;
            int sendTraffic_ = NULL;

            int recvIOPending_ = NULL;
            int sendIOPending_ = NULL;
        };

    public:
        explicit LanRoot();
        virtual ~LanRoot();

        void Start(
            const int IP,
            const unsigned short port,
            const int waitingThreadNum,
            const int runningThreadNum,
            const bool nagle,
            const int timeOutInterval,
            const bool logicMultithreaded,
            const bool useSendIOPending,
            const int monitorInterval = 1000); //����IP, ��Ʈ, ��Ŀ ������ ��, ���̱� �ɼ�, �ִ� ������ ��
        void Stop(void);
 
        //===========================
        // ���������� ��û�ϴ� IO���� �Լ���
        //===========================
        bool SendPacket(const LanSessionID sessionID, LanPacket* const packet);
        void SendPacket_Async(const LanSessionID sessionID, LanPacket* const packet);
        void SendPacket_Async(LanSessionIDSet* const sessionIDQueue, LanPacket* const packet);
        bool Disconnect(const LanSessionID sessionID);
        void DisconnectAfterLastMessage(const LanSessionID sessionID, LanPacket* const packet);
        void DisconnectAfterLastMessage(LanSessionIDSet* const sessionIDQueue, LanPacket* const packet);

        //===========================
        // Pipe 
        //===========================
        bool RegisterPipe(const unsigned int code, LanPipe* const  pipe);
        bool MovePipe(const LanSessionID ID, const unsigned int pipeCode);
        bool MovePipe(LanUser* const user, const unsigned int pipeCode);
        void TLSClear();

        void AfterPipeMoveOut(LanUser* const user);
        void AfterPipeEnter(const LanSessionID ID, LanPipe* const thisPipe);
        void DeletePipeUser(const LanSessionID sessionID);
        QueueWithoutCount<LanPacketSet*>* GetSessionJobQ(const LanSessionID sessionID);

    protected:
        //==========================
        //���� �����Լ���
        //�������̵��ؼ� ����ϴ� �뵵�̴�.
        //==========================
        virtual bool OnConnectionRequest(const SOCKADDR_IN* const addr) = 0; //Accept ����. return false�� Ŭ���̾�Ʈ �ź�, true�� ���� ���
        virtual void OnClientJoin(const LanSessionID sessionID) = 0; //Accept �� ���� ó�� �Ϸ� �� ȣ��.
        virtual void OnClientLeave(const LanSessionID sessionID) = 0; //Release �� ȣ��
        virtual void OnRecv(LanPacketSet* const packetList) { EnqueueJob(packetList->GetSessionID(), packetList); } //��Ŷ ���� �Ϸ� ��
        virtual void OnSend(const LanSessionID sessionID, const int sendSize) = 0; //��Ŷ �۽� �Ϸ� ��
        virtual void OnWorkerThreadBegin() = 0; //��Ŀ������ GQCS �ϴܿ��� ȣ��
        virtual void OnWorkerThreadEnd() = 0; //��Ŀ������ 1���� ���� ��
        virtual void OnError(const int errCode, const WCHAR* msg) = 0;
        virtual void OnMonitor(const MonitoringInfo* const info) = 0;

        //==========================
        //���� ����
        //==========================
        void ErrorQuit(const WCHAR* const msg);
        void ErrorQuitWithErrorCode(const WCHAR* const function);
        void ErrorDisplay(const WCHAR* const msg);
        void ErrorDisplayWithErrorCode(const WCHAR* const function);

        //==========================
        //������ ���� �Լ�
        //==========================
        void SetTLSIndex(DWORD* const index);
        bool isServerRunning();

        //==========================
        //���� ť�� ���� job �ֱ�
        //==========================
        bool EnqueueJob(const LanSessionID sessionID, LanPacketSet* const job);


    private:
        //=============================
        // ������ �Լ�
        //=============================
        static DWORD WINAPI WorkerThread(LPVOID arg); //�۾��� ������ �Լ�
        static DWORD WINAPI AcceptThread(LPVOID arg); //Accept ������ �Լ�
        static DWORD WINAPI ObserverThread(LPVOID arg); //TPS ������ Ÿ�̸� �Լ�
        static DWORD WINAPI TimeoutThread(LPVOID arg);
        
        DWORD MyWorkerProc(); //��Ŀ������ ���� �Լ�
        DWORD MyAcceptProc(); //Accept ���� ������ �Լ�
        DWORD MyObserverProc(); //TPS ���� ������ �Լ�
        DWORD MyTimeoutProc();
  
        //=============================
        // Recv, Send Post
        //=============================
        void RecvPost(LanSession* const  session);
        void SendPost(LanSession* const session);
        void SendAndDisconnectPost(LanSession* const  session);
        void OnRecvComplete(LanSession* const session, const DWORD transferred);
        void OnSendComplete(LanSession* const  session);
        void PostOtherProcedures(const LanSessionID session, ULONG_PTR const transferred, LanPacket* const packet);

        //=============================
        // ���� ������ �ʱ�ȭ
        //=============================
        void ServerInitiate();
        void InitializeLogs();
        bool isWinSockReady();
        bool isIOCPCreated();
        bool isSocketReady();
        bool isBound();
        bool isListening();
        bool isAcceptThreadRunning();
        bool isObserverThreadRunning();
        bool areWorkerThreadsRunning();
        bool arePipesRunning();

        //===============================================
        //TPS ���, �ʱ�ȭ, ����
        //===============================================
        void AddRecvTPS(const DWORD recvCount);
        void AddSendTPS(const DWORD sendCount);
        void AddRecvIOPending(const DWORD recvIOPendingCount);
        void AddSendIOPending(const DWORD sendIOPendingCount);
        void AddRecvTraffic(const DWORD transferred);
        void AddSendTraffic(const DWORD transferred);
        DWORD GetRecvTPS();
        DWORD GetSendTPS();
        DWORD GetRecvIOPending();
        DWORD GetSendIOPending();
        DWORD GetRecvTraffic();
        DWORD GetSendTraffic();
        void ZeroTPS();
        TPSInfo* GetTPS();
        TPSInfo* MakeTPS();

        //=============================================
        //���� IOCount ���� �� ����
        //=============================================
        void IncrementIOCount(LanSession* const session);
        void DecrementIOCount(LanSession* const session);
        void AddIOCount(LanSession* const  session, const int num);
        void SubIOCount(LanSession* const  session, const int num);
        void ReleaseSession(LanSession* const session);
        void DeleteSession(LanSession* const  session);

        //=============================================
        //���� Ž��
        //=============================================
        LanSession* FindSession(const LanSessionID sessionID);
        LanSession* FindSession_NotInterlocked(const LanSessionID sessionID);
        LanSession* FindSessionForSendPacket(const LanSessionID sessionID);

        //=============================================
        //���� ����
        //=============================================
        void DisconnectSession(LanSession* const  session);
        void DisconnectSessionForSendPacket(LanSession* const session);

        //=============================================
        //Pipe
        //=============================================
        LanPipe* FindPipe_LoadBalanced(const unsigned int pipeCode);

    private:
        LanSessionArray* onlineArray_;
        long long acceptTotal_;
        long acceptTPS_;

        vector<HANDLE> threadHandle_;
        HANDLE timeoutHandle_;
        SOCKET listenSocket_ = INVALID_SOCKET;
        HANDLE hcp_ = NULL;
        int waitingThreadNum_;
        int runningThreadNum_;
        int timeoutInterval_;
        int monitorInterval_;
        unsigned short port_;
        bool isRunning_;
        bool isAcceptThreadRunning_;
        bool isObserverThreadRunning_;
        bool isSendIOPending_;
        bool isUsingNagle_;
        bool isLogicMultithreaded_;

        vector<LanPipe*> pipeArray_;
        MyMultimap<unsigned int, LanPipe*> pipeMap_;

        vector<TPSInfo*> TPSArray_;
        DWORD TPS_TLS_;
        SHORT TPSIndex_;

        struct tm startT_ = { 0 };
        time_t startTimer_;

    };


    inline void server_baby::LanRoot::AddRecvTPS(const DWORD recvCount)
    {
        TPSInfo* info = GetTPS();
        info->recvTPS_ += recvCount;
    }

    inline void server_baby::LanRoot::AddSendTPS(const DWORD sendCount)
    {
        TPSInfo* info = GetTPS();
        info->sendTPS_ += sendCount;
    }

    inline void server_baby::LanRoot::AddRecvIOPending(const DWORD recvIOPendingCount)
    {
        TPSInfo* info = GetTPS();
        info->recvIOPending_ += recvIOPendingCount;
    }

    inline void server_baby::LanRoot::AddSendIOPending(const DWORD sendIOPendingCount)
    {
        TPSInfo* info = GetTPS();
        info->sendIOPending_ += sendIOPendingCount;
    }

    inline void server_baby::LanRoot::AddRecvTraffic(const DWORD transferred)
    {
        int headerCount = (int)(transferred / eSEGMENT_SIZE);
        int traffic = transferred + ((headerCount + 1) * eTCPIP_HEADER_SIZE);

        TPSInfo* info = GetTPS();
        info->recvTraffic_ += traffic;
    }

    inline void server_baby::LanRoot::AddSendTraffic(const DWORD transferred)
    {
        int headerCount = (int)(transferred / eSEGMENT_SIZE);
        int traffic = transferred + ((headerCount + 1) * eTCPIP_HEADER_SIZE);

        TPSInfo* info = GetTPS();
        info->sendTraffic_ += traffic;
    }

    inline DWORD server_baby::LanRoot::GetRecvTPS()
    {
        DWORD recvTPS = 0;
        for (int i = 0; i < TPSIndex_; i++)
        {
            recvTPS += TPSArray_[i]->recvTPS_;
        }

        return recvTPS;
    }

    inline DWORD server_baby::LanRoot::GetSendTPS()
    {
        DWORD sendTPS = 0;
        for (int i = 0; i < TPSIndex_; i++)
        {
            sendTPS += TPSArray_[i]->sendTPS_;
        }

        return sendTPS;
    }

    inline DWORD server_baby::LanRoot::GetRecvIOPending()
    {
        DWORD recvIOPending = 0;
        for (int i = 0; i < TPSIndex_; i++)
        {
            recvIOPending += TPSArray_[i]->recvIOPending_;
        }

        return recvIOPending;
    }

    inline DWORD server_baby::LanRoot::GetSendIOPending()
    {

        DWORD sendIOPending = 0;
        for (int i = 0; i < TPSIndex_; i++)
        {
            sendIOPending += TPSArray_[i]->sendIOPending_;
        }

        return sendIOPending;
    }

    inline DWORD server_baby::LanRoot::GetRecvTraffic()
    {
        DWORD recvTraffic = 0;
        for (int i = 0; i < TPSIndex_; i++)
        {
            recvTraffic += TPSArray_[i]->recvTraffic_;
        }

        return recvTraffic;
    }

    inline DWORD server_baby::LanRoot::GetSendTraffic()
    {
        DWORD sendTraffic = 0;
        for (int i = 0; i < TPSIndex_; i++)
        {
            sendTraffic += TPSArray_[i]->sendTraffic_;
        }

        return sendTraffic;
    }

    inline void server_baby::LanRoot::ZeroTPS()
    {
        for (int i = 0; i < TPSIndex_; i++)
        {
            TPSArray_[i]->recvIOPending_ = 0;
            TPSArray_[i]->sendIOPending_ = 0;
            TPSArray_[i]->recvTPS_ = 0;
            TPSArray_[i]->sendTPS_ = 0;
            TPSArray_[i]->recvTraffic_ = 0;
            TPSArray_[i]->sendTraffic_ = 0;
        }
    }

    inline bool server_baby::LanRoot::isServerRunning()
    {
        return isRunning_;
    }

    inline bool server_baby::LanRoot::EnqueueJob(const LanSessionID sessionID, LanPacketSet* const job)
    {
        LanSession* session = FindSession_NotInterlocked(sessionID);
        return session->jobQ_->Enqueue(job);
    }

    inline server_baby::LanRoot::TPSInfo* server_baby::LanRoot::GetTPS()
    {
        TPSInfo* tps = (TPSInfo*)TlsGetValue(TPS_TLS_);
        if (tps)
            return tps;

        return MakeTPS();
    }

    inline void LanRoot::IncrementIOCount(LanSession* const  session)
    {
        session->IncrementIOCount();
    }

    inline void LanRoot::DecrementIOCount(LanSession* const  session)
    {
        short ioCount = session->DecrementIOCount();
        if (ioCount == NULL)
            ReleaseSession(session);
        else if (ioCount == 0x7FFF)
            ErrorQuit(L"IOCount Under Zero");
        
    }

    inline void LanRoot::AddIOCount(LanSession* const  session, const int num)
    {
        session->AddIOCount(num);
    }

    inline void LanRoot::SubIOCount(LanSession* const  session, const int num)
    {
        short ioCount = session->SubIOCount(num);
        if (ioCount == NULL)
            ReleaseSession(session);
        else if (ioCount == 0x7FFE)
            ErrorQuit(L"IOCount Under Zero");
    }

    inline LanSession* LanRoot::FindSession(const LanSessionID sessionID)
    {
        
        LanSession* session = onlineArray_->GetSession(sessionID);
        session->IncrementIOCount();

        if (!session->AcquireSession(sessionID))
        {
            DecrementIOCount(session);
            return nullptr;
        }

        return session;
    }

    inline LanSession* LanRoot::FindSession_NotInterlocked(const LanSessionID sessionID)
    {
        LanSession* session = onlineArray_->GetSession(sessionID);
        return session;
    }

    inline LanSession* LanRoot::FindSessionForSendPacket(const LanSessionID sessionID)
    {
        LanSession* session = onlineArray_->GetSession(sessionID);
        session->AddIOCount(2);

        if (!session->AcquireSession(sessionID))
        {
            SubIOCount(session, -2);
            return nullptr;
        }

        return session;
    }

    inline void LanRoot::DisconnectSession(LanSession* const session)
    {
        session->Disconnect();
        DecrementIOCount(session);
    }

    inline void LanRoot::DisconnectSessionForSendPacket(LanSession* const  session)
    {
        session->Disconnect();
        SubIOCount(session, -2);
    }

    
    inline server_baby::LanRoot::TPSInfo* server_baby::LanRoot::MakeTPS()
    {
        TPSInfo* newTps = new TPSInfo();
        short tempIndex = 0;
        short newIndex = 0;
        do {
            tempIndex = TPSIndex_;
            newIndex = tempIndex + 1;

        } while (InterlockedCompareExchange16(
            (SHORT*)&TPSIndex_,
            newIndex,
            tempIndex) != tempIndex);

        TPSArray_[tempIndex] = newTps;

        if (TlsSetValue(TPS_TLS_, newTps) == false)
            ErrorQuit(L"TlsSetValue");

        return newTps;
    }


    inline void LanRoot::InitializeLogs()
    {
        acceptTPS_ = 0;
        ZeroTPS();
    }

    inline void server_baby::LanRoot::DeleteSession(LanSession* const session)
    {
        session->Destroy();
        onlineArray_->PushSessionIndex(session->GetTotalID());
    }
}




#endif