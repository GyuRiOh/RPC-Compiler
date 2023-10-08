#include <Windows.h>
#include "NetClient/SessionID.h"
#include "AuthClient.h"

namespace server_baby
{
	class AuthClient_SC_Stub
	{
	public:
		explicit AuthClient_SC_Stub(AuthClient* client) : client_(nullptr)
		{
			client_ = client;
		}

		bool PacketProc(SessionID sessionID, DummyPacket* msg)
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
				ResLogin(status, accountNo, sessionID);
			}
				return true;
			}
			return false;
		}

		void ResLogin(BYTE status, INT64 accountNo, SessionID sessionID)
		{
		}

	private:
		AuthClient* client_;
	};
}
