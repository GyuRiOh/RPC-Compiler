
#include "LanServer.h"
#include "../Common/LockFreeEnqQueue.h"
#include "../Common/PDHMonitor.h"
#include "LanPacket.h"
#include "LanPipe.h"
#include "LanUser.h"
#include "../Common/SizedMemoryPool.h"
#include <conio.h>

using namespace std;
using namespace server_baby;

//=========================================
//Ŭ���� ����� �ִ� �Լ���
//=========================================

LanRoot::LanRoot() 
    : listenSocket_(INVALID_SOCKET), isAcceptThreadRunning_(false), isObserverThreadRunning_(false), timeoutHandle_(INVALID_HANDLE_VALUE),
    isRunning_(false), TPS_TLS_(NULL), TPSIndex_(NULL), startTimer_(NULL), acceptTotal_(NULL), acceptTPS_(NULL),
    runningThreadNum_(0), port_(0), timeoutInterval_(0), onlineArray_(nullptr), isSendIOPending_(false), isUsingNagle_(true)
{

    CrashDump::GetInstance();
    SetTLSIndex(&TPS_TLS_);
    setlocale(LC_ALL, "");

}

LanRoot::~LanRoot()
{
    onlineArray_->~LanSessionArray();
    _aligned_free(onlineArray_);

    for (int i = 0; i < TPSIndex_; i++)
    {
        delete TPSArray_[i];
        TPSArray_[i] = nullptr;
    }
    TlsFree(TPS_TLS_);
}

void LanRoot::Start(
    const int IP,
    const unsigned short port,
    const int waitingThreadNum,
    const int runningThreadNum,
    const bool nagle,
    const int timeoutInterval,
    const bool logicMultithreaded,
    const bool useSendIOPending,
    const int monitorInterval)
{

    waitingThreadNum_ = waitingThreadNum;
    runningThreadNum_ = runningThreadNum;
    port_ = port;
    isRunning_ = true;
    isSendIOPending_ = useSendIOPending;
    timeoutInterval_ = timeoutInterval;
    isUsingNagle_ = !nagle;
    isLogicMultithreaded_ = logicMultithreaded;
    monitorInterval_ = monitorInterval;


    threadHandle_.reserve(waitingThreadNum_ + 2);
    TPSArray_.reserve(waitingThreadNum_ + 2);

    ServerInitiate();
}

void LanRoot::Stop(void)
{
    closesocket(listenSocket_);
    isAcceptThreadRunning_ = false;
    onlineArray_->Invalidate();

    Sleep(5000);
    PostQueuedCompletionStatus(hcp_, 0, 0, 0);
    WaitForMultipleObjects(waitingThreadNum_ + 1, &threadHandle_[1], true, INFINITE);
    SystemLogger::GetInstance()->Console(L"LanServer", LEVEL_SYSTEM, L"Server Down...");

    Sleep(5000);
    isRunning_ = false;
    for (int i = 0; i < waitingThreadNum_ + 2; i++)
    {
        CloseHandle(threadHandle_[i]);
    }
    CloseHandle(hcp_);
    CloseHandle(timeoutHandle_);
    hcp_ = NULL;
    WSACleanup();
    TLSClear();

    Sleep(5000);
    isObserverThreadRunning_ = false;
    SystemLogger::GetInstance()->Console(L"LanServer", LEVEL_SYSTEM, L"Server Stopped");
}

void server_baby::LanRoot::ServerInitiate()
{
    if (!isWinSockReady())
        ErrorQuit(L"isWinSockReady : incomplete");

    if (!isIOCPCreated())
        ErrorQuit(L"isIOCPCreated : incomplete");

    if (!isSocketReady())
        ErrorQuit(L"isSocketReady : incomplete");

    if (!isBound())
        ErrorQuit(L"isBound : incomplete");

    if (!isListening())
        ErrorQuit(L"isListening : incomplete");

    if (!areWorkerThreadsRunning())
        ErrorQuit(L"areWorkerThreadsRunning : incomplete");

    if (!onlineArray_)
    {
        onlineArray_ = (LanSessionArray*)_aligned_malloc(sizeof(LanSessionArray), 64);
        new (onlineArray_) LanSessionArray(timeoutInterval_);
    }
    else
        onlineArray_->Validate();

    if (!arePipesRunning())
        ErrorQuit(L"arePipesRunning : incomplete");

    if (!isObserverThreadRunning())
        ErrorQuit(L"isObserverThreadRunning : incomplete");

    Sleep(1000);

    if (!isAcceptThreadRunning())
        ErrorQuit(L"isAcceptThreadRunning : incomplete");

    wchar_t string[64] = { 0 };
    startTimer_ = time(NULL);
    localtime_s(&startT_, &startTimer_);

    swprintf(string, 64, L"%d/%02d/%02d/%02d/%02d",
        startT_.tm_year + 1900,
        startT_.tm_mon + 1,
        startT_.tm_mday,
        startT_.tm_hour,
        startT_.tm_min);

    SystemLogger::GetInstance()->Console(L"LanServer", LEVEL_SYSTEM, L"%ws : Server is running...",string);

}


