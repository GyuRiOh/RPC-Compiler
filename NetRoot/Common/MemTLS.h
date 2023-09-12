
#ifndef  __MEM_TLS__
#define  __MEM_TLS__
#include "Crash.h"
#include "ChunkLockFree.h"
#include <vector>

namespace server_baby
{
	template<typename DATA>
	class MemTLS
	{
		struct Top
		{
			DataBlock<DATA>* data = nullptr;
			size_t index = NULL;
			short capacity = NULL;
		};

	public:
		explicit MemTLS(unsigned short chunkSize, unsigned short chunkNum, unsigned short code)
			:top_TLS_(NULL), code_(NULL), chunkSize_(NULL), dataSize_(NULL)
		{

			code_ = code;
			chunkSize_ = chunkSize;

			unsigned int size = sizeof(DATA);
			if (size > sizeof(void*))
				dataSize_ = size;
			else
				dataSize_ = sizeof(void*);

			if(!chunkPool_)
				chunkPool_ = new ChunkLockFree<DataBlock<DATA>>(chunkSize, chunkNum, code_);

			InitializeSRWLock(&topArrayLock_);

			SetTLSIndex(&top_TLS_);

		}

		~MemTLS()
		{
			Destroy();			
		}

		DATA* Alloc(void);
		bool Free(DATA* const pData);
		int GetTotalCapacity(void);
		int	GetTotalUseCount(void);
		void RemoveRemainders();
		void Destroy();
		void GarbageCollect();

	private:
		MemTLS() {}

		DataBlock<DATA>* AllocateNewChunk();
		void SetTLSIndex(DWORD* index);
		void SetTLSTop(Top* top);
		void ReturnDataToChunkPool();

		void Error(const WCHAR* message);
		void ErrorWithCode(const WCHAR* message);	

		Top* GetTLSTop_Alloc()
		{
			Top* top = reinterpret_cast<Top*>(TlsGetValue(top_TLS_));
			if (top != nullptr)
			{
				if (top->capacity == 0)
				{
					top->data = AllocateNewChunk();
					top->capacity = chunkSize_;
				}
				return top;
			}

			Top* newTop = new Top();
			newTop->data = AllocateNewChunk();
			newTop->capacity = chunkSize_;

			AcquireSRWLockExclusive(&topArrayLock_);			
			topArray_.push_back(move(newTop));
			newTop->index = topArray_.size() - 1;
			ReleaseSRWLockExclusive(&topArrayLock_);

			SetTLSTop(newTop);
			return newTop;
		}

		Top* GetTLSTop_Free()
		{
			Top* top = reinterpret_cast<Top*>(TlsGetValue(top_TLS_));
			if (top != nullptr)
				return top;

			Top* newTop = new Top();

			AcquireSRWLockExclusive(&topArrayLock_);
			topArray_.push_back(move(newTop));
			newTop->index = topArray_.size() - 1;
			ReleaseSRWLockExclusive(&topArrayLock_);

			SetTLSTop(newTop);

			return newTop;
		}

	private:
		vector<Top*> topArray_;
		DWORD top_TLS_;
		RTL_SRWLOCK topArrayLock_;
		unsigned int dataSize_;
		unsigned short chunkSize_;
		unsigned short code_;
		static ChunkLockFree<DataBlock<DATA>>* chunkPool_;
	};

	template<typename DATA>
	ChunkLockFree<DataBlock<DATA>>* MemTLS<DATA>::chunkPool_ = nullptr;

	template<typename DATA>
	inline DATA* MemTLS<DATA>::Alloc(void)
	{
		Top* top = GetTLSTop_Alloc();

		DataBlock<DATA>* retData = top->data;
		top->data = retData->next;
		top->capacity--;

		retData->underguard = UNDERGUARD;

		return &(retData->data);
	}

	template<typename DATA>
	bool MemTLS<DATA>::Free(DATA* const pData)
	{
		DataBlock<DATA>* block = nullptr;
		block = *(DataBlock<DATA>**)(((unsigned long long*)((char*)pData + dataSize_)) + 2);

		//����, ����÷ο� ���� üũ	
		if (block->overguard != OVERGUARD || block->underguard != UNDERGUARD)
			Error(L"Free : Overflow or Underflow");

		//�ڵ� Ȯ��
		if (block->code != code_)
			Error(L"Free : Code Check Failed");

		block->underguard = UNDERGUARD_AFTER_FREE;

		Top* top = GetTLSTop_Free();
		block->next = top->data;
		top->data = block;
		top->capacity++;

		if (top->capacity >= (chunkSize_ * 2))
			ReturnDataToChunkPool();

		return true;

	}

