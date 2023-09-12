#include <Windows.h>
#include "NetRoot/NetServer/NetSessionID.h"
#include "NetRoot/LanServer/LanPacketSet.h"
#include "MonitorRelayServer_SC_Proxy.h"
#include "MonitorRelayServer.h"

namespace server_baby
{
	class MonitorRelayServer_CS_Stub
	{
	public:
		explicit MonitorRelayServer_CS_Stub(MonitorRelayServer* server) : server_(nullptr)
		{
			server_ = server;
		}

		
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
				int serverNo;
				*msg >> serverNo;
				ReqLogin(serverNo, sessionID);
			}
				return true;
			case 1:
			{
				BYTE dataType;
				int dataValue;
				*msg >> dataType;
				*msg >> dataValue;
				ReqDataUpdate(dataType, dataValue, sessionID);
			}
				return true;
			}
			return false;
		}

		void ReqLogin(int serverNo, NetSessionID sessionID)
		{

		}

		void ReqDataUpdate(BYTE dataType, int dataValue, NetSessionID sessionID)
		{

		}

	private:
		MonitorRelayServer* server_;
	};
}