bool LanRoot::isWinSockReady()
{
    //���� �ʱ�ȭ
    WSADATA wsa;
    return (WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
}

bool LanRoot::isIOCPCreated()
{
    //����� �Ϸ� ��Ʈ ����
    hcp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, waitingThreadNum_);
    return static_cast<bool>(hcp_);
}

bool LanRoot::isSocketReady()
{
    //socket()
    listenSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket_ == INVALID_SOCKET)
    {
        ErrorDisplayWithErrorCode(L"isSocketReady()");
        return false;
    }

    //���� �ɼ� ����. Ÿ�Ӿƿ� ���� 0
    struct linger optval;
    optval.l_onoff = 1;
    optval.l_linger = 0;
    int retval = setsockopt(listenSocket_, SOL_SOCKET, SO_LINGER, (char*)&optval, sizeof(optval));

    //���̱� ����            
    setsockopt(listenSocket_,
        IPPROTO_TCP, 
        TCP_NODELAY,
        (const char*)&isUsingNagle_, 
        sizeof(isUsingNagle_)); 


    //�۽� ���� ������
    int bufSize = 1024*1024;
    if (isSendIOPending_)
        bufSize = 0;

    int len = sizeof(bufSize);
    retval = setsockopt(listenSocket_, SOL_SOCKET, SO_SNDBUF, (char*)&bufSize, sizeof(bufSize));
    return true;
}

bool LanRoot::isBound()
{
    SOCKADDR_IN serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port_);

    int retval = ::bind(listenSocket_, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (retval == SOCKET_ERROR)
    {
        ErrorDisplayWithErrorCode(L"isBound()");
        return false;
    }
    else
        return true;
}

bool LanRoot::isListening()
{
    //listen()
    int retval = listen(listenSocket_, SOMAXCONN_HINT(10000));    
    if (retval == SOCKET_ERROR)
    {
        ErrorDisplayWithErrorCode(L"isListening()");
        return false;
    }
    else
        return true;
}

bool LanRoot::isAcceptThreadRunning()
{
    isAcceptThreadRunning_ = true;
    //Accept() ���� ������ ����

    unsigned threadID;
    threadHandle_[1] = (HANDLE)_beginthreadex(
        NULL, 
        0, 
        (_beginthreadex_proc_type)&LanRoot::AcceptThread, 
        (LPVOID)this,
        0, 
        &threadID);

    if (threadHandle_[1] == NULL)
    {
        isAcceptThreadRunning_ = false;
        return false;
    }


    return true;
}

bool server_baby::LanRoot::isObserverThreadRunning()
{
    isObserverThreadRunning_ = true;
    //������ ������ ����
    unsigned threadID;
    threadHandle_[0] = (HANDLE)_beginthreadex(
        NULL,
        0,
        (_beginthreadex_proc_type)&LanRoot::ObserverThread,
        (LPVOID)this,
        0,
        &threadID);

    if (threadHandle_[0] == NULL)
    {
        isObserverThreadRunning_ = false;
        return false;
    }

    //Ÿ�Ӿƿ� ������ ����
    timeoutHandle_ = (HANDLE)_beginthreadex(
        NULL,
        0,
        (_beginthreadex_proc_type)&LanRoot::TimeoutThread,
        (LPVOID)this,
        0,
        &threadID);

    if (timeoutHandle_ == NULL)
    {
        isObserverThreadRunning_ = false;
        return false;
    }

    return true;
}

bool LanRoot::areWorkerThreadsRunning()
{
    
    unsigned int* threadIDforWorker = (unsigned int*)malloc(sizeof(unsigned int) * waitingThreadNum_);
    for (int i = 0; i < waitingThreadNum_; i++)
    {
        threadHandle_[i+2] = (HANDLE)_beginthreadex(
            NULL, 
            0, 
            (_beginthreadex_proc_type)&WorkerThread, 
            (LPVOID)this, 
            0, 
            &threadIDforWorker[i]);

        if (threadHandle_[i+2] == NULL)
            return false;
    }
    free(threadIDforWorker);
    return true;
}

bool server_baby::LanRoot::arePipesRunning()
{
    for (int i = 0; i < pipeArray_.size(); i++)
    {
        pipeArray_[i]->Start(&isRunning_);
    }

    return true;
}


