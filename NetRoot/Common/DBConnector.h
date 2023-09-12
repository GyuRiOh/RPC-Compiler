#ifndef __DBCONNECTOR__
#define __DBCONNECTOR__

/////////////////////////////////////////////////////////
// MySQL DB ���� Ŭ����
//
// �ܼ��ϰ� MySQL Connector �� ���� DB ���Ḹ �����Ѵ�.
//
// �����忡 �������� �����Ƿ� ���� �ؾ� ��.
// ���� �����忡�� ���ÿ� �̸� ����Ѵٸ� ������ ��.
//
/////////////////////////////////////////////////////////

#include <mysql.h>
#define MAX_INFO_COUNT 3000

namespace server_baby
{

	class DBConnector
	{
		enum en_DB_CONNECTOR
		{
			eQUERY_MAX_LEN = 2048
		};

		struct ConnectorInfo
		{
			MYSQL MySQL_;
			MYSQL* pMySQL_ = nullptr;
			MYSQL_RES* SQLResult_;

			WCHAR		Query_[eQUERY_MAX_LEN] = { 0 };
			char		QueryUTF8_[eQUERY_MAX_LEN] = { 0 };

			int			lastError_ = NULL;
			WCHAR		lastErrorMsg_[128] = { 0 };
		};

	public:

		DBConnector(const WCHAR* szDBIP, const WCHAR* szUser, const WCHAR* szPassword, const WCHAR* szDBName, const int iDBPort);
		virtual		~DBConnector();
		bool		Connect(void);
		bool		Disconnect(void);
		bool		Query(const WCHAR* szStringFormat, ...);
		bool		Query_Save(const WCHAR* szStringFormat, ...);	// DBWriter �������� Save ���� ��

		MYSQL_ROW	FetchRow(void);
		void		FreeResult(void);
		int			GetLastError(void);
		WCHAR* GetLastErrorMsg(void);


	private:
		void SaveLastError(void);
		void SetTLSIndex(DWORD* index);
		void SetTLSInfo(ConnectorInfo* info);
		ConnectorInfo* GetTLSInfo();

	private:
		ConnectorInfo* infoArray_[MAX_INFO_COUNT] = { 0 };
		RTL_SRWLOCK initLock_;
		WCHAR DBIP_[16] = { 0 };
		WCHAR DBUser_[64] = { 0 };
		WCHAR DBPassword_[64] = { 0 };
		WCHAR DBName_[64] = { 0 };
		DWORD TLSIndex_;
		int	DBPort_;
		short infoArrayIndex_;

	};
}


#endif
#pragma once
