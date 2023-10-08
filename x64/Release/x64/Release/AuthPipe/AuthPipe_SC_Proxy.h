#include "NetServer/NetPacket.h"
#include "NetServer/SessionID.h"
#include "AuthPipe.h"

namespace server_baby
{
	class AuthPipe_SC_Proxy
	{
	public:
		explicit AuthPipe_SC_Proxy(AuthPipe* server) : server_(nullptr)
		{
			server_ = server;
		}

		void ResLogin(BYTE status, INT64 accountNo, SessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)0;
			*msg << status;
			*msg << accountNo;

			server_->SendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

	private:
		AuthPipe* server_;
	};
}