bool server_baby::LanRoot::SendPacket(const LanSessionID sessionID, LanPacket* const  packet)
{
    LanSession* session = FindSessionForSendPacket(sessionID);
    if (!session)
        return false;

    packet->AddRef();

    if (!session->EnqueueSendQ(packet))
    {
        SystemLogger::GetInstance()->LogText(L"SendPacket", LEVEL_ERROR, L"Enq Fail - Disconecct. Cur ID : %d, IO Count : %d", sessionID.element_.unique_, session->GetIOCount());
        DisconnectSessionForSendPacket(session);
        LanPacket::Free(packet);
        return false;
    }

    SendPost(session);

    return true;
}

void LanRoot::SendPacket_Async(const LanSessionID sessionID, LanPacket* const packet)
{
    packet->AddRef();

    PostQueuedCompletionStatus(hcp_,
        eLAN_SENDPACKET_KEY,
        (ULONG_PTR)sessionID.total_,
        (LPOVERLAPPED)packet);

}

void server_baby::LanRoot::SendPacket_Async(LanSessionIDSet* const sessionIDSet, LanPacket* const packet)
{
    packet->AddRef(sessionIDSet->GetSize());

    int size = sessionIDSet->GetSize();
    int distributeNum = size / waitingThreadNum_;
    if (distributeNum == NULL)
        distributeNum = 1;

    while (size >= distributeNum)
    {
        LanSessionIDSet* distributedSet = LanSessionIDSet::Alloc();

        for (int i = 0; i < distributeNum; i++)
        {
            LanSessionID ID;
            if (!sessionIDSet->Dequeue(&ID))
                ErrorQuit(L"SendPacket_Async - LanSessionIDSet deq failed");

            distributedSet->Enqueue(ID);
        }

        PostQueuedCompletionStatus(hcp_,
            eLAN_SENDPACKET_QUEUE_KEY,
            (ULONG_PTR)distributedSet,
            (LPOVERLAPPED)packet);

        size -= distributeNum;
    }

    if (size == 0)
    {
        LanSessionIDSet::Free(sessionIDSet);
        return;
    }

    PostQueuedCompletionStatus(hcp_,
        eLAN_SENDPACKET_QUEUE_KEY,
        (ULONG_PTR)sessionIDSet,
        (LPOVERLAPPED)packet); 

}

bool server_baby::LanRoot::Disconnect(const LanSessionID sessionID)
{

    LanSession* session = FindSession(sessionID);
    if (!session)
        return false;

    if (session->isIOCanceled())
    {
        DecrementIOCount(session);
        return false;
    }

    SystemLogger::GetInstance()->LogText(L"LanRoot", LEVEL_DEBUG, L"Disconnect, %d, %d", sessionID.element_.index_, sessionID.element_.unique_);
    DisconnectSession(session);
    return true;
}

void server_baby::LanRoot::DisconnectAfterLastMessage(const LanSessionID sessionID, LanPacket* const  packet)
{
    packet->AddRef();

    PostQueuedCompletionStatus(hcp_,
        eLAN_SEND_DISCONNECT_KEY,
        (ULONG_PTR)sessionID.total_,
        (LPOVERLAPPED)packet);
   
}

void server_baby::LanRoot::DisconnectAfterLastMessage(LanSessionIDSet* const sessionIDQueue, LanPacket* const packet)
{
    packet->AddRef(sessionIDQueue->GetSize());

    int size = sessionIDQueue->GetSize();
    int distributeNum = size / waitingThreadNum_;
    if (distributeNum == NULL)
        distributeNum = 1;

    while (size >= distributeNum)
    {
        LanSessionIDSet* distributedSet = LanSessionIDSet::Alloc();

        for (int i = 0; i < distributeNum; i++)
        {
            LanSessionID ID;
            if (!sessionIDQueue->Dequeue(&ID))
                ErrorQuit(L"DisconnectWithLastMessage - LanSessionIDSet deq failed");

            distributedSet->Enqueue(ID);
        }

        PostQueuedCompletionStatus(hcp_,
            eLAN_SEND_DISCONNECT_QUEUE_KEY,
            (ULONG_PTR)distributedSet,
            (LPOVERLAPPED)packet);

        size -= distributeNum;
    }

    if (size == 0)
    {
        LanSessionIDSet::Free(sessionIDQueue);
        return;
    }

    PostQueuedCompletionStatus(hcp_,
        eLAN_SEND_DISCONNECT_QUEUE_KEY,
        (ULONG_PTR)sessionIDQueue,
        (LPOVERLAPPED)packet);
}

