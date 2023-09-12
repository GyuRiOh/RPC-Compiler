
#ifndef  __PACKET__
#define  __PACKET__
#include "Crash.h"
#include "SystemLogger.h"
#include "Enums.h"
#include "MemTLS_Revised.h"

namespace server_baby
{
	class NetPacket
	{	
	private:
		explicit NetPacket();

		enum en_PACKET
		{
			eINIT_FLAG = 0x15, 
			ePACKET_DEFAULT = 1024,	// 패킷의 기본 버퍼 사이즈.
			eDATA_DEFAULT = ePACKET_DEFAULT - eHEADER_SIZE,
			ePACKET_CODE = 0x77,
			eCERTAIN_KEY = 0x32,
			eCHECKSUM_CODE = 256
		};

	public:
		~NetPacket();

		static NetPacket* Alloc();
		static bool Free(NetPacket* packet);
		static void DeletePacketPool();
		static int GetUsedCount();
		static int GetCapacity();

		int	DeqData(char* dest, const int size); 
		int	EnqData(char* src, const int size);

		//음수 이동 불가
		int MoveBodyWritePos(const int size);
		bool Encode();
		void Clear();
		long AddRef();

		char* GetPacketStart();
		long long GetPacketUsedSize();

		NetPacket& operator << (const unsigned char byValue);
		NetPacket& operator << (const char chValue);

		NetPacket& operator << (const short shValue);
		NetPacket& operator << (const unsigned short wValue);

		NetPacket& operator << (const int iValue);
		NetPacket& operator << (const long lValue);
		NetPacket& operator << (const DWORD dwValue);

		NetPacket& operator << (const float fValue);
		NetPacket& operator << (const __int64 iValue);
		NetPacket& operator << (const double dValue);

		NetPacket& operator >> (BYTE& byValue);
		NetPacket& operator >> (char& chValue);

		NetPacket& operator >> (short& shValue);
		NetPacket& operator >> (WORD& wValue);

		NetPacket& operator >> (int& iValue);
		NetPacket& operator >> (DWORD& dwValue);
		NetPacket& operator >> (float& fValue);

		NetPacket& operator >> (__int64& iValue);
		NetPacket& operator >> (double& dValue);


	private:
		void Error(const WCHAR* message) const;
		void Initialize();

		void isOkayToMoveWritePos(const int size);
		void isOkayToMoveReadPos(const int size);

		//음수 이동 불가
		int MoveBodyReadPos(const int size);
		bool isFullWithWritePos(const int size) const;
		bool isEmptyWithReadPos(const int size) const;

	private:
		char data_[ePACKET_DEFAULT] = { 0 };
		RTL_SRWLOCK lock_;
		char* start_ = nullptr;
		char* end_ = nullptr;
		char* header_ = nullptr;		
		char* readPos_ = nullptr;
		char initFlag_ = NULL;
		bool encodeFlag_ = NULL;

		char padding[22] = { 0 };

		char* writePos_ = nullptr;
		long refCnt_ = 0;

		static server_baby::MemTLS<NetPacket>* packetPool_;
		friend class Session;
	};


	inline long NetPacket::AddRef()
	{
		return InterlockedIncrement(&refCnt_);
	}

	inline char* NetPacket::GetPacketStart()
	{
		return header_;
	}

	inline long long NetPacket::GetPacketUsedSize(void)
	{
		long long usedSize = writePos_ - readPos_ + eHEADER_SIZE;
		return usedSize;
	}

	inline void NetPacket::Clear(void)
	{
		readPos_ = writePos_ = start_;
	};

	inline void NetPacket::DeletePacketPool()
	{
		delete packetPool_;
	}

	inline int NetPacket::GetUsedCount()
	{
		return packetPool_->GetTotalUseCount();
	}

	inline int NetPacket::GetCapacity()
	{
		return packetPool_->GetTotalCapacity();
	}

	inline int NetPacket::DeqData(char* dest, int size)
	{
		isOkayToMoveReadPos(size);

		memmove(dest, readPos_, size);
		MoveBodyReadPos(size);
		return size;
	}

	inline int NetPacket::EnqData(char* src, int size)
	{
		isOkayToMoveWritePos(size);

		memmove(writePos_, src, size);
		MoveBodyWritePos(size);

		return size;
	}

	inline int server_baby::NetPacket::MoveBodyWritePos(int size)
	{
		isOkayToMoveWritePos(size);

		writePos_ += size;
		return size;
	}


	inline NetPacket* NetPacket::Alloc()
	{
		NetPacket* packet = packetPool_->Alloc();
		packet->Initialize();
		return packet;
	}

	inline bool NetPacket::Free(NetPacket* packet)
	{
		bool isFree = false;
		long refCnt = InterlockedDecrement(&packet->refCnt_);
		if (refCnt == 0)
			isFree = packetPool_->Free(packet);

		return isFree;
	}

	inline void NetPacket::Error(const WCHAR* message) const
	{
		SystemLogger::GetInstance()->LogText(L"Packet", LEVEL_SYSTEM, message);
		CrashDump::Crash();
	}

	inline void server_baby::NetPacket::Initialize()
	{
		if (initFlag_ != eINIT_FLAG)
		{
			InitializeSRWLock(&lock_);
			header_ = &data_[0];
			start_ = &data_[eHEADER_SIZE];
			end_ = &data_[ePACKET_DEFAULT - 1];
			refCnt_ = 0;
		}

		readPos_ = start_;
		writePos_ = start_;
		initFlag_ = eINIT_FLAG;
		refCnt_ = 1;
		encodeFlag_ = false;
	}

