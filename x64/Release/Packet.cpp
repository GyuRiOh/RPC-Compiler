
#include "Packet.h"

using namespace server_baby;

//MemTLS<RPCMessage>* RPCMessage::packetPool_ = new MemTLS<RPCMessage>(1, ePACKET_POOL_CODE);

RPCMessage::RPCMessage()
{}

RPCMessage::~RPCMessage()
{}

RPCMessage& RPCMessage::operator<<(unsigned char byValue)
{
	isOkayToMoveWritePos(sizeof(byValue));

	memmove(writePos_, &byValue, sizeof(byValue));
	MoveDataWritePos(sizeof(byValue));
	return *this;
}

RPCMessage& RPCMessage::operator<<(char chValue)
{
	isOkayToMoveWritePos(sizeof(chValue));

	memmove(writePos_, &chValue, sizeof(chValue));
	MoveDataWritePos(sizeof(chValue));
	return *this;
}

RPCMessage& RPCMessage::operator<<(short shValue)
{
	isOkayToMoveWritePos(sizeof(shValue));

	memmove(writePos_, &shValue, sizeof(shValue));
	MoveDataWritePos(sizeof(shValue));
	return *this;
}

RPCMessage& RPCMessage::operator<<(unsigned short wValue)
{
	isOkayToMoveWritePos(sizeof(wValue));

	memmove(writePos_, &wValue, sizeof(wValue));
	MoveDataWritePos(sizeof(wValue));
	return *this;
}

RPCMessage& RPCMessage::operator<<(int iValue)
{
	isOkayToMoveWritePos(sizeof(iValue));

	memmove(writePos_, &iValue, sizeof(iValue));
	MoveDataWritePos(sizeof(iValue));
	return *this;
}

RPCMessage& RPCMessage::operator<<(long lValue)
{
	isOkayToMoveWritePos(sizeof(lValue));

	memmove(writePos_, &lValue, sizeof(lValue));
	MoveDataWritePos(sizeof(lValue));
	return *this;
}

RPCMessage& RPCMessage::operator<<(DWORD dwValue)
{
	isOkayToMoveWritePos(sizeof(dwValue));

	memmove(writePos_, &dwValue, sizeof(dwValue));
	MoveDataWritePos(sizeof(dwValue));
	return *this;
}

RPCMessage& RPCMessage::operator<<(float fValue)
{
	isOkayToMoveWritePos(sizeof(fValue));

	memmove(writePos_, &fValue, sizeof(fValue));
	MoveDataWritePos(sizeof(fValue));
	return *this;
}

RPCMessage& RPCMessage::operator<<(__int64 i64Value)
{
	isOkayToMoveWritePos(sizeof(i64Value));

	memmove(writePos_, &i64Value, sizeof(i64Value));
	MoveDataWritePos(sizeof(i64Value));
	return *this;
}

RPCMessage& RPCMessage::operator<<(double dValue)
{
	isOkayToMoveWritePos(sizeof(dValue));

	memmove(writePos_, &dValue, sizeof(dValue));
	MoveDataWritePos(sizeof(dValue));
	return *this;
}

RPCMessage& RPCMessage::operator>>(BYTE& byValue)
{
	isOkayToMoveReadPos(sizeof(byValue));

	memmove(&byValue, readPos_, sizeof(byValue));
	MoveDataReadPos(sizeof(byValue));
	return *this;
}

RPCMessage& RPCMessage::operator>>(char& chValue)
{
	isOkayToMoveReadPos(sizeof(chValue));

	memmove(&chValue, readPos_, sizeof(chValue));
	MoveDataReadPos(sizeof(chValue));
	return *this;
}

RPCMessage& RPCMessage::operator>>(short& shValue)
{
	isOkayToMoveReadPos(sizeof(shValue));

	memmove(&shValue, readPos_, sizeof(shValue));
	MoveDataReadPos(sizeof(shValue));
	return *this;
}

RPCMessage& RPCMessage::operator>>(WORD& wValue)
{
	isOkayToMoveReadPos(sizeof(wValue));

	memmove(&wValue, readPos_, sizeof(wValue));
	MoveDataReadPos(sizeof(wValue));
	return *this;
}

RPCMessage& RPCMessage::operator>>(int& iValue)
{
	isOkayToMoveReadPos(sizeof(iValue));

	memmove(&iValue, readPos_, sizeof(iValue));
	MoveDataReadPos(sizeof(iValue));
	return *this;
}

RPCMessage& RPCMessage::operator>>(DWORD& dwValue)
{
	isOkayToMoveReadPos(sizeof(dwValue));

	memmove(&dwValue, readPos_, sizeof(dwValue));
	MoveDataReadPos(sizeof(dwValue));
	return *this;
}

RPCMessage& RPCMessage::operator>>(float& fValue)
{
	isOkayToMoveReadPos(sizeof(fValue));

	memmove(&fValue, readPos_, sizeof(fValue));
	MoveDataReadPos(sizeof(fValue));
	return *this;
}

RPCMessage& RPCMessage::operator>>(__int64& i64Value)
{
	isOkayToMoveReadPos(sizeof(i64Value));

	memmove(&i64Value, readPos_, sizeof(i64Value));
	MoveDataReadPos(sizeof(i64Value));
	return *this;
}

RPCMessage& RPCMessage::operator>>(double& dValue)
{
	isOkayToMoveReadPos(sizeof(dValue));

	memmove(&dValue, readPos_, sizeof(dValue));
	MoveDataReadPos(sizeof(dValue));
	return *this;
}

int RPCMessage::DeqData(char* chpDest, int iSize)
{
	isOkayToMoveReadPos(iSize);

	memmove(chpDest, readPos_, iSize);
	MoveDataReadPos(iSize);
	return iSize;
}

int RPCMessage::EnqData(char* chpSrc, int iSrcSize)
{
	isOkayToMoveWritePos(iSrcSize);

	memmove(writePos_, chpSrc, iSrcSize);
	MoveDataWritePos(iSrcSize);

	return iSrcSize;
}

//
//RPCMessage* RPCMessage::Alloc()
//{
//	RPCMessage* packet = packetPool_->Alloc();
//	packet->Initialize();
//	return packet;
//}
//
//bool RPCMessage::Free(RPCMessage* packet)
//{
//	return packetPool_->Free(packet);
//}
//
//void RPCMessage::Error(const WCHAR* message) const
//{
//	SystemLogger::GetInstance()->InputText(L"Packet", LEVEL_SYSTEM, message);
//	CrashDump::Crash();
//}