void server_baby::LanRoot::DeletePipeUser(const LanSessionID sessionID)
{
    LanSession* session = FindSession_NotInterlocked(sessionID);
    DeleteSession(session);
}

bool server_baby::LanRoot::MovePipe(const LanSessionID ID, const unsigned int pipeCode)
{
    LanSession* session = FindSession(ID);
    if (!session)
        return false;

    //�̸� �˻��ϰ� ������
    if (!pipeCode)
        ErrorQuit(L"Pipe Code NULL !!");

    if (session->curPipe_)
    {
        if (session->curPipe_->GetPipeID().element_.code_ == pipeCode)
        {
            SystemLogger::GetInstance()->LogText(L"MovePipe", LEVEL_SYSTEM, L"Same PipeCode");
            DecrementIOCount(session);
            return false;
        }
    }

    LanPipe* newPipe = FindPipe_LoadBalanced(pipeCode);
    if (!newPipe)
        ErrorQuit(L"NewPipe Does Not Exist In the Map");

    session->ClearJobQ();
    session->destPipe_ = newPipe;

    newPipe->RequestEnter(ID);

    return true;
}

bool server_baby::LanRoot::MovePipe(LanUser* const  user, const unsigned int pipeCode)
{
    LanSession* session = FindSession(user->GetSessionID());
    if (!session)
        return false;

    //�̸� �˻��ϰ� ������
    if (!pipeCode)
        ErrorQuit(L"Pipe Code NULL !!");

    if (user->GetCurrentPipeID().element_.code_ == pipeCode)
    {
        SystemLogger::GetInstance()->LogText(L"MovePipe", LEVEL_SYSTEM, L"Same PipeCode");
        DecrementIOCount(session);
        return false;
    }

    user->PipeMoveStart();

    LanPipe* newPipe = FindPipe_LoadBalanced(pipeCode);
    if (!newPipe)
        ErrorQuit(L"NewPipe Does Not Exist In the Map");

    session->ClearJobQ();
    session->destPipe_ = newPipe;
    

    if (session->curPipe_->GetPipeID().element_.code_)
        session->curPipe_->RequestMoveOut(user);
    else
    {
        LanPipe* newPipe = FindPipe_LoadBalanced(pipeCode);
        if (!newPipe)
            ErrorQuit(L"NewPipe Does Not Exist In the Map");

        newPipe->RequestEnter(user);
    }


    return true;
}

QueueWithoutCount<LanPacketSet*>* server_baby::LanRoot::GetSessionJobQ(const LanSessionID sessionID)
{
    LanSession* session = FindSession_NotInterlocked(sessionID);
    return session->jobQ_;
}

void server_baby::LanRoot::AfterPipeMoveOut(LanUser* const  user)
{
    LanSession* session = FindSession_NotInterlocked(user->GetSessionID());
    session->destPipe_->RequestEnter(user);
}

void server_baby::LanRoot::AfterPipeEnter(const LanSessionID ID, LanPipe* const  thisPipe)
{
    LanSession* session = FindSession_NotInterlocked(ID);

    if (session->destPipe_ != thisPipe)
        ErrorQuit(L"AfterPipeEnter - DestPipe, ThisPipe Not Equal");

    session->curPipe_ = session->destPipe_;
    session->destPipe_ = nullptr;


    DecrementIOCount(session);
}

bool server_baby::LanRoot::RegisterPipe(const unsigned int code, LanPipe* const pipe)
{
    if (!code)
        ErrorQuit(L"Register Pipe - Code NULL");

    LanPipeID pipeID;
    pipeID.element_.code_ = code;
    pipeID.element_.index_ = static_cast<unsigned int>(pipeArray_.size());


    pipe->Initialize(pipeID);
    pipeArray_.push_back(std::move(pipe));
    pipeMap_.Put(code, pipe);
    

    return true;
}

LanPipe* LanRoot::FindPipe_LoadBalanced(const unsigned int pipeCode)
{

    LanPipe* retPipe = nullptr;

    auto function = [&retPipe](unsigned int code, LanPipe* pipe)
    {
        if (!retPipe)
        {
            retPipe = pipe;
            return;
        }

        if (pipe->GetUserSize() < retPipe->GetUserSize())
        {
            retPipe = pipe;
        }
    };

    pipeMap_.ForeachForSameKey(function, pipeCode);

    return retPipe;
}


//==========================================
// Send, Recv ȣ�� �Լ�
//==========================================


