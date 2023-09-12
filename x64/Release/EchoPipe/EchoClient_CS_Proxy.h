#include "NetClient/NetPacket.h"
#include "NetClient/SessionID.h"
#include "EchoClient.h"

namespace server_baby
{
	class EchoClient_CS_Proxy
	{
	public:
		explicit EchoClient_CS_Proxy(EchoClient* client) : client_(nullptr)
		{
			client_ = client;
		}

		void ReqEcho(INT64 accountNo, LONGLONG sendTick, SessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)0;
			*msg << accountNo;
			*msg << sendTick;

			client_->SendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

	private:
		EchoClient* client_;
	};
}
