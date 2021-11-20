#include <list>

#include "hash/extendible_hash.h"
#include "page/page.h"

namespace scudb {

/*
 * construcmtor
 * array_size: fixed array size for each bucket
 */
    template <typename K, typename V>
    ExtendibleHash<K, V>::ExtendibleHash(size_t size):globalDepth(0), bucketSize(size), numBuckets(0)
    {
        bucketTable.push_back(std::make_shared<Bucket>(0));
    }

/*
 * helper function to calculate the hashing address of input key
 */
    template <typename K, typename V>
    size_t ExtendibleHash<K,V>::HashKey(const K &key) const{
        return std::hash<K>()(key);
    }

/*
 * helper function to return global depth of hash table
 * NOTE: you must implement this function in order to pass test
 */
    template <typename K, typename V>
    int ExtendibleHash<K, V>::GetGlobalDepth() const {
        return globalDepth;//得到全局位深度
    }

/*
 * helper function to return local depth of one specific bucket
 * NOTE: you must implement this function in order to pass test
 */
    template <typename K, typename V>
    int ExtendibleHash<K, V>::GetLocalDepth(int i) const {
        return bucketTable[i]->localDepth;
    }

/*
 * helper function to return current number of bucket in hash table
 */
    template <typename K, typename V>
    int ExtendibleHash<K, V>::GetNumBuckets() const {
        return numBuckets;
    }

/*
 * lookup function to find value associate with input key
 */
    template <typename K, typename V>
    bool ExtendibleHash<K, V>::Find(const K &key, V &value) {
        std::lock_guard<std::mutex> guard(mutex);

        auto index = bucketNumber(key);
        std::shared_ptr<Bucket> bucket = bucketTable[index];
        if (bucket != nullptr && bucket->items.find(key) != bucket->items.end()) {
            value = bucket->items[key];
            return true;
        }
        return false;
    }

/*
 * delete <key,value> entry in hash table
 * Shrink & Combination is not required for this project
 */
    template <typename K, typename V>
    bool ExtendibleHash<K, V>::Remove(const K &key) {
        std::lock_guard<std::mutex> guard(mutex);

        auto index = bucketNumber(key);
        std::shared_ptr<Bucket> bucket = bucketTable[index];

        if (bucket == nullptr || bucket->items.find(key) == bucket->items.end()) {
            return false;
        }
        bucket->items.erase(key);
        return true;
    }

    template <typename K, typename V>
    int ExtendibleHash<K, V>::bucketNumber(const K &key) const {
        //
        return HashKey((K)key) & ((1 << globalDepth) - 1);

    }

/*
 * insert <key,value> entry in hash table
 * Split & Redistribute bucket when there is overflow and if necessary increase
 * global depth
 */
    template <typename K, typename V>
    void ExtendibleHash<K, V>::Insert(const K &key, const V &value) {
        auto index = bucketNumber(key);//得到hash后的结果指向的桶的序号
        std::shared_ptr<Bucket> targetBucket = bucketTable[index];
        std::lock_guard<std::mutex> guard(mutex);//多线程互斥锁
        while (targetBucket->items.size() == bucketSize) //待插入的桶已满
        {
            if (targetBucket->localDepth == globalDepth) //已达到最大，桶的局部位深度无法再增加
            {
                size_t length = bucketTable.size();
                for (size_t i = 0; i < length; i++) {
                    bucketTable.push_back(bucketTable[i]);
                }
                globalDepth++;//全局位深度增加
            }
            //分裂后，要判断新增的那一位是1还是0，分别放入1桶和0桶
            int newBit = 1 << targetBucket->localDepth;

            auto zeroBucket = std::make_shared<Bucket>(targetBucket->localDepth + 1);
            auto oneBucket = std::make_shared<Bucket>(targetBucket->localDepth + 1);
            for (auto item : targetBucket->items) //遍历目标桶中的元素
            {
                size_t hashkey = HashKey(item.first);
                if (hashkey & newBit)//新增的位数为1时
                {
                    oneBucket->items.insert(item);
                }
                else//相反为0时
                {
                    zeroBucket->items.insert(item);
                }
            }

            for (size_t i = 0; i < bucketTable.size(); i++) {
                if (bucketTable[i] == targetBucket) {
                    if (i & newBit) {
                        bucketTable[i] = oneBucket;
                    } else {
                        bucketTable[i] = zeroBucket;
                    }
                }
            }

            index = bucketNumber(key);
            targetBucket = bucketTable[index];
        } //end while

        targetBucket->items[key] = value;
    }

    template class ExtendibleHash<page_id_t, Page *>;
    template class ExtendibleHash<Page *, std::list<Page *>::iterator>;
// test purpose
    template class ExtendibleHash<int, std::string>;
    template class ExtendibleHash<int, std::list<int>::iterator>;
    template class ExtendibleHash<int, int>;
} // namespace cmudb