void LanRoot::RecvPost(LanSession* const session)
{
    char retval = session->RecvPost();

    switch (retval)
    {
    case LAN_SUCCESS:
        DecrementIOCount(session);
        break;
    case LAN_SUCCESS_IO_PENDING:
        AddRecvIOPending(1);
        DecrementIOCount(session);
        break;
    case LAN_FAILURE_IN_IO_REQUEST:
        SubIOCount(session, -2);
        break;
    case LAN_FAILURE_IN_IO_CANCEL_FLAG:
        DecrementIOCount(session);
        break;
    default:
        ErrorQuit(L"RecvPost - Undefined Error Code");
        break;
    }
}


void LanRoot::SendPost(LanSession* const session)
{

    long oldSendCount = 0;
    char retval = session->SendPost(&oldSendCount);

    switch (retval)
    {
    case LAN_SUCCESS:
        AddSendTPS(oldSendCount);
        break;
    case LAN_SUCCESS_IO_PENDING:
    {
        AddSendTPS(oldSendCount);
        AddSendIOPending(1);
        if (!isSendIOPending_)
        {
            ErrorDisplay(L"Disconnect For SendIOPending");
            DisconnectSession(session);
        }
        DecrementIOCount(session);
        break;
    }
    case LAN_SEND_FAILURE_NOT_OKAY_TO_SEND:
        SubIOCount(session, -2);
        break;
    case LAN_SEND_FAILURE_SENDQ_EMPTY:
        AddSendTPS(oldSendCount);
        SubIOCount(session, -2);
        break;
    case LAN_FAILURE_IN_IO_REQUEST:
    {
        AddSendTPS(oldSendCount);
        SubIOCount(session, -2);
        break;
    }
    case LAN_FAILURE_IN_IO_CANCEL_FLAG:
        AddSendTPS(oldSendCount);
        DecrementIOCount(session);
        break;
    default:
        ErrorQuit(L"SendPost - Undefined Error Code");
        break;
    }

}

void server_baby::LanRoot::OnRecvComplete(LanSession* const session, const DWORD transferred)
{

    if (session->isIOCanceled())
    {
        DecrementIOCount(session);
        return;
    }

    LanPacketSet* packetQ = nullptr;
    int packetCnt = 0; 

    char retval = session->CompleteRecvCheck_PacketQ(&packetQ, &packetCnt, transferred);

    switch (retval)
    {
    case LAN_SUCCESS:
    {  
        OnRecv(packetQ);
        AddRecvTPS(packetCnt);
        RecvPost(session);
    }
    break;
    case LAN_SUCCESS_ZERO_PACKET_COUNT:
        RecvPost(session);
        break;
    case LAN_FAILURE_RECV_PACKET_FULL:
        ErrorQuit(L"OnRecvComplete - Recv Packet Full");
        break;
    case LAN_FAILURE_PACKET_ERROR:
        DisconnectSession(session);
        break;
    default:
        ErrorQuit(L"OnRecvComplete - Undefined Error Code");
        break;
    }
}

void server_baby::LanRoot::OnSendComplete(LanSession* const session)
{
    OnSend(session->GetTotalID(), session->GetSendCount());

    if (isSendIOPending_)
        session->IncrementIOCount();

    if (session->isIOCanceled())
    {
        SubIOCount(session, -2);
        return;
    }

    session->SetSendFlagFalse();
    SendPost(session);
}



//==========================================
//�������
//==========================================

DWORD __stdcall LanRoot::WorkerThread(LPVOID arg)
{
    LanRoot* server = (LanRoot*)arg;
    return server->MyWorkerProc();

}

DWORD __stdcall LanRoot::AcceptThread(LPVOID arg)
{

    LanRoot* server = (LanRoot*)arg;
    return server->MyAcceptProc();

}

DWORD __stdcall LanRoot::ObserverThread(LPVOID arg)
{

    LanRoot* server = (LanRoot*)arg;
    return server->MyObserverProc();
}

DWORD __stdcall server_baby::LanRoot::TimeoutThread(LPVOID arg)
{
    LanRoot* server = (LanRoot*)arg;
    return server->MyTimeoutProc();
}

DWORD LanRoot::MyAcceptProc()
{
    while(isAcceptThreadRunning_)
    {

        SOCKADDR_IN clientAddr;
        int addrLen = sizeof(clientAddr);

        SOCKET clientSock = accept(listenSocket_, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSock == INVALID_SOCKET)
        {
            SystemLogger::GetInstance()->LogText(L"LanServer_Disconnect", 
                LEVEL_ERROR, L"%ws : error code - %d", L"Accept", WSAGetLastError());
            continue;
        }

        acceptTPS_++;
        acceptTotal_++;

        //Ŀ�ؼ� ��û�� ���� IP, ��Ʈ ���͸�
        if (!OnConnectionRequest(&clientAddr))
        {
            closesocket(clientSock);
            continue;
        }
           
        //���� �Ҵ�
        LanSession* session = onlineArray_->AllocSession(clientSock, &clientAddr);
        if (!session)
        {
            closesocket(clientSock);
            continue;
        }

        CreateIoCompletionPort((HANDLE)clientSock, hcp_, (ULONG_PTR)session, 0);

        OnClientJoin(session->GetTotalID());

        RecvPost(session);

    }

    TLSClear();

    return 0;
}

