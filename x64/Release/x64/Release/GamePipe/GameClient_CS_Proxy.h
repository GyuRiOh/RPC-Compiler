#pragma once
#include "NetRoot/NetServer/NetPacket.h"
#include "GameClient.h"

namespace server_baby
{
	class GameClient_CS_Proxy final
	{
	public:
		explicit GameClient_CS_Proxy(GameClient* client) : client_(client) {}

		void ReqLogin(INT64 accountNo, char* sessionKey, int version, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(0);
			*msg << accountNo;
			msg->EnqData((char*)sessionKey, 64);
			*msg << version;

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

		void ReqCharacterSelect(BYTE type, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(1);
			*msg << type;

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

		void ReqMoveCharacter(INT64 clientID, float posX, float posY, USHORT rotation, BYTE vkey, BYTE hkey, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(2);
			*msg << clientID;
			*msg << posX;
			*msg << posY;
			*msg << rotation;
			*msg << vkey;
			*msg << hkey;

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

		void ReqStopCharacter(INT64 clientID, float posX, float posY, USHORT rotation, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(3);
			*msg << clientID;
			*msg << posX;
			*msg << posY;
			*msg << rotation;

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

		void ReqAttack1(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(4);
			*msg << clientID;

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

		void ReqAttack2(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(5);
			*msg << clientID;

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

		void ReqPick(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(6);
			*msg << clientID;

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

		void ReqSit(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(7);
			*msg << clientID;

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

		void ReqPlayerRestart(NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(8);

			client_->SendPacket(msg);
			NetPacket::Free(msg);
		}

	private:
		GameClient* client_;
	};
}
