#pragma once
#include <Windows.h>
#include "NetRoot/NetServer/NetSessionID.h"
#include "NetRoot/NetServer/NetPacketSet.h"
#include "NetRoot/Common/RPCBuffer.h"
#include "NetRoot/NetServer/NetStub.h"
#include "GamePipe_SC_Proxy.h"
#include "GamePipe.h"

namespace server_baby
{
	class GamePipe_CS_Stub final : public NetStub
	{
	public:
		explicit GamePipe_CS_Stub(GamePipe* server) : server_(server) {}

		void OnRecv(NetPacketSet* msgPack) override
		{

		}

		void OnContentsUserJoin(NetSessionID sessionID) override
		{

		}

		void OnContentsUserLeave(NetSessionID sessionID) override
		{

		}

		void OnWorkerClientJoin(NetSessionID sessionID) override
		{

		}

		void OnWorkerClientLeave(NetSessionID sessionID) override
		{

		}

		bool PacketProc(NetSessionID sessionID, NetDummyPacket* msg) override
		{
			WORD type;
			*msg >> type;
			switch (type)
			{
			case 0:
			{
				INT64 accountNo;
				int version;
				*msg >> accountNo;
				RPCBuffer sessionKeyBuf(64);
				msg->DeqData((char*)sessionKeyBuf.data, 64);
				char* sessionKey = (char*)sessionKeyBuf.Data();
				*msg >> version;
				return ReqLogin(accountNo, sessionKey, version, sessionID);
			}
			case 1:
			{
				BYTE type;
				*msg >> type;
				return ReqCharacterSelect(type, sessionID);
			}
			case 2:
			{
				INT64 clientID;
				float posX;
				float posY;
				USHORT rotation;
				BYTE vkey;
				BYTE hkey;
				*msg >> clientID;
				*msg >> posX;
				*msg >> posY;
				*msg >> rotation;
				*msg >> vkey;
				*msg >> hkey;
				return ReqMoveCharacter(clientID, posX, posY, rotation, vkey, hkey, sessionID);
			}
			case 3:
			{
				INT64 clientID;
				float posX;
				float posY;
				USHORT rotation;
				*msg >> clientID;
				*msg >> posX;
				*msg >> posY;
				*msg >> rotation;
				return ReqStopCharacter(clientID, posX, posY, rotation, sessionID);
			}
			case 4:
			{
				INT64 clientID;
				*msg >> clientID;
				return ReqAttack1(clientID, sessionID);
			}
			case 5:
			{
				INT64 clientID;
				*msg >> clientID;
				return ReqAttack2(clientID, sessionID);
			}
			case 6:
			{
				INT64 clientID;
				*msg >> clientID;
				return ReqPick(clientID, sessionID);
			}
			case 7:
			{
				INT64 clientID;
				*msg >> clientID;
				return ReqSit(clientID, sessionID);
			}
			case 8:
			{
				return ReqPlayerRestart(sessionID);
			}
			}
			return false;
		}

		bool ReqLogin(INT64 accountNo, char* sessionKey, int version, NetSessionID sessionID)
		{
			return false;
		}

		bool ReqCharacterSelect(BYTE type, NetSessionID sessionID)
		{
			return false;
		}

		bool ReqMoveCharacter(INT64 clientID, float posX, float posY, USHORT rotation, BYTE vkey, BYTE hkey, NetSessionID sessionID)
		{
			return false;
		}

		bool ReqStopCharacter(INT64 clientID, float posX, float posY, USHORT rotation, NetSessionID sessionID)
		{
			return false;
		}

		bool ReqAttack1(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ReqAttack2(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ReqPick(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ReqSit(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ReqPlayerRestart(NetSessionID sessionID)
		{
			return false;
		}

	private:
		GamePipe* server_;
	};
}
