#include <Windows.h>
#include "NetServer/SessionID.h"
#include "EchoPipe.h"

namespace server_baby
{
	class EchoPipe_CS_Stub
	{
	public:
		explicit EchoPipe_CS_Stub(EchoPipe* server) : server_(nullptr)
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
				LONGLONG sendTick;
				*msg >> accountNo;
				*msg >> sendTick;
				ReqEcho(accountNo, sendTick, sessionID);
			}
				return true;
			}
			return false;
		}

		void ReqEcho(INT64 accountNo, LONGLONG sendTick, SessionID sessionID)
		{
		}

	private:
		EchoPipe* server_;
	};
}