DWORD LanRoot::MyObserverProc()
{
 
    timeBeginPeriod(1);

    MonitoringInfo info;

    int division = monitorInterval_ / 1000;

    unsigned int currentTick = timeGetTime();
    unsigned int monitorTick = currentTick;

    while (isObserverThreadRunning_)
    {

        currentTick = timeGetTime();
        unsigned int tickSum = monitorTick + monitorInterval_;

        if (currentTick >= tickSum)
        {
            info.onlineCount_ = onlineArray_->GetSessionCount();

            info.packetCount_ = LanPacket::GetUsedCount();
            info.packetReadyCount_ = LanPacket::GetCapacity();

            info.largePacketCount_ = LanLargePacket::GetUsedCount();
            info.largePacketReadyCount_ = LanLargePacket::GetCapacity();
            info.largePacketBufferCount_ = LanLargePacket::GetBufferCount();

            info.packetQCount_ = LanPacketSet::GetUsedCount();
            info.packetQReadyCount_ = LanPacketSet::GetCapacity();

            info.stackCount_ = onlineArray_->GetStackUsedCount();
            info.stackReadyCount_ = onlineArray_->GetStackCapacity();
            info.stackSize_ = onlineArray_->GetStackSize();

            info.LFQNodeUsed_ = LanSession::GetSendQPoolCount();
            info.LFQNodeCap_ = LanSession::GetSendQPoolCapacity();

            info.sessionSetUsed_ = LanSessionIDSet::GetUsedCount();
            info.sessionSetCap_ = LanSessionIDSet::GetCapacity();

            info.recvTPS_ = GetRecvTPS() / division;
            info.sendTPS_ = GetSendTPS() / division;
            info.acceptTotal_ = acceptTotal_;
            info.acceptTPS_ = acceptTPS_ / division;
            info.recvTraffic_ = GetRecvTraffic() / division;
            info.sendTraffic_ = GetSendTraffic() / division;
            info.recvIOPending_ = GetRecvIOPending() / division;
            info.sendIOPending_ = GetSendIOPending() / division;

            InitializeLogs();
            OnMonitor(&info);
            monitorTick = currentTick;
        }
        else
            Sleep(tickSum - currentTick);

    }

    timeEndPeriod(1);
}

DWORD server_baby::LanRoot::MyTimeoutProc()
{
    while (isObserverThreadRunning_)
    {
        onlineArray_->Timeout();
        Sleep(10000);
    }
    return 0;
}


void server_baby::LanRoot::PostOtherProcedures(const LanSessionID sessionID, ULONG_PTR const key, LanPacket* const  packet)
{

    switch (static_cast<int>(key))
    {
    case eLAN_SENDPACKET_KEY:
    {
        LanSession* session = FindSessionForSendPacket(sessionID);
        if (!session)
        {
            LanPacket::Free(packet);
            return;
        }

        if (!session->EnqueueSendQ(packet))
        {
            DisconnectSessionForSendPacket(session);
            LanPacket::Free(packet);
            return;
        }

        SendPost(session);
        break;
    }
    case eLAN_SENDPACKET_QUEUE_KEY:
    {
        LanSessionIDSet* set = (LanSessionIDSet*)sessionID.total_;
        while(set->GetSize() > 0)
        {
            LanSessionID ID;
            set->Dequeue(&ID);

            LanSession* session = FindSessionForSendPacket(ID);
            if (!session)
            {
                LanPacket::Free(packet);
                continue;
            }

            if (!session->EnqueueSendQ(packet))
            {
                DisconnectSessionForSendPacket(session);
                LanPacket::Free(packet);
                continue;
            }

            SendPost(session);
        }

        LanSessionIDSet::Free(set);
        break;
    }
    case eLAN_SEND_DISCONNECT_KEY:
    {
        LanSession* session = FindSessionForSendPacket(sessionID);
        if (!session)
        {
            LanPacket::Free(packet);
            return;
        }

        if (!session->EnqueueSendQ(packet))
        {
            DisconnectSessionForSendPacket(session);
            LanPacket::Free(packet);
            return;
        }

        SendAndDisconnectPost(session);
        break;
    }
    case eLAN_SEND_DISCONNECT_QUEUE_KEY: 
    {
        LanSessionIDSet* set = (LanSessionIDSet*)sessionID.total_;
        while (set->GetSize() > 0)
        {
            LanSessionID ID;
            set->Dequeue(&ID);

            LanSession* session = FindSessionForSendPacket(ID);
            if (!session)
            {
                LanPacket::Free(packet);
                continue;
            }

            if (!session->EnqueueSendQ(packet))
            {
                DisconnectSessionForSendPacket(session);
                LanPacket::Free(packet);
                continue;
            }

            SendAndDisconnectPost(session);
        }

        LanSessionIDSet::Free(set);
        break;
    }
    case eLAN_CLIENT_LEAVE_KEY:
        OnClientLeave(sessionID);
        break;
    case eLAN_DISCONNECT_KEY:
        Disconnect(sessionID);
        break;
    default:
        ErrorQuit(L"PostOtherProc - Undefined Procedure");
    }

}

