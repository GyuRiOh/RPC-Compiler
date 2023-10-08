#include <Windows.h>
#include "NetClient/SessionID.h"
#include "MonitorClient_CS_Proxy.h"
#include "MonitorClient.h"

namespace server_baby
{
	class MonitorClient_SC_Stub
	{
	public:
		explicit MonitorClient_SC_Stub(MonitorClient* client) : client_(nullptr)
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
				*msg >> status;
				ResLogin(status, sessionID);
			}
				return true;
			case 1:
			{
				BYTE serverNo;
				BYTE dataType;
				int dataValue;
				int timeStamp;
				*msg >> serverNo;
				*msg >> dataType;
				*msg >> dataValue;
				*msg >> timeStamp;
				ResDataUpdate(serverNo, dataType, dataValue, timeStamp, sessionID);
			}
				return true;
			}
			return false;
		}

		void ResLogin(BYTE status, SessionID sessionID)
		{

		}

		void ResDataUpdate(BYTE serverNo, BYTE dataType, int dataValue, int timeStamp, SessionID sessionID)
		{

		}

	private:
		MonitorClient* client_;
	};
}
