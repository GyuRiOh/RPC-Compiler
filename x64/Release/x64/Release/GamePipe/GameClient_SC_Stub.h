#pragma once
#include <Windows.h>
#include "NetRoot/NetServer/NetSessionID.h"
#include "NetRoot/NetServer/NetPacketSet.h"
#include "NetRoot/Common/RPCBuffer.h"
#include "GameClient_CS_Proxy.h"
#include "GameClient.h"

namespace server_baby
{
	class GameClient_SC_Stub final
	{
	public:
		explicit GameClient_SC_Stub(GameClient* client) : client_(client) {}

		bool PacketProc(NetPacketSet* msgPack)
		{
			switch (msgPack->GetType())
			{
			case eNET_RECVED_PACKET_SET:
			{
				while (msgPack->GetSize() > 0)
				{
					NetDummyPacket* packet = nullptr;
					if (msgPack->Dequeue(&packet) == false)
						CrashDump::Crash();

					if (!PacketProc(msgPack->GetSessionID(), packet))
						return false;
				}
				break;
			}
			default:
				CrashDump::Crash();
				break;
			}

			return true;
		}

		bool PacketProc(NetSessionID sessionID, NetDummyPacket* msg)
		{
			WORD type;
			*msg >> type;
			switch (type)
			{
			case 0:
			{
				BYTE status;
				INT64 accountNo;
				*msg >> status;
				*msg >> accountNo;
				return ResLogin(status, accountNo, sessionID);
			}
			case 1:
			{
				BYTE status;
				*msg >> status;
				return ResCharacterSelect(status, sessionID);
			}
			case 2:
			{
				INT64 clientID;
				BYTE charaType;
				float posX;
				float posY;
				USHORT rotation;
				int crystal;
				int HP;
				INT64 Exp;
				USHORT level;
				*msg >> clientID;
				*msg >> charaType;
				RPCBuffer nickNameBuf(20);
				msg->DeqData((char*)nickNameBuf.data, 20);
				WCHAR* nickName = (WCHAR*)nickNameBuf.Data();
				*msg >> posX;
				*msg >> posY;
				*msg >> rotation;
				*msg >> crystal;
				*msg >> HP;
				*msg >> Exp;
				*msg >> level;
				return ResCreateMyCharacter(clientID, charaType, nickName, posX, posY, rotation, crystal, HP, Exp, level, sessionID);
			}
			case 3:
			{
				INT64 clientID;
				BYTE charaType;
				float posX;
				float posY;
				USHORT rotation;
				USHORT level;
				BYTE respawn;
				BYTE sit;
				BYTE die;
				*msg >> clientID;
				*msg >> charaType;
				RPCBuffer nickNameBuf(20);
				msg->DeqData((char*)nickNameBuf.data, 20);
				WCHAR* nickName = (WCHAR*)nickNameBuf.Data();
				*msg >> posX;
				*msg >> posY;
				*msg >> rotation;
				*msg >> level;
				*msg >> respawn;
				*msg >> sit;
				*msg >> die;
				return ResCreateOtherCharacter(clientID, charaType, nickName, posX, posY, rotation, level, respawn, sit, die, sessionID);
			}
			case 4:
			{
				INT64 clientID;
				*msg >> clientID;
				return ResRemoveObject(clientID, sessionID);
			}
			case 5:
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
				return ResMoveCharacter(clientID, posX, posY, rotation, vkey, hkey, sessionID);
			}
			case 6:
			{
				INT64 clientID;
				float posX;
				float posY;
				USHORT rotation;
				*msg >> clientID;
				*msg >> posX;
				*msg >> posY;
				*msg >> rotation;
				return ResStopCharacter(clientID, posX, posY, rotation, sessionID);
			}
			case 7:
			{
				INT64 clientID;
				float posX;
				float posY;
				USHORT rotation;
				*msg >> clientID;
				*msg >> posX;
				*msg >> posY;
				*msg >> rotation;
				return ResMoveMonster(clientID, posX, posY, rotation, sessionID);
			}
			case 8:
			{
				INT64 clientID;
				*msg >> clientID;
				return ResAttack1(clientID, sessionID);
			}
			case 9:
			{
				INT64 clientID;
				*msg >> clientID;
				return ResAttack2(clientID, sessionID);
			}
			case 10:
			{
				INT64 clientID;
				*msg >> clientID;
				return ResMonsterAttack(clientID, sessionID);
			}
			case 11:
			{
				INT64 attackClientID;
				INT64 targetClientID;
				int damage;
				*msg >> attackClientID;
				*msg >> targetClientID;
				*msg >> damage;
				return ResDamage(attackClientID, targetClientID, damage, sessionID);
			}
			case 12:
			{
				INT64 monsterClientID;
				*msg >> monsterClientID;
				return ResMonsterDie(monsterClientID, sessionID);
			}
			case 13:
			{
				INT64 crystalClientID;
				BYTE crystalType;
				float posX;
				float posY;
				*msg >> crystalClientID;
				*msg >> crystalType;
				*msg >> posX;
				*msg >> posY;
				return ResCreateCrystal(crystalClientID, crystalType, posX, posY, sessionID);
			}
			case 14:
			{
				INT64 clientID;
				*msg >> clientID;
				return ResPick(clientID, sessionID);
			}
			case 15:
			{
				INT64 clientID;
				*msg >> clientID;
				return ResSit(clientID, sessionID);
			}
			case 16:
			{
				INT64 clientID;
				INT64 crystalClientID;
				int amountCrystal;
				*msg >> clientID;
				*msg >> crystalClientID;
				*msg >> amountCrystal;
				return ResPickCrystal(clientID, crystalClientID, amountCrystal, sessionID);
			}
			case 17:
			{
				INT64 hp;
				*msg >> hp;
				return ResPlayerHP(hp, sessionID);
			}
			case 18:
			{
				INT64 clientID;
				*msg >> clientID;
				return ResPlayerDie(clientID, sessionID);
			}
			case 19:
			{
				return ResPlayerRestart(sessionID);
			}
			}
			return false;
		}

