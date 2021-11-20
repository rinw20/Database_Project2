/*
 * extendible_hash.h : implementation of in-memory hash table using extendible
 * hashing
 *
 * Functionality: The buffer pool manager must maintain a page table to be able
 * to quickly map a PageId to its corresponding memory location; or alternately
 * report that the PageId does not match any currently-buffered page.
 */

#pragma once
#include <cstdlib>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include "hash/hash_table.h"

namespace scudb {

template <typename K, typename V>
class ExtendibleHash : public HashTable<K, V> {

public:
  // constructor
  ExtendibleHash(size_t size);
  // helper function to generate hash addressing
  size_t HashKey(const K &key) const;
  // helper function to get global & local depth
  int GetGlobalDepth() const;
  int GetLocalDepth(int bucket_id) const;
  int GetNumBuckets() const;
  // lookup and modifier
  bool Find(const K &key, V &value) override;
  bool Remove(const K &key) override;
  void Insert(const K &key, const V &value) override;
  class Bucket {
  public:
        Bucket(int depth):localDepth(depth) {};
        int localDepth;//局部位深度
        std::map<K, V> items;//map存放键值对K,V
  };

private:
  // add your own member variables here


    int bucketNumber(const K &key) const;

    int globalDepth;//全局位深度
    size_t bucketSize;
    int numBuckets;
    std::vector<std::shared_ptr<Bucket>> bucketTable;//存放指向bucket的容器，即索引表
    std::mutex mutex;
};
} // namespace scudb
