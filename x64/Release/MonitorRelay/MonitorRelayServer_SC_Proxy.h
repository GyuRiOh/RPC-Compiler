#include "NetRoot/NetServer/NetPacket.h"
#include "NetRoot/NetServer/NetSessionID.h"
#include "MonitorRelayServer.h"

namespace server_baby
{
	class MonitorRelayServer_SC_Proxy
	{
	public:
		explicit MonitorRelayServer_SC_Proxy(MonitorRelayServer* server) : server_(nullptr)
		{
			server_ = server;
		}

		void ResLogin(BYTE status, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)0;
			*msg << status;

			server_->SendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ResDataUpdate(BYTE serverNo, BYTE dataType, int dataValue, int timeStamp, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)1;
			*msg << serverNo;
			*msg << dataType;
			*msg << dataValue;
			*msg << timeStamp;

			server_->SendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

	private:
		MonitorRelayServer* server_;
	};
}