		bool ResLogin(BYTE status, INT64 accountNo, NetSessionID sessionID)
		{
			return false;
		}

		bool ResCharacterSelect(BYTE status, NetSessionID sessionID)
		{
			return false;
		}

		bool ResCreateMyCharacter(INT64 clientID, BYTE charaType, WCHAR* nickName, float posX, float posY, USHORT rotation, int crystal, int HP, INT64 Exp, USHORT level, NetSessionID sessionID)
		{
			return false;
		}

		bool ResCreateOtherCharacter(INT64 clientID, BYTE charaType, WCHAR* nickName, float posX, float posY, USHORT rotation, USHORT level, BYTE respawn, BYTE sit, BYTE die, NetSessionID sessionID)
		{
			return false;
		}

		bool ResRemoveObject(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ResMoveCharacter(INT64 clientID, float posX, float posY, USHORT rotation, BYTE vkey, BYTE hkey, NetSessionID sessionID)
		{
			return false;
		}

		bool ResStopCharacter(INT64 clientID, float posX, float posY, USHORT rotation, NetSessionID sessionID)
		{
			return false;
		}

		bool ResMoveMonster(INT64 clientID, float posX, float posY, USHORT rotation, NetSessionID sessionID)
		{
			return false;
		}

		bool ResAttack1(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ResAttack2(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ResMonsterAttack(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ResDamage(INT64 attackClientID, INT64 targetClientID, int damage, NetSessionID sessionID)
		{
			return false;
		}

		bool ResMonsterDie(INT64 monsterClientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ResCreateCrystal(INT64 crystalClientID, BYTE crystalType, float posX, float posY, NetSessionID sessionID)
		{
			return false;
		}

		bool ResPick(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ResSit(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ResPickCrystal(INT64 clientID, INT64 crystalClientID, int amountCrystal, NetSessionID sessionID)
		{
			return false;
		}

		bool ResPlayerHP(INT64 hp, NetSessionID sessionID)
		{
			return false;
		}

		bool ResPlayerDie(INT64 clientID, NetSessionID sessionID)
		{
			return false;
		}

		bool ResPlayerRestart(NetSessionID sessionID)
		{
			return false;
		}

	private:
		GameClient* client_;
	};
}
