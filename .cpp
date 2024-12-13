#include <algorithm>
#include <cassert>
#include <iostream>
#include <cassert>
#include <cstring>
#include <functional>
#include <utility>

#define GlobalBucketNum 193

template <typename key_type, typename mapped_type>
class KeyValuePair
{
public:
    key_type key;
    mapped_type value;

    bool operator==(const KeyValuePair& to_cmp)
    {
        if (this == nullptr) return false;
        if (this->key == to_cmp.key && this->value == to_cmp.value)
            return true; 
        return false;
    }
    KeyValuePair& operator = (const KeyValuePair& other) {
        if (*this == other) {
            return *this;
        }
        this->key = other.key;
        this->value = other.value;
        return *this;
    }
    bool operator!=(const KeyValuePair& to_cmp)
    {
        return !operator==(to_cmp);
    }
    friend std::ostream& operator<<(std::ostream& os, KeyValuePair& it)
    {
        os << (it).key << "-" << (it).value;
        return os;
    }
};

template <typename key_type = size_t, typename mapped_type = int>
class MyHashMap
{
public:
    typedef KeyValuePair<key_type, mapped_type> value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    //typedef std::ptrdiff_t difference_type;
public:
    class Bucket
    {
    public:
        pointer data = nullptr;
        size_t size = 0;
        ~Bucket()
        {
            size = 0;
            if (data != nullptr)
            {
                delete[] data;
                data = nullptr;
            }
        }
    };

    // итераторы
    class iterator
    {
    public:
        typedef KeyValuePair<key_type, mapped_type> value_type;
        typedef value_type* pointer;
        typedef value_type& reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef std::forward_iterator_tag iterator_category;

    private:
        size_type key_value_pos = 0;
        MyHashMap::Bucket* currentBucket;
        MyHashMap::Bucket* lastBucket;

    public:
        iterator() = delete;
        iterator(const iterator&) = default;
        iterator(size_type key_value_pos_in, Bucket* bucket_in, Bucket* lastBucket_in)
            : key_value_pos(key_value_pos_in), currentBucket(bucket_in), lastBucket(lastBucket_in) {}

        value_type& operator*()
        {
            return currentBucket->data[key_value_pos];
        }
        void next()
        {
            if (key_value_pos + 1 < currentBucket->size)
            {
                key_value_pos++;
                return;
            }
            key_value_pos = 0;
            for (currentBucket++; currentBucket < lastBucket; currentBucket++)
            {
                if (currentBucket->size > 0)
                    return;
            }
            return;
        }
        //difference_type operator - (const iterator other) {
        //    difference_type diff = 0;

        //}
        iterator operator++(int)
        {
            auto tmp = *this;
            next();
            return tmp;
        }

        iterator& operator++()
        {
            next();
            return *this;
        }

        iterator& operator+=(difference_type n)
        {
            difference_type i = 0;
            while (i < n)
            {
                next();
                i++;
            }
            return *this;
        };

        friend iterator operator+(iterator it, difference_type n)
        {

            difference_type i = 0;
            while (i < n)
            {
                it.next();
                i++;
            }
            return it;
        };

        bool operator==(const iterator& iter)
        {
            return this->currentBucket == iter.currentBucket && this->key_value_pos == iter.key_value_pos;
        };
        bool operator!=(const iterator& iter)
        {
            return !operator==(iter);
        }
    };

    // мап филдс
private:
    size_t key_value_count;
    Bucket* bucket = nullptr;
    size_t bucket_count = GlobalBucketNum;

    // мап методы для итераторов

public:
    iterator begin()
    {
        if (bucket[0].size > 0)
            return iterator(0, &this->bucket[0], &this->bucket[bucket_count]);
        return ++iterator(0, &this->bucket[0], &this->bucket[bucket_count]);
    }

    iterator end()
    {
        return iterator(0, &this->bucket[bucket_count], &this->bucket[bucket_count]);
    }

    // конструкторы для мапа

