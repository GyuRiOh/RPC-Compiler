#include <Windows.h>
#include "NetServer/SessionID.h"
#include ".h"

namespace server_baby
{
	class _CS_Stub
	{
	public:
		explicit _CS_Stub(* server) : server_(nullptr)
		{
			server_ = server;
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
		* server_;
	};
}
