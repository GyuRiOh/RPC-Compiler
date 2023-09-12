#include "NetServer/NetPacket.h"
#include "NetServer/SessionID.h"
#include ".h"

namespace server_baby
{
	class _SC_Proxy
	{
	public:
		explicit _SC_Proxy(* server) : server_(nullptr)
		{
			server_ = server;
		}

	private:
		* server_;
	};
}