	inline void server_baby::NetPacket::isOkayToMoveWritePos(int size)
	{
		if (isFullWithWritePos(size))
			Error(L"MoveDataWritePos : Packet is full");
	}

	inline void server_baby::NetPacket::isOkayToMoveReadPos(int size)
	{
		if (isEmptyWithReadPos(size))
			Error(L"MoveDataReadPos : Packet is Empty");
	}

	inline int server_baby::NetPacket::MoveBodyReadPos(int size)
	{
		isOkayToMoveReadPos(size);

		readPos_ += size;
		return size;
	}

	inline bool server_baby::NetPacket::isFullWithWritePos(int iSize) const
	{
		void* sum = writePos_ + iSize;
		return (sum >= end_);
	}

	inline bool server_baby::NetPacket::isEmptyWithReadPos(int iSize) const
	{
		void* sum = readPos_ + iSize;
		return (sum >= writePos_);
	}

	inline NetPacket& NetPacket::operator<<(unsigned char byValue)
	{
		isOkayToMoveWritePos(sizeof(byValue));

		memmove(writePos_, &byValue, sizeof(byValue));
		MoveBodyWritePos(sizeof(byValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator<<(char chValue)
	{
		isOkayToMoveWritePos(sizeof(chValue));

		memmove(writePos_, &chValue, sizeof(chValue));
		MoveBodyWritePos(sizeof(chValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator<<(short shValue)
	{
		isOkayToMoveWritePos(sizeof(shValue));

		memmove(writePos_, &shValue, sizeof(shValue));
		MoveBodyWritePos(sizeof(shValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator<<(unsigned short wValue)
	{
		isOkayToMoveWritePos(sizeof(wValue));

		memmove(writePos_, &wValue, sizeof(wValue));
		MoveBodyWritePos(sizeof(wValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator<<(int iValue)
	{
		isOkayToMoveWritePos(sizeof(iValue));

		memmove(writePos_, &iValue, sizeof(iValue));
		MoveBodyWritePos(sizeof(iValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator<<(long lValue)
	{
		isOkayToMoveWritePos(sizeof(lValue));

		memmove(writePos_, &lValue, sizeof(lValue));
		MoveBodyWritePos(sizeof(lValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator<<(DWORD dwValue)
	{
		isOkayToMoveWritePos(sizeof(dwValue));

		memmove(writePos_, &dwValue, sizeof(dwValue));
		MoveBodyWritePos(sizeof(dwValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator<<(float fValue)
	{
		isOkayToMoveWritePos(sizeof(fValue));

		memmove(writePos_, &fValue, sizeof(fValue));
		MoveBodyWritePos(sizeof(fValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator<<(__int64 i64Value)
	{
		isOkayToMoveWritePos(sizeof(i64Value));

		memmove(writePos_, &i64Value, sizeof(i64Value));
		MoveBodyWritePos(sizeof(i64Value));
		return *this;
	}

	inline NetPacket& NetPacket::operator<<(double dValue)
	{
		isOkayToMoveWritePos(sizeof(dValue));

		memmove(writePos_, &dValue, sizeof(dValue));
		MoveBodyWritePos(sizeof(dValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator>>(BYTE& byValue)
	{
		isOkayToMoveReadPos(sizeof(byValue));

		memmove(&byValue, readPos_, sizeof(byValue));
		MoveBodyReadPos(sizeof(byValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator>>(char& chValue)
	{
		isOkayToMoveReadPos(sizeof(chValue));

		memmove(&chValue, readPos_, sizeof(chValue));
		MoveBodyReadPos(sizeof(chValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator>>(short& shValue)
	{
		isOkayToMoveReadPos(sizeof(shValue));

		memmove(&shValue, readPos_, sizeof(shValue));
		MoveBodyReadPos(sizeof(shValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator>>(WORD& wValue)
	{
		isOkayToMoveReadPos(sizeof(wValue));

		memmove(&wValue, readPos_, sizeof(wValue));
		MoveBodyReadPos(sizeof(wValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator>>(int& iValue)
	{
		isOkayToMoveReadPos(sizeof(iValue));

		memmove(&iValue, readPos_, sizeof(iValue));
		MoveBodyReadPos(sizeof(iValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator>>(DWORD& dwValue)
	{
		isOkayToMoveReadPos(sizeof(dwValue));

		memmove(&dwValue, readPos_, sizeof(dwValue));
		MoveBodyReadPos(sizeof(dwValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator>>(float& fValue)
	{
		isOkayToMoveReadPos(sizeof(fValue));

		memmove(&fValue, readPos_, sizeof(fValue));
		MoveBodyReadPos(sizeof(fValue));
		return *this;
	}

	inline NetPacket& NetPacket::operator>>(__int64& i64Value)
	{
		isOkayToMoveReadPos(sizeof(i64Value));

		memmove(&i64Value, readPos_, sizeof(i64Value));
		MoveBodyReadPos(sizeof(i64Value));
		return *this;
	}

	inline NetPacket& NetPacket::operator>>(double& dValue)
	{
		isOkayToMoveReadPos(sizeof(dValue));

		memmove(&dValue, readPos_, sizeof(dValue));
		MoveBodyReadPos(sizeof(dValue));
		return *this;
	}

}
#endif



