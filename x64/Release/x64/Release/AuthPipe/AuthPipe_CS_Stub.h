#include <Windows.h>
#include "NetServer/SessionID.h"
#include "AuthPipe.h"

namespace server_baby
{
	class AuthPipe_CS_Stub
	{
	public:
		explicit AuthPipe_CS_Stub(AuthPipe* server) : server_(nullptr)
		{
			server_ = server;
		}

		bool PacketProc(SessionID sessionID, DummyPacket* msg)
		{
			WORD type;
			*msg >> type;
			switch (type)
			{
			case 0:
			{
				INT64 accountNo;
				char sessionKey[64] = {0};
				int version;
				*msg >> accountNo;
				msg->DeqData((char*)sessionKey, 64);
				*msg >> version;
				ReqLogin(accountNo, sessionKey, version, sessionID);
			}
				return true;
			}
			return false;
		}

		void ReqLogin(INT64 accountNo, char* sessionKey, int version, SessionID sessionID)
		{
		}

	private:
		AuthPipe* server_;
	};
}
