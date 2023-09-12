
#ifndef __SINGLETON__
#define __SINGLETON__

#include <stdlib.h>

namespace server_baby
{

	template<typename DATA>
	class Singleton
	{
	protected:
		explicit Singleton() {}
		virtual ~Singleton() {}

		static DATA* instance_;
		static RTL_SRWLOCK mutex_;

	public:
		static DATA* GetInstance()
		{
			if (instance_)
				return instance_;

			AcquireSRWLockExclusive(&mutex_);
			if (instance_ == nullptr)
			{
				instance_ = (DATA*)malloc(sizeof(DATA));
				if (instance_ == nullptr)
				{
					int* p = nullptr;
					*p = 100;
				}
				//������ ��� ȣ���ϴ� �ʱ�ȭ �Լ�
				instance_->Initialize();
				atexit(DestroyInstance);
			}
			ReleaseSRWLockExclusive(&mutex_);

			return instance_;
		}

		//���ʿ� �ѹ� �� ģ���� ȣ���ؾ� �Ѵ�.
		static DATA* GetInstance(int logLevel)
		{
			if (instance_)
				return instance_;

			AcquireSRWLockExclusive(&mutex_);
			if (instance_ == nullptr)
			{
				instance_ = (DATA*)malloc(sizeof(DATA));

				if (instance_ == nullptr)
				{
					int* p = nullptr;
					*p = 100;
				}

				//������ ��� ȣ���ϴ� �ʱ�ȭ �Լ�
				instance_->Initialize(logLevel);
				atexit(DestroyInstance);
			}
			ReleaseSRWLockExclusive(&mutex_);

			return instance_;
		}

		static void DestroyInstance()
		{
			instance_->Destroy();
			free(instance_);
			instance_ = nullptr;
		}
	};

	template<typename DATA>
	DATA* Singleton<DATA>::instance_ = nullptr;

	template<typename DATA>
	RTL_SRWLOCK Singleton<DATA>::mutex_;
}

#endif