void server_baby::LanRoot::SendAndDisconnectPost(LanSession* const session)
{
    long oldSendCount = 0;
    char retval = session->SendAndDisconnectPost(&oldSendCount);

    switch (retval)
    {
    case LAN_SUCCESS:
        AddSendTPS(oldSendCount);
        break;
    case LAN_SUCCESS_IO_PENDING:
    {
        AddSendTPS(oldSendCount);
        AddSendIOPending(1);
        if (!isSendIOPending_)
        {
            ErrorDisplay(L"Disconnect For SendIOPending");
            DisconnectSession(session);
        }
        DecrementIOCount(session);
        break;
    }
    case LAN_SEND_FAILURE_SENDQ_EMPTY:
        AddSendTPS(oldSendCount);
        SubIOCount(session, -2);
        break;
    case LAN_FAILURE_IN_IO_REQUEST:
        AddSendTPS(oldSendCount);
        SubIOCount(session, -2);
        break;
    default:
        ErrorQuit(L"SendAndDisconnectPost - Undefined Error Code");
        break;
    }
}

DWORD server_baby::LanRoot::MyWorkerProc()
{
    int retval;

    for (;;)
    {
        DWORD transferred = 0;
        LanSession* session = nullptr;
        LPOVERLAPPED overlapped = nullptr;
        retval = GetQueuedCompletionStatus(
            hcp_,
            (LPDWORD)&transferred,
            (PULONG_PTR)&session,
            &overlapped,
            INFINITE);

        if (session == nullptr && transferred == 0 && overlapped == nullptr)
        {
            PostQueuedCompletionStatus(hcp_, 0, 0, 0);
            break;
        }

        OnWorkerThreadBegin();

        if (!retval)
        {
            int errCode = WSAGetLastError();
            switch (errCode)
            {
            case ERROR_NETNAME_DELETED:
            {
                //��밡 RST������ ����
                //���� ����, ���μ��� ����� ���� ���������� ��� �� ��ƾ�� ź��.
                if (session->isRecvOverlapped(overlapped)
                    || session->isSendOverlapped(overlapped))
                    DisconnectSession(session);
                else
                    ErrorQuit(L"Undefined Overlapped-64");
            }
            break;
            case ERROR_SEM_TIMEOUT:
            {
                //��ŷ� ������ �߻��ϴ� �������� Ÿ�Ӿƿ�
                session->SaveOverlappedError(transferred, errCode, overlapped);

                if (session->isRecvOverlapped(overlapped)
                    || session->isSendOverlapped(overlapped))
                    DisconnectSession(session);
                else
                    ErrorQuit(L"Undefined Overlapped-121");
            }
            break;
            case ERROR_OPERATION_ABORTED:
            {
                //Cancel IO EX ȣ��� ����   
                if (session->isRecvOverlapped(overlapped))
                {
                    if (session->isIOCanceled())
                        DecrementIOCount(session);
                    else
                        ErrorQuit(L"Recv Overlapped-IO Not Canceled - 995");
                }
                else if (session->isSendOverlapped(overlapped))
                {
                    if (session->isIOCanceled())
                        DecrementIOCount(session);
                    else
                        ErrorQuit(L"Send Overlapped-IO Not Canceled - 995");

                }
                else
                    ErrorQuit(L"Undefined Overlapped-995");
            }
            break;
            case ERROR_CONNECTION_ABORTED:
            {
                session->SaveOverlappedError(transferred, errCode, overlapped);
                if (session->isRecvOverlapped(overlapped))
                {
                    if (session->isIOCanceled())
                        DecrementIOCount(session);
                    else
                        ErrorQuit(L"Recv Overlapped-IO Not Canceled - 1236");
                }
                else if (session->isSendOverlapped(overlapped))
                {
                    if (session->isIOCanceled())
                        DecrementIOCount(session);
                    else
                        ErrorQuit(L"Send Overlapped-IO Not Canceled - 1236");

                }
                else
                    ErrorQuit(L"Undefined Overlapped-1236");
            }
            break;
            default:
            {      
                SystemLogger::GetInstance()->LogText(L"Unhandled GQCS false", LEVEL_SYSTEM, L"Error Code : %d", errCode);
                ErrorQuit(L"GQCS Failed - Unhandled Error Code");
            }
            break;
            }


            OnWorkerThreadEnd();
            continue;
        }


        //Fin �޾��� ��
        if (transferred == 0)
        {
            if (session->isRecvOverlapped(overlapped))
                DisconnectSession(session);
            else
                ErrorQuit(L"Undefined Overlapped-Retval True, Transferred 0");

            OnWorkerThreadEnd();
            continue;
        }

        if (session->isRecvOverlapped(overlapped))
        {
            AddRecvTraffic(transferred);
            OnRecvComplete(session, transferred);
        }
        else if (session->isSendOverlapped(overlapped))
        {
            AddSendTraffic(transferred);
            OnSendComplete(session);
        }
        else
            PostOtherProcedures(session, transferred, (LanPacket*)overlapped);

        OnWorkerThreadEnd();
    }

    TLSClear();

    return 0;
}


