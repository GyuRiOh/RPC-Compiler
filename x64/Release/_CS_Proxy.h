#include "NetClient/NetPacket.h"
#include "NetClient/SessionID.h"
#include ".h"

namespace server_baby
{
	class _CS_Proxy
	{
	public:
		explicit _CS_Proxy(* client) : client_(nullptr)
		{
			client_ = client;
		}

	private:
		* client_;
	};
}
