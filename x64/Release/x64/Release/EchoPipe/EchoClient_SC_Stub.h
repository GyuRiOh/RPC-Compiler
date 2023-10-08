#include <Windows.h>
#include "NetClient/SessionID.h"
#include "EchoClient.h"

namespace server_baby
{
	class EchoClient_SC_Stub
	{
	public:
		explicit EchoClient_SC_Stub(EchoClient* client) : client_(nullptr)
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
				INT64 accountNo;
				LONGLONG sendTick;
				*msg >> accountNo;
				*msg >> sendTick;
				ResEcho(accountNo, sendTick, sessionID);
			}
				return true;
			}
			return false;
		}

		void ResEcho(INT64 accountNo, LONGLONG sendTick, SessionID sessionID)
		{
		}

	private:
		EchoClient* client_;
	};
}
