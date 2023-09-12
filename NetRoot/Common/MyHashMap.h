
#ifndef __HASH__MAP__
#define __HASH__MAP__

namespace server_baby
{

    constexpr int TABLE_SIZE = 65535;

    template <typename Key, typename Value>
    class HashNode 
    {
    public:
        HashNode(const Key& key, const Value& value) :
            key_(key), value_(value), next_(nullptr) {}

        Key GetKey() const { return key_;}
    
    public:
        HashNode* next_;
        Value value_;

    private:
        Key key_;

    };

    template <typename Key>
    struct KeyHash 
    {
        unsigned long operator()(const Key& key) const
        {
            unsigned long keyCopy = static_cast<unsigned long>(key);
            return keyCopy % TABLE_SIZE;
        }
    };

    //MemTLS ²¸³Ö±â
    template <typename Key, typename Value, typename F = KeyHash<Key>>
    class MyHashMap 
    {
    public:
        MyHashMap() 
        {
            table_ = new HashNode<Key, Value>* [TABLE_SIZE]();
        }

        virtual ~MyHashMap() 
        {
            for (int i = 0; i < TABLE_SIZE; ++i) 
            {
                HashNode<Key, Value>* entry = table_[i];
                while (entry != NULL) 
                {
                    HashNode<Key, Value>* prev = entry;
                    entry = entry->next_;
                    delete prev;
                }
                table_[i] = NULL;
            }
            delete[] table_;
        }

        bool Get(const Key& key, Value& value) 
        {
            unsigned long hashValue = hashFunction_(key);
            HashNode<Key, Value>* entry = table_[hashValue];

            while (entry != NULL) 
            {
                if (entry->GetKey() == key) 
                {
                    value = entry->value_;
                    return true;
                }

                entry = entry->next_;
            }
            return false;
        }

        void Put(const Key& key, const Value& value) 
        {
            unsigned long hashValue = hashFunction_(key);
            HashNode<Key, Value>* prev = NULL;
            HashNode<Key, Value>* entry = table_[hashValue];

            while (entry != NULL && entry->GetKey() != key) 
            {
                prev = entry;
                entry = entry->next_;
            }

            if (entry == NULL) 
            {
                entry = new HashNode<Key, Value>(key, value);
                if (prev == NULL)
                    table_[hashValue] = entry;
                else
                    prev->next_ = entry;               
            }
            else 
                entry->value_ = value;
        }

        void Remove(const Key& key) 
        {
            unsigned long hashValue = hashFunction_(key);
            HashNode<Key, Value>* prev = NULL;
            HashNode<Key, Value>* entry = table_[hashValue];

            while (entry != NULL && entry->GetKey() != key) 
            {
                prev = entry;
                entry = entry->next_;
            }

            if (entry == NULL) 
                return;
            
            else 
            {
                if (prev == NULL)
                    table_[hashValue] = entry->next_;
                else 
                    prev->next_ = entry->next_;
                delete entry;
            }
        }

    private:
        HashNode<Key, Value>** table_;
        F hashFunction_;
        

    };
}
#endif