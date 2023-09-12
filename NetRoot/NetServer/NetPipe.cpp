

#define _WINSOCKAPI_
#include <Windows.h>
#include <process.h>
#include "NetPipe.h"
#include "../Common/SystemLogger.h"
#include "../Common/Crash.h"
#include "NetSessionID.h"
#include "NetServer.h"
#include "NetPacket.h"
#include "NetLargePacket.h"
#include "NetPacketSet.h"
#include "NetUser.h"

using namespace server_baby;

server_baby::NetPipe::NetPipe(NetRoot* const server, const unsigned int framePerSecond)
    : framePerSecond_(framePerSecond),
    runningFlag_(nullptr), server_(server), stopFlag_(false), zeroUserCount_(NULL)
{}

void server_baby::NetPipe::Initialize(const NetPipeID ID)
{
    if (pipeID_.total_)
        ErrorQuit(L"Double Initialized");

    pipeID_.total_ = ID.total_;
}

void server_baby::NetPipe::Start(const bool* const flag)
{
    SetRunningFlag(flag);

    if (!TrySubmitThreadpoolCallback(
        (PTP_SIMPLE_CALLBACK)WorkerThread,
        (PVOID)this,
        NULL))
    {
        SystemLogger::GetInstance()->LogText(L"NetPipe", LEVEL_ERROR, L"Start Failed. Error Code : %d", GetLastError());
        ErrorQuit(L"Can't Submit ThreadpoolCallback");
    } 

}

void server_baby::NetPipe::Stop()
{
    stopFlag_ = true;
}

void server_baby::NetPipe::SetRunningFlag(const bool* const flag)
{
    runningFlag_ = flag;
}

void __stdcall server_baby::NetPipe::WorkerThread(PTP_CALLBACK_INSTANCE instance, PVOID arg)
{
    NetPipe* pipe = static_cast<NetPipe*>(arg);
    unsigned int ret = pipe->MyWorkerProc();
    delete pipe;
}

unsigned int server_baby::NetPipe::MyWorkerProc()
{
    timeBeginPeriod(1); 

    unsigned int millisecPerFrame = 1000 / framePerSecond_;

    unsigned int currentTick_ = NULL;
    unsigned int updateTick_ = NULL;
    unsigned int frameTick_ = NULL;
 
    LoopInfo info;
    info.startTimer_ = time(NULL);
    updateTick_ = timeGetTime();

    OnStart();

    while ((*runningFlag_)) 
    {
        if (stopFlag_)
            break;

        if (currentTick_ >= frameTick_ + 1000)
        {

            OnMonitor(&info);
            frameTick_ = currentTick_;
            info.frameCountPerSecond_ = 0;
        }

        currentTick_ = timeGetTime();
        int sleepTime = (updateTick_ + millisecPerFrame) - currentTick_;

        if (sleepTime > 0)
        {
            Sleep(sleepTime);
            continue;
        }

        updateTick_ += millisecPerFrame;

        Join();
        Move();
        Leave();

        MessageProc();
        UpdateProc();

        info.frameCountPerSecond_++;


    }


    Move();
    Leave();

    server_->TLSClear();

    timeEndPeriod(1);  

    OnStop();

    return 0;
}

void server_baby::NetPipe::Join()
{
    NetSessionID ID;
    while (joinQ_.Dequeue(&ID))
    {
        NetUser* user = OnUserJoin(ID);
        if (!userMap_.Insert(ID.total_, user))
            ErrorQuit(L"Join - Insert Failed");
        

        server_->AfterPipeEnter(ID, this);
    }
}

void server_baby::NetPipe::Move()
{
    NetUser* user = nullptr;
    while (moveInQ_.Dequeue(&user))
    {
        user->curPipe_ = pipeID_;
        OnUserMoveIn(user);
        if (!userMap_.Insert(user->GetSessionID().total_, user))
            ErrorQuit(L"MoveIn - Insert Failed");       

        server_->AfterPipeEnter(user->GetSessionID(), this);
        user->PipeMoveEnd();
    }

    user = nullptr;
    while (moveOutQ_.Dequeue(&user))
    {
        OnUserMoveOut(user);
        if (!userMap_.Delete(user->GetSessionID().total_))
            ErrorQuit(L"MoveOut - Delete Failed");

        server_->AfterPipeMoveOut(user);
    }
}

void server_baby::NetPipe::Leave()
{
    NetSessionID ID;
    while (leaveQ_.Dequeue(&ID))
    {

        NetUser* user = FindUser(ID);

        if (!user)
            ErrorQuit(L"Leave - Find Failed");

        if(user->GetSessionID().total_ != ID.total_)
            ErrorQuit(L"Leave - DeqID, UserID Not Equal");

        if (!userMap_.Delete(ID.total_)) 
            ErrorQuit(L"Leave - Delete Failed");

        OnUserLeave(user);

        server_->DeletePipeUser(ID);   
    }
}

void server_baby::NetPipe::MessageProc()
{
    auto function = [this](ULONG id, NetUser* user)
    {
        DequeueJob(user);
    };

    userMap_.Foreach(function);
}

void server_baby::NetPipe::UpdateProc()
{

    auto function = [this](ULONG id, NetUser* const user)
    {
        OnUpdate(user);
    };

    userMap_.Foreach(function);

}

void server_baby::NetPipe::DequeueJob(NetUser* const user)
{
    for (;;)
    {
        NetPacketSet* packetQ = user->DequeueJob();
        if (!packetQ)
            break;

        if (packetQ->GetSessionID().total_ != user->GetSessionID().total_)
            ErrorQuit(L"DeqJob - packet ID, user ID Not Equal");
        
        bool ret = UnpackMsg(packetQ);
        NetPacketSet::Free(packetQ);

        if (!ret)
        {
            SystemLogger::GetInstance()->LogText(L"NetPipe", LEVEL_DEBUG, L"Disconnect, %d, %d", user->GetSessionID().element_.index_, user->GetSessionID().element_.unique_);
            server_->Disconnect(user->GetSessionID());
            break;
        }
    }
}

bool server_baby::NetPipe::UnpackMsg(NetPacketSet* const msgPack)
{
    switch (msgPack->GetType())
    {
    case eNET_RECVED_PACKET_SET:
    {
        while (msgPack->GetSize() > 0)
        {
            NetDummyPacket* packet = nullptr;
            msgPack->Dequeue(&packet);
      
            if (!OnMessage(msgPack->GetSessionID(), packet))
                return false;
        }
        break;
    }
    default:
        ErrorQuit(L"UnpackMsg - Undefined PacketSet Type");
        break;
    }

    return true;
}
