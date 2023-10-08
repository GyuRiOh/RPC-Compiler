#include "NetRoot/NetClient/NetPacket.h"
#include "NetRoot/NetClient/NetSessionID.h"
#include "MonitorClient.h"

namespace server_baby
{
	class MonitorClient_CS_Proxy
	{
	public:
		explicit MonitorClient_CS_Proxy(MonitorClient* client) : client_(nullptr)
		{
			client_ = client;
		}

		void ReqLogin(int serverNo, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)0;
			*msg << serverNo;

			client_->SendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

		void ReqDataUpdate(BYTE dataType, int dataValue, NetSessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)1;
			*msg << dataType;
			*msg << dataValue;

			client_->SendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

	private:
		MonitorClient* client_;
	};
}
