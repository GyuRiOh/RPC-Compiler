#include "NetClient/NetPacket.h"
#include "NetClient/SessionID.h"
#include "AuthClient.h"

namespace server_baby
{
	class AuthClient_CS_Proxy
	{
	public:
		explicit AuthClient_CS_Proxy(AuthClient* client) : client_(nullptr)
		{
			client_ = client;
		}

		void ReqLogin(INT64 accountNo, char* sessionKey, int version, SessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)0;
			*msg << accountNo;
			msg->EnqData((char*)sessionKey, 64);
			*msg << version;

			client_->SendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

	private:
		AuthClient* client_;
	};
}
