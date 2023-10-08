
#ifndef  __PACKET__
#define  __PACKET__
#include <Windows.h>
#include "Crash.h"/*
#include "Logger.h"
#include "Enums.h"
#include "MemTLS.h"*/

namespace server_baby
{
	class RPCMessage
	{	
	private:
		explicit RPCMessage();

		enum Setting
		{
			eINIT_FLAG = 0x15, 
			ePACKET_DEFAULT = 64,	// 패킷의 기본 버퍼 사이즈.
		};

	public:
		~RPCMessage();

		static RPCMessage* Alloc();
		static bool Free(RPCMessage* packet);

		int	DeqData(char* dest, int size); 
		int	EnqData(char* src, int size);

		inline long long GetMaxSize()
		{
			return ePACKET_DEFAULT;
		}

		inline long long GetUsedSize(void)
		{
			long long usedSize = writePos_ - readPos_;
			return usedSize;
		}

		inline long long GetEmptySize(void)
		{
			long long emptySize = end_ - writePos_;
			return emptySize;
		}

		inline char* GetStart(void) const
		{
			return start_;
		}

		inline char* GetReadPos(void) const
		{
			return readPos_;
		}

		inline char* GetWritePos(void) const
		{
			return writePos_;
		}

		inline void Clear(void)
		{
			readPos_ = writePos_ = start_;
		};

		//음수 이동 불가
		inline int MoveDataWritePos(int size)
		{
			isOkayToMoveWritePos(size);

			writePos_ += size;
			return size;
		}

		//inline static void DeletePacketPool()
		//{
		//	delete packetPool_;
		//}

		//inline static int GetUsedCount()
		//{
		//	return packetPool_->GetTotalUseCount();
		//}

		//inline static int GetCapacity()
		//{
		//	return packetPool_->GetTotalCapacity();
		//}


		/* ============================================================================= */
		// 연산자 오버로딩
		/* ============================================================================= */

		//////////////////////////////////////////////////////////////////////////
		// 넣기.	각 변수 타입마다 모두 만듬.
		//////////////////////////////////////////////////////////////////////////
		RPCMessage& operator << (unsigned char byValue);
		RPCMessage& operator << (char chValue);

		RPCMessage& operator << (short shValue);
		RPCMessage& operator << (unsigned short wValue);

		RPCMessage& operator << (int iValue);
		RPCMessage& operator << (long lValue);
		RPCMessage& operator << (DWORD dwValue);

		RPCMessage& operator << (float fValue);
		RPCMessage& operator << (__int64 iValue);
		RPCMessage& operator << (double dValue);


		//////////////////////////////////////////////////////////////////////////
		// 빼기.	각 변수 타입마다 모두 만듬.
		//////////////////////////////////////////////////////////////////////////
		RPCMessage& operator >> (BYTE& byValue);
		RPCMessage& operator >> (char& chValue);

		RPCMessage& operator >> (short& shValue);
		RPCMessage& operator >> (WORD& wValue);

		RPCMessage& operator >> (int& iValue);
		RPCMessage& operator >> (DWORD& dwValue);
		RPCMessage& operator >> (float& fValue);

		RPCMessage& operator >> (__int64& iValue);
		RPCMessage& operator >> (double& dValue);

	private:
		inline void Initialize()
		{
			if (initFlag_ != eINIT_FLAG)
			{
				start_ = &data_[0];
				end_ = &data_[ePACKET_DEFAULT - 1];
			}

			readPos_ = start_;
			writePos_ = start_;
			initFlag_ = eINIT_FLAG;
		}

		inline void isOkayToMoveWritePos(int size)
		{
			if (isFullWithWritePos(size))
				Error(L"MoveDataWritePos : Packet is full");
		}

		inline void isOkayToMoveReadPos(int size)
		{
			if (isEmptyWithReadPos(size))
				Error(L"MoveDataReadPos : Packet is Empty");
		}

		//음수 이동 불가
		inline int MoveDataReadPos(int size)
		{
			isOkayToMoveReadPos(size);

			readPos_ += size;
			return size;
		}

		inline bool isFullWithWritePos(int iSize) const
		{
			void* sum = writePos_ + iSize;
			if (sum >= end_)
				return true;
			else
				return false;
		}

		inline bool isEmptyWithReadPos(int iSize) const
		{
			void* sum = readPos_ + iSize;
			if (sum > writePos_)
				return true;
			else
				return false;
		}

		void Error(const WCHAR* message) const;



	private:
		char data_[ePACKET_DEFAULT] = { 0 };
		char* start_ = nullptr;
		char* end_ = nullptr;
		char* readPos_ = nullptr;
		char initFlag_ = NULL;

		alignas(64) char* writePos_ = nullptr;

		//static server_baby::MemTLS<Packet>* packetPool_;
		//friend struct server_baby::DataBlock<Packet>;

	};


}
#endif



