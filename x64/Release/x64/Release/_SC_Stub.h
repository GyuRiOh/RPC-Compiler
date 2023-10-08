#include <Windows.h>
#include "NetClient/SessionID.h"
#include ".h"

namespace server_baby
{
	class _SC_Stub
	{
	public:
		explicit _SC_Stub(* client) : client_(nullptr)
		{
			client_ = client;
		}

		bool PacketProc(SessionID sessionID, DummyPacket* msg)
		{
			WORD type;
			*msg >> type;
			switch (type)
			{
			}
			return false;
		}

	private:
		* client_;
	};
}
