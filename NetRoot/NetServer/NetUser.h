
#ifndef __NET__PIPE__USER__
#define __NET__PIPE__USER__

#include "NetPipe.h"
#include "../Common/Queue.h"
#include "NetServer.h"

namespace server_baby
{
	class NetPacket;

	class NetUser
	{
	public:
		explicit NetUser(const NetSessionID ID, NetRoot* const server, const NetPipeID pipeID) : isMoving_(false), jobQ_(nullptr), server_(nullptr)
		{
			Initialize(ID, server, pipeID);
		}
		virtual ~NetUser() 
		{ 
			sessionID_.total_ = NULL;
			curPipe_.total_ = NULL;
			jobQ_ = nullptr;
			server_ = nullptr;
		}

		void Initialize(const NetSessionID ID, NetRoot* const server, const NetPipeID pipeID);
		void SendPacket(NetPacket* const packet);
		void SendPacket(NetSessionIDSet* const idSet, NetPacket* const packet);
		bool Disconnect();
		void DisconnectAfterLastMessage(NetPacket* const packet);
		void DisconnectAfterLastMessage(NetSessionIDSet* const idSet, NetPacket* const packet);

		NetSessionID GetSessionID() { return sessionID_; }
		NetPipeID GetCurrentPipeID() { return curPipe_; }

		bool IsJobQEmpty() { return jobQ_->isEmpty(); }
		NetPacketSet* DequeueJob();
		void PipeMoveStart() { isMoving_ = true; }
		void PipeMoveEnd() { isMoving_ = false; }

	private:
		void ErrorQuit(const wchar_t* const msg);

	private:
		NetSessionID sessionID_;
		QueueWithoutCount<NetPacketSet*>* jobQ_;
		NetPipeID curPipe_;
		NetRoot* server_;
		bool isMoving_;

		friend class NetPipe;
	};	


	inline void NetUser::Initialize(const NetSessionID ID, NetRoot* const server, const NetPipeID pipeID)
	{
		sessionID_ = ID;
		server_ = server;
		curPipe_ = pipeID;
		jobQ_ = server_->GetSessionJobQ(ID);
	}

	inline void NetUser::SendPacket(NetPacket* const packet)
	{
		server_->SendPacket_Async(sessionID_, packet);
	}

	inline void NetUser::SendPacket(NetSessionIDSet* const idSet, NetPacket* const packet)
	{
		server_->SendPacket_Async(idSet, packet);
	}

	inline bool NetUser::Disconnect()
	{
		server_->Disconnect(sessionID_);
	}

	inline void NetUser::DisconnectAfterLastMessage(NetPacket* const packet)
	{
		server_->DisconnectAfterLastMessage(sessionID_, packet);
	}

	inline void NetUser::DisconnectAfterLastMessage(NetSessionIDSet* const idSet, NetPacket* const packet)
	{
		server_->DisconnectAfterLastMessage(idSet, packet);
	}

	inline NetPacketSet* NetUser::DequeueJob()
	{
		if (isMoving_)
			return nullptr;

		NetPacketSet* set = nullptr;
		jobQ_->Dequeue(&set);

		return set;
	}

	inline void NetUser::ErrorQuit(const wchar_t* const msg)
	{

		SystemLogger::GetInstance()->Console(L"NetUser", LEVEL_SYSTEM, msg);
		SystemLogger::GetInstance()->LogText(L"NetUser", LEVEL_SYSTEM, msg);

		CrashDump::Crash();
	}
}

#endif