	template<typename DATA>
	inline int MemTLS<DATA>::GetTotalCapacity(void)
	{
		if (!chunkPool_)
			return 0;
		else
			return chunkPool_->GetBlockCapacity();
	}

	template<typename DATA>
	inline int MemTLS<DATA>::GetTotalUseCount(void)
	{
		if (!chunkPool_)
			return 0;
		else
			return chunkPool_->GetBlockUsed();
	}

	template<typename DATA>
	inline void MemTLS<DATA>::RemoveRemainders()
	{
		MemTLS<DATA>::Top* top = reinterpret_cast<Top*>(TlsGetValue(top_TLS_));
		if (!top)
			return;
			
		if (top->capacity)
		{
			DataBlock<DATA>* head = top->data;
			DataBlock<DATA>* tail = head;

			for (int i = 0; i < (top->capacity - 1); i++)
			{
				tail = tail->next;
			}

			chunkPool_->FreeFromTLSDestruction(head, tail, top->capacity);		
		}	

		AcquireSRWLockExclusive(&topArrayLock_);
		delete topArray_[top->index];
		topArray_[top->index] = nullptr;
		ReleaseSRWLockExclusive(&topArrayLock_);

		SetTLSTop(nullptr);
	}

	template<typename DATA>
	inline void MemTLS<DATA>::Destroy()
	{
		AcquireSRWLockExclusive(&topArrayLock_);
		for (int i = 0; i < topArray_.size(); i++)
		{
			if (topArray_[i])
			{
				MemTLS<DATA>::Top* top = topArray_[i];
				DataBlock<DATA>* head = top->data;
				DataBlock<DATA>* tail = head;

				for (int i = 0; i < (top->capacity - 1); i++)
				{
					tail = tail->next;
				}

				chunkPool_->FreeFromTLSDestruction(head, tail, top->capacity);

				delete topArray_[i];
				topArray_[i] = nullptr;
			}
		}
		ReleaseSRWLockExclusive(&topArrayLock_);

		delete chunkPool_;
		TlsFree(top_TLS_);

	}

	template<typename DATA>
	inline void MemTLS<DATA>::Error(const WCHAR* message)
	{
		SystemLogger::GetInstance()->LogText(L"MemTLS", LEVEL_SYSTEM, message);
		CrashDump::Crash();
	}

	template<typename DATA>
	inline void MemTLS<DATA>::ErrorWithCode(const WCHAR* message)
	{
		SystemLogger::GetInstance()->LogText(L"MemTLS",
			LEVEL_SYSTEM, L"%ws : error code - %d", message, GetLastError());

		CrashDump::Crash();
	}

	template<typename DATA>
	inline DataBlock<DATA>* MemTLS<DATA>::AllocateNewChunk()
	{
		return chunkPool_->Alloc();
	}

	template<typename DATA>
	inline bool isEmpty(DataBlock<DATA>* top)
	{
		return (top == nullptr);
	}

	template<typename DATA>
	inline void MemTLS<DATA>::SetTLSIndex(DWORD* index)
	{

		if (InterlockedCompareExchange(index, TlsAlloc(), NULL) == NULL)
		{
			if (*index == TLS_OUT_OF_INDEXES)
			{
				//TLS Alloc �Լ��� ��Ʈ �÷��� �迭�κ��� ���� ������ �÷��׸� ã�� ���ߴ�.
				Error(L"TLSAlloc Failed - OUT OF INDEX");
			}
		}
	}

	template<typename DATA>
	inline void MemTLS<DATA>::SetTLSTop(MemTLS<DATA>::Top* top)
	{
		BOOL retval = TlsSetValue(top_TLS_, top);

		if (retval == false)
			ErrorWithCode(L"TlsSetValue");
	}

	template<typename DATA>
	inline void MemTLS<DATA>::ReturnDataToChunkPool()
	{
		MemTLS<DATA>::Top* top = GetTLSTop_Alloc();
		DataBlock<DATA>* head = top->data;
		DataBlock<DATA>* tail = head;

		for (int i = 0; i < (chunkSize_ - 1); i++)
		{
			tail = tail->next;
		}

		top->data = tail->next;
		tail->next = nullptr;

		chunkPool_->Free(head, tail);
		top->capacity -= chunkSize_;
	}

	template<typename DATA>
	inline void MemTLS<DATA>::GarbageCollect()
	{
		chunkPool_->GarbageCollect();
	}
}
#endif