    ~MyHashMap()
    {
        if (bucket != nullptr)
        {
            delete[] bucket;
            bucket = nullptr;
        }

        key_value_count = 0;
        bucket_count = 0;
    }
    MyHashMap() : key_value_count(0), bucket(new Bucket[GlobalBucketNum]) {}
    MyHashMap(size_t n) : key_value_count(n), bucket(new Bucket[GlobalBucketNum]) {}
    MyHashMap(const key_type a[], size_t n)
        : key_value_count(n), bucket(new Bucket[GlobalBucketNum])
    {
        auto numOfKeysInBuckets = new int[bucket_count];
        for (size_t i = 0; i < bucket_count; i++)
        {
            numOfKeysInBuckets[i] = 0;
        }

        for (size_t i = 0; i < n; i++)
        {
            numOfKeysInBuckets[hashFunction(i)]++;
        }
        for (size_t i = 0; i < bucket_count; i++)
        {
            if (numOfKeysInBuckets[i] > 0)
            {
                bucket[i].data = new value_type[numOfKeysInBuckets[i]];
            }
        }

        delete[] numOfKeysInBuckets;
        numOfKeysInBuckets = nullptr;
        for (size_t i = 0; i < n; i++)
        {
            size_t bucketId = hashFunction(i);
            bucket[bucketId].data[bucket[bucketId].size].key = i;
            bucket[bucketId].data[bucket[bucketId].size].value = a[i];
            bucket[bucketId].size += 1;
        }
    }
    MyHashMap(const MyHashMap& other)
        : key_value_count(other.key_value_count), bucket_count(other.bucket_count)
    {
        bucket = new Bucket[bucket_count];
        for (size_t i = 0; i < bucket_count; i++)
        {
            bucket[i].size = other.bucket[i].size;

            if (bucket[i].size > 0)
            {
                bucket[i].data = new value_type[bucket[i].size];
                std::copy(other.bucket[i].data, other.bucket[i].data + bucket[i].size, bucket[i].data);
            }
            else
            {
                bucket[i].data = nullptr;
            }
        }
    }
    MyHashMap(MyHashMap&& other) noexcept
    {
        key_value_count = other.key_value_count;
        bucket_count = other.bucket_count;
        bucket = std::exchange(other.bucket, nullptr);
        other.bucket_count = 0;
        other.key_value_count = 0;
    }

    MyHashMap(iterator begin, iterator end) : key_value_count(0), bucket(new Bucket[GlobalBucketNum])
    {
        for (; begin != end; begin++)
        {
            auto pair = *begin;
            insert(pair.key, pair.value);
            key_value_count++;
        }
    }

    MyHashMap(std::initializer_list<value_type> a)
        : bucket(new Bucket[GlobalBucketNum])

    {
        std::copy(a.begin(), a.end(), std::inserter(*this, begin()));
        key_value_count = a.size();
    }

    // хэш функция


    size_t hashFunction(key_type key)
    {
        return std::hash<key_type>()(key) % GlobalBucketNum;
    }

    // мап методы

    static bool is_equal(const MyHashMap& first, const MyHashMap& second)
    {
        if (first.bucket_count != second.bucket_count)
            return false;
        if (first.key_value_count != second.key_value_count)
            return false;
        auto tmpFirst = (MyHashMap)first;
        auto tmpSecond = (MyHashMap)second;
        for (iterator iteratorFirst = tmpFirst.begin(), iteratorSecond = tmpSecond.begin();
            iteratorFirst != tmpFirst.end(); iteratorFirst++, iteratorSecond++)
        {
            if ((*iteratorFirst).key != (*iteratorSecond).key || (*iteratorFirst).value != (*iteratorSecond).value)
                return false;
        }
        return true;
    }

    void clear()
    {
        for (size_t i = 0; i < bucket_count; ++i)
        {
            bucket[i].size = 0;
            if (bucket[i].data != nullptr)
            {
                delete[] bucket[i].data;
                bucket[i].data = nullptr;
            }
        }
        key_value_count = 0;
    }
    mapped_type at(key_type key)
    {
        size_t bucket_num = hashFunction(key);
        for (size_t i = 0; i < bucket[bucket_num].size; i++)
        {
            if (bucket[bucket_num].data[i].key == key)
                return bucket[bucket_num].data[i].value;
        }
        throw std::out_of_range{ "out of range" };
    }

    iterator find(key_type key)
    {
        size_t bucket_num = hashFunction(key);
        iterator it = *(new iterator(0, &bucket[bucket_num], &bucket[bucket_count]));
        while (it != end()) {
            if ((*it).key == key)
                return it;
            ++it;
        }
        std::cout << "element with key " << key << " not found in map" << std::endl;
        return end();
    }

    void erase(key_type key)
    {
        size_t bucket_for_this_key = hashFunction(key);
        for (size_t i = 0; i < bucket[bucket_for_this_key].size; i++)
        {
            if (bucket[bucket_for_this_key].data[i].key == key)
            {
                for (size_t j = i; j < bucket[bucket_for_this_key].size - 1; j++)
                {
                    bucket[bucket_for_this_key].data[j] = bucket[bucket_for_this_key].data[j + 1];
                }
                bucket[bucket_for_this_key].size--;
                key_value_count--;

                if (bucket[bucket_for_this_key].size > 0)
                {
                    pointer newData = new value_type[bucket[bucket_for_this_key].size];
                    std::copy(bucket[bucket_for_this_key].data, bucket[bucket_for_this_key].data + bucket[bucket_for_this_key].size, newData);
                    delete[] bucket[bucket_for_this_key].data;
                    bucket[bucket_for_this_key].data = newData;
                }
                else
                {
                    delete[] bucket[bucket_for_this_key].data;
                    bucket[bucket_for_this_key].data = nullptr;
                }
                return;
            }
        }
    }

    iterator erase(iterator& it) {
        erase((*(++it)).key);
        it.next();
        return it;
    }

    void insert_or_assign(key_type key, value_type v)
    {
        size_t current_bucket = hashFunction(key);
        for (size_t i = 0; i < bucket[current_bucket].size; i++)
        {
            if (bucket[current_bucket].data[i].key == key)
            {
                bucket[current_bucket].data[i].value = v;
                return;
            }
        }
        insert(key, v);
    }
    size_t length() { return key_value_count; }
    bool empty() { return !length(); }
    void swap(MyHashMap& other)
    {
        std::swap(bucket_count, other.bucket_count);
        std::swap(key_value_count, other.key_value_count);
        std::swap(bucket, other.bucket);
    }

    void print()
    {
        for (size_t i = 0; i < bucket_count; i++)
        {
            for (size_t j = 0; j < bucket[i].size; j++)
            {
                std::cout << "Key - Value : " << bucket[i].data[j].key << " - " << bucket[i].data[j].value << '\n';
            }
        }
    }
    void print_using_iterator()
    {
        for (auto&& it : *this)
        {
            std::cout << (it).key << "-" << (it).value << "\n";
        }
    }

    bool contains(key_type key)
    {
        size_t current_bucket = hashFunction(key);
        for (size_t i = 0; i < bucket[current_bucket].size; i++)
        {
            if (bucket[current_bucket].data[i].key == key)
                return true;
        }
        return false;
    }

    int count_key(key_type key) // если сравнивать по ключам, получается аналогично методу contains.
    {
        size_t current_bucket = hashFunction(key);
        for (size_t i = 0; i < bucket[current_bucket].size; i++)
        {
            if (bucket[current_bucket].data[i].key == key)
                return 1;
        }
        return 0;
    }

    size_t count_value(mapped_type value) // если сравнивать по значениям
    {
        size_t num = 0;
        for (size_t i = 0; i < bucket_count; i++)
        {
            for (size_t j = 0; j < bucket[i].size; j++)
            {
                if (bucket[i].data[j].value == value)
                {
                    num++;
                }
            }
        }
        return num;
    }


    void insert(key_type key, mapped_type value)
    {
        if (contains(key)) {
            return;
        }

        size_t current_bucket = hashFunction(key);
        bucket[current_bucket].size++;
        pointer tmp = new value_type[bucket[current_bucket].size];

        memcpy(tmp, bucket[current_bucket].data, sizeof(value_type) * (bucket[current_bucket].size - 1));

        delete[] bucket[current_bucket].data;
        bucket[current_bucket].data = nullptr;
        bucket[current_bucket].data = tmp;
        bucket[current_bucket].data[bucket[current_bucket].size - 1].key = key;
        bucket[current_bucket].data[bucket[current_bucket].size - 1].value = value;
        key_value_count++;
    }

    MyHashMap::iterator insert(MyHashMap::iterator it, const MyHashMap::value_type pair)
    {
        insert(pair.key, pair.value);
        it.next();
        return it;
    }


    void merge(MyHashMap& other) {
        for (auto it = other.begin(); it != other.end(); it++) { //итерация по элементам
            auto& pair = *it; //получаю пары ключ-значение
            if (this->contains(pair.key)) //проверяю,  есть ключ в в моей хэш-таблице
                continue; 
            insert(pair.key, pair.value); // встаавляю элемент в хэш таблицу
            other.erase(pair.key); // когда добавил, удаляю из other
        }
    }

    // перегруженные операторы

    MyHashMap& operator=(MyHashMap&& other) noexcept
    {
        if (this != &other)
        {
            delete[] bucket;
            bucket = nullptr;

            bucket_count = other.bucket_count;
            key_value_count = other.key_value_count;
            bucket = new Bucket[other.bucket_count];
            for (size_t i = 0; i < bucket_count; i++)
            {
                bucket[i].data = new value_type[other.bucket[i].size];
                bucket[i].size = other.bucket[i].size;
                for (size_t j = 0; j < bucket[i].size; j++)
                {
                    bucket[i].data[j].key = other.bucket[i].data[j].key;
                    bucket[i].data[j].value = other.bucket[i].data[j].value;
                }
            }
            delete[] other.bucket;
            other.bucket = nullptr;
            other.key_value_count = 0;
            other.bucket_count = 0;
        }
        return *this;
    }
    MyHashMap& operator=(const MyHashMap& other)
    {
        if (this != &other)
        {
            delete[] bucket;
            bucket = nullptr;

            bucket_count = other.bucket_count;
            key_value_count = other.key_value_count;
            bucket = new Bucket[other.bucket_count];
            for (size_t i = 0; i < bucket_count; i++)
            {
                bucket[i].data = new value_type[other.bucket[i].size];
                bucket[i].size = other.bucket[i].size;
                for (size_t j = 0; j < bucket[i].size; j++)
                {
                    bucket[i].data[j].key = other.bucket[i].data[j].key;
                    bucket[i].data[j].value = other.bucket[i].data[j].value;
                }
            }
        }
        return *this;
    }

    friend bool operator==(const MyHashMap& left, const MyHashMap& right)
    {
        if (left.bucket_count != right.bucket_count)
            return false;
        if (left.key_value_count != right.key_value_count)
            return false;
        for (size_t i = 0; i < left.bucket_count; i++)
        {
            if (left.bucket[i].size != right.bucket[i].size)
                return false;
            for (size_t j = 0; j < left.bucket[i].size; j++)
            {
                if (left.bucket[i].data[j].key != right.bucket[i].data[j].key)
                    return false;
                if (left.bucket[i].data[j].value != right.bucket[i].data[j].value)
                    return false;
            }
        }
        return true;
    }

    friend bool operator!=(const MyHashMap& left, const MyHashMap& right)
    {
        return !(left == right);
    }

    mapped_type& operator[](key_type key)
    {
        size_t current_bucket = hashFunction(key);
        if (contains(key))
        {
            for (size_t i = 0; i < bucket[current_bucket].size; i++)
            {
                if (bucket[current_bucket].data[i].key == key)
                    return bucket[current_bucket].data[i].value;
            }
        }
        insert(key, 0);
        return bucket[current_bucket].data[bucket[current_bucket].size - 1].value;
    }

    const mapped_type& operator[](key_type key) const
    {
        return operator[](key);
    }

    friend std::ostream& operator<<(std::ostream& os, const MyHashMap& map)
    {
        for (size_t i = 0; i < map.bucket_count; i++)
        {
            for (size_t j = 0; j < map.bucket[i].size; j++)
            {
                os << "Key - Value : " << map.bucket[i].data[j].key << " - " << map.bucket[i].data[j].value << '\n';
            }
        }
        return os;
    }

    friend std::istream& operator>>(std::istream& is, MyHashMap& map)
    {
        key_type key;
        mapped_type value;
        for (size_t i = 0; i < map.key_value_count; i++)
        {
            is >> key >> value;
            map.insert(key, value);
        }
        return is;
    }
};

int main()
{
    int array[] = { 19, 47, 74, 91 };
    MyHashMap a(array, 4);
    for (auto it = a.begin(); it != a.end(); ++it)
        std::cout << *it << " ";
    std::cout << '\n';

    MyHashMap <size_t, double>d({
        {3, 13.5}, {2, 74.158}, {1, 47.84}, {0, 19} });
    for (auto&& it : d) std::cout << it << " ";
    std::cout << std::endl;

    MyHashMap<std::string, std::string> colors({
        {"RED", "#FF0000"},
        {"GREEN", "#00FF00"},
        {"BLUE", "#0000FF"} });
    colors.print_using_iterator();
    std::cout << (*colors.find("BLUE")).value << std::endl;
    colors.insert("CYAN", "#00FFFF");
    colors.print();
    std::cout << std::endl;
    colors.erase("RED");
    colors.print();
    std::cout << std::endl;
    MyHashMap<std::string, std::string> shades_of_brown({
        {"ALMOND", "#EADDCA"},
        {"Brass", "#E1C16E"},
        {"Brown", "#A52A2A"} });
    colors.merge(shades_of_brown);
    colors.print();
    assert(colors.length() == 6);
}
