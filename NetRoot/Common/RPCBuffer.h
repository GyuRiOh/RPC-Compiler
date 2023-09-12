#pragma once
#include <stdlib.h>

namespace server_baby
{
	struct RPCBuffer
	{
		explicit RPCBuffer(INT64 size)
		{ 
			data = (char*)malloc(size);
		}
		~RPCBuffer()
		{
			free(data);
		}

		void* Data()
		{
			return data;
		}

		char* data = nullptr;
	};
}