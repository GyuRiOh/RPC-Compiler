
#ifndef __LINKED__LIST__
#define __LINKED__LIST__

#include "MemTLS.h"

namespace server_baby
{
	template<typename DATA>
	struct ListNode
	{
		DATA data;
		ListNode* prev;
		ListNode* next;
	};

	template <typename DATA>
	class MyList
	{
	public:

		class iterator 
		{
			friend class MyList;
		private:
			ListNode<DATA>* cursor_; 
		public:
			iterator(ListNode<DATA>* node = nullptr)
			{
				//���ڷ� ���� Node �����͸� ����
				cursor_ = node;
			}

			iterator operator ++(int) //����
			{
				//���� ��带 ���� ���� �̵�
				iterator temp(cursor_); //nullptr�ȵ���.
				cursor_ = cursor_->next;

				return temp;
			}

			iterator& operator++()
			{
				cursor_ = cursor_->next;
				return *this;
			}

			iterator operator --(int)
			{
				iterator temp(cursor_); //nullptr�ȵ���.
				cursor_ = cursor_->prev;

				return temp;
			}

			iterator& operator--()
			{

				cursor_ = cursor_->prev;
				return *this;
			}

			DATA& operator *()
			{
				//���� ����� �����͸� ����
				return cursor_->data;

			}

			bool operator ==(const iterator& other)
			{
				return (cursor_ == other.cursor_);
			}

			bool operator !=(const iterator& other)
			{
				return (cursor_ != other.cursor_);
			}
		};


	public:
		MyList() : size_(0), head_(nullptr), tail_(nullptr)
		{
			head_ = listNodePool_->Alloc();
			tail_ = listNodePool_->Alloc();

			head_->next = tail_;
			tail_->prev = head_;
		}

		~MyList()
		{
			Clear();

			head_ = nullptr;
			tail_ = nullptr;
		}

		void DeleteNodePool()
		{
			delete listNodePool_->Alloc();
		}

		iterator begin() const
		{
			//ù��° ������ ��带 ����Ű�� ���ͷ����� ����
			return iterator(head_->next);

		}

		iterator end() const
		{
			//Tail ��带 ����Ű��(�����Ͱ� ���� ��¥ ���� �� ���) 
			//���ͷ����͸� ���� �Ǵ� ������ ������ �� �ִ� ���ͷ����͸� ����
			return iterator(tail_);
		}

		void Push_Front(DATA data) 
		{
			ListNode<DATA>* newNode = listNodePool_->Alloc();
			newNode->data = data;

			newNode->prev = head_;
			newNode->next = head_->next;

			head_->next->prev = newNode;
			head_->next = newNode;

			size_++;

		}

		void Push_Back(DATA data)
		{
			//���ϳ�� PREV�� INSERT
			ListNode<DATA>* newNode = listNodePool_->Alloc();
			newNode->data = data;

			newNode->prev = tail_->prev;
			newNode->next = tail_;

			tail_->prev->next = newNode;
			tail_->prev = newNode;

			size_++;
		}

		void Pop_Front(DATA* data)
		{
			//����� NEXT ��� ����
			ListNode<DATA>* removed;

			removed = head_->next;
			head_->next->next->prev = head_;
			head_->next = removed->next;

			*data = removed->data;
			listNodePool_->Free(removed);

			size_--;
		}

		void Pop_Back(DATA* data)
		{
			//���ϳ�� PREV ��� ����
			ListNode<DATA>* removed;

			removed = tail_->prev;
			tail_->prev->prev->next = tail_;
			tail_->prev = removed->prev;

			*data = removed->data;
			listNodePool_->Free(removed);

			size_--;
		}

		void Clear()
		{
			iterator iter;

			for (iter = begin(); iter != end();)
			{
				iter = Erase(iter);
			}

			size_ = 0;
		}

		int Size()
		{
			return size_;
		};

		bool isEmpty()
		{
			return (size_ == 0);
		};

		iterator Erase(iterator iter)
		{
			//���ͷ������� �� ��带 ����.
			//�׸��� ���� ����� ���� ��带 ī��Ű�� ���ͷ����� ����
			ListNode<DATA>* removed = iter.cursor_;

			iter.cursor_->prev->next = iter.cursor_->next;
			iter.cursor_->next->prev = iter.cursor_->prev;
			iter.cursor_ = iter.cursor_->next;

			listNodePool_->Free(removed);

			size_--;
			return iter;
		}

		void Remove(DATA Data)
		{
			iterator iter;

			for (iter = begin(); iter != end();)
			{
				if (*iter == Data)
					iter = Erase(iter);
				else
					++iter;
			}
		}

		int GetTotalUsedNode()
		{
			return listNodePool_->GetUseCount();
		}

		int GetTotalCapacity()
		{
			return listNodePool_->GetCapacityCount();
		}

	private:
		ListNode<DATA>* head_ = nullptr;
		ListNode<DATA>* tail_ = nullptr;
		int size_ = 0;

		static MemTLS<ListNode<DATA>>* listNodePool_;
	};

	template<typename DATA>
	MemTLS<ListNode<DATA>>* MyList<DATA>::listNodePool_ = new MemTLS<ListNode<DATA>>(100, 1, eLIST_NODE_CODE);
};

#endif