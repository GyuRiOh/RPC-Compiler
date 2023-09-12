#include "NetServer/NetPacket.h"
#include "NetServer/SessionID.h"
#include "EchoPipe.h"

namespace server_baby
{
	class EchoPipe_SC_Proxy
	{
	public:
		explicit EchoPipe_SC_Proxy(EchoPipe* server) : server_(nullptr)
		{
			server_ = server;
		}

		void ResEcho(INT64 accountNo, LONGLONG sendTick, SessionID sessionID)
		{
			NetPacket* msg = NetPacket::Alloc();

			*msg << (unsigned short)0;
			*msg << accountNo;
			*msg << sendTick;

			server_->SendPacket(sessionID, msg);
			NetPacket::Free(msg);
		}

	private:
		EchoPipe* server_;
	};
}
