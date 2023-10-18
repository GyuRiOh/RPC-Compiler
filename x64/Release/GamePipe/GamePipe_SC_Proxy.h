#pragma once
#include "NetRoot/NetServer/NetPacket.h"
#include "NetRoot/NetServer/NetSessionID.h"
#include "GamePipe.h"

namespace server_baby
{
	class GamePipe_SC_Proxy final
	{
	public:
		explicit GamePipe_SC_Proxy(GamePipe* server) : server_(server) {}

		void ResLogin(BYTE status, INT64 accountNo, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(0);
			*msg << status;
			*msg << accountNo;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResLogin(BYTE status, INT64 accountNo, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(0);
			*msg << status;
			*msg << accountNo;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResCharacterSelect(BYTE status, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(1);
			*msg << status;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResCharacterSelect(BYTE status, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(1);
			*msg << status;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResCreateMyCharacter(INT64 clientID, BYTE charaType, WCHAR* nickName, float posX, float posY, USHORT rotation, int crystal, int HP, INT64 Exp, USHORT level, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(2);
			*msg << clientID;
			*msg << charaType;
			msg->EnqData((char*)nickName, 20);
			*msg << posX;
			*msg << posY;
			*msg << rotation;
			*msg << crystal;
			*msg << HP;
			*msg << Exp;
			*msg << level;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResCreateMyCharacter(INT64 clientID, BYTE charaType, WCHAR* nickName, float posX, float posY, USHORT rotation, int crystal, int HP, INT64 Exp, USHORT level, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(2);
			*msg << clientID;
			*msg << charaType;
			msg->EnqData((char*)nickName, 20);
			*msg << posX;
			*msg << posY;
			*msg << rotation;
			*msg << crystal;
			*msg << HP;
			*msg << Exp;
			*msg << level;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResCreateOtherCharacter(INT64 clientID, BYTE charaType, WCHAR* nickName, float posX, float posY, USHORT rotation, USHORT level, BYTE respawn, BYTE sit, BYTE die, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(3);
			*msg << clientID;
			*msg << charaType;
			msg->EnqData((char*)nickName, 20);
			*msg << posX;
			*msg << posY;
			*msg << rotation;
			*msg << level;
			*msg << respawn;
			*msg << sit;
			*msg << die;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResCreateOtherCharacter(INT64 clientID, BYTE charaType, WCHAR* nickName, float posX, float posY, USHORT rotation, USHORT level, BYTE respawn, BYTE sit, BYTE die, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(3);
			*msg << clientID;
			*msg << charaType;
			msg->EnqData((char*)nickName, 20);
			*msg << posX;
			*msg << posY;
			*msg << rotation;
			*msg << level;
			*msg << respawn;
			*msg << sit;
			*msg << die;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResCreateMonsterCharacter(INT64 clientID, float posX, float posY, USHORT rotation, BYTE respawn, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(4);
			*msg << clientID;
			*msg << posX;
			*msg << posY;
			*msg << rotation;
			*msg << respawn;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResCreateMonsterCharacter(INT64 clientID, float posX, float posY, USHORT rotation, BYTE respawn, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(4);
			*msg << clientID;
			*msg << posX;
			*msg << posY;
			*msg << rotation;
			*msg << respawn;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResRemoveObject(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(5);
			*msg << clientID;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResRemoveObject(INT64 clientID, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(5);
			*msg << clientID;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResMoveCharacter(INT64 clientID, float posX, float posY, USHORT rotation, BYTE vkey, BYTE hkey, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(6);
			*msg << clientID;
			*msg << posX;
			*msg << posY;
			*msg << rotation;
			*msg << vkey;
			*msg << hkey;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResMoveCharacter(INT64 clientID, float posX, float posY, USHORT rotation, BYTE vkey, BYTE hkey, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(6);
			*msg << clientID;
			*msg << posX;
			*msg << posY;
			*msg << rotation;
			*msg << vkey;
			*msg << hkey;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResStopCharacter(INT64 clientID, float posX, float posY, USHORT rotation, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(7);
			*msg << clientID;
			*msg << posX;
			*msg << posY;
			*msg << rotation;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResStopCharacter(INT64 clientID, float posX, float posY, USHORT rotation, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(7);
			*msg << clientID;
			*msg << posX;
			*msg << posY;
			*msg << rotation;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResMoveMonster(INT64 clientID, float posX, float posY, USHORT rotation, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(8);
			*msg << clientID;
			*msg << posX;
			*msg << posY;
			*msg << rotation;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResMoveMonster(INT64 clientID, float posX, float posY, USHORT rotation, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(8);
			*msg << clientID;
			*msg << posX;
			*msg << posY;
			*msg << rotation;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResAttack1(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(9);
			*msg << clientID;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResAttack1(INT64 clientID, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(9);
			*msg << clientID;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResAttack2(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(10);
			*msg << clientID;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResAttack2(INT64 clientID, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(10);
			*msg << clientID;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResMonsterAttack(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(11);
			*msg << clientID;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResMonsterAttack(INT64 clientID, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(11);
			*msg << clientID;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResDamage(INT64 attackClientID, INT64 targetClientID, int damage, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(12);
			*msg << attackClientID;
			*msg << targetClientID;
			*msg << damage;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResDamage(INT64 attackClientID, INT64 targetClientID, int damage, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(12);
			*msg << attackClientID;
			*msg << targetClientID;
			*msg << damage;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResMonsterDie(INT64 monsterClientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(13);
			*msg << monsterClientID;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResMonsterDie(INT64 monsterClientID, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(13);
			*msg << monsterClientID;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResCreateCrystal(INT64 crystalClientID, BYTE crystalType, float posX, float posY, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(14);
			*msg << crystalClientID;
			*msg << crystalType;
			*msg << posX;
			*msg << posY;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResCreateCrystal(INT64 crystalClientID, BYTE crystalType, float posX, float posY, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(14);
			*msg << crystalClientID;
			*msg << crystalType;
			*msg << posX;
			*msg << posY;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResPick(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(15);
			*msg << clientID;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResPick(INT64 clientID, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(15);
			*msg << clientID;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResSit(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(16);
			*msg << clientID;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResSit(INT64 clientID, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(16);
			*msg << clientID;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResPickCrystal(INT64 clientID, INT64 crystalClientID, int amountCrystal, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(17);
			*msg << clientID;
			*msg << crystalClientID;
			*msg << amountCrystal;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResPickCrystal(INT64 clientID, INT64 crystalClientID, int amountCrystal, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(17);
			*msg << clientID;
			*msg << crystalClientID;
			*msg << amountCrystal;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResPlayerHP(INT64 hp, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(18);
			*msg << hp;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResPlayerHP(INT64 hp, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(18);
			*msg << hp;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResPlayerDie(INT64 clientID, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(19);
			*msg << clientID;

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResPlayerDie(INT64 clientID, NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(19);
			*msg << clientID;

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

		void ResPlayerRestart(NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(20);

			server_->AsyncSendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResPlayerRestart(NetSessionIDSet* sessionIDset)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << static_cast<unsigned short>(20);

			server_->AsyncSendPacket(sessionIDset, msg);
			NetPacket::Free(msg);
		}

	private:
		GamePipe* server_;
	};
}