void server_baby::LanRoot::SetTLSIndex(DWORD* const index)
{
    if (InterlockedCompareExchange(index, TlsAlloc(), NULL) == NULL)
    {
        if (*index == TLS_OUT_OF_INDEXES)
        {
            //TLS Alloc �Լ��� ��Ʈ �÷��� �迭�κ��� ���� ������ �÷��׸� ã�� ���ߴ�.
            ErrorQuit(L"TLSAlloc Failed - OUT OF INDEX");
        }
    }
}

void server_baby::LanRoot::ErrorQuit(const WCHAR* const msg)
{

    SystemLogger::GetInstance()->Console(L"LanServer", LEVEL_SYSTEM, msg);
    SystemLogger::GetInstance()->LogText(L"LanServer", LEVEL_SYSTEM, msg);

    CrashDump::Crash();
}

void server_baby::LanRoot::ErrorQuitWithErrorCode(const WCHAR* const function)
{
    int errorCode = WSAGetLastError();

    SystemLogger::GetInstance()->Console(L"LanServer",
        LEVEL_SYSTEM, L"%ws : error code - %d", function, errorCode);

    SystemLogger::GetInstance()->LogText(L"LanServer",
        LEVEL_SYSTEM, L"%ws : error code - %d", function, errorCode);

    OnError(errorCode, (WCHAR*)function);
    CrashDump::Crash();
}
void server_baby::LanRoot::ErrorDisplay(const WCHAR* const msg)
{
    SystemLogger::GetInstance()->Console(L"LanServer", LEVEL_SYSTEM, msg);
    SystemLogger::GetInstance()->LogText(L"LanServer", LEVEL_SYSTEM, msg);
}

void server_baby::LanRoot::ErrorDisplayWithErrorCode(const WCHAR* const function)
{
    int errorCode = WSAGetLastError();

    SystemLogger::GetInstance()->Console(L"LanServer",
        LEVEL_SYSTEM, L"%ws : error code - %d", function, errorCode);

    SystemLogger::GetInstance()->LogText(L"LanServer",
        LEVEL_SYSTEM, L"%ws : error code - %d", function, errorCode);

    int a = sizeof(SOCKADDR_IN);
    int b = sizeof(LockFreeEnqQueue<LanPacket*, 1, 1>);
    OnError(errorCode, (WCHAR*)function);
}

void server_baby::LanRoot::TLSClear()
{
    LanPacket::DeleteTLS();
    LanLargePacket::DeleteTLS();
    LanPacketSet::DeleteTLS();
    LanSessionIDSet::DeleteTLS();
    LanSession::DeleteSendQPoolTLS();
    onlineArray_->DeleteStackTLS();

    SizedMemoryPool::GetInstance()->DeleteTLS();
}


void server_baby::LanRoot::ReleaseSession(LanSession* session)
{
    if (session->SetDeleteFlag())
        return;

    if (isLogicMultithreaded_)
    {
        PostQueuedCompletionStatus(hcp_,
            eLAN_CLIENT_LEAVE_KEY,
            (ULONG_PTR)session->GetTotalID().total_,
            NULL);
    }
    else
        OnClientLeave(session->GetTotalID());

    if (session->curPipe_)
        session->curPipe_->RequestLeave(session->GetTotalID());
    else
        DeleteSession(session);
}

