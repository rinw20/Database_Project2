/**
 * lru_replacer.h
 *
 * Functionality: The buffer pool manager must maintain a LRU list to collect
 * all the pages that are unpinned and ready to be swapped. The simplest way to
 * implement LRU is a FIFO queue, but remember to dequeue or enqueue pages when
 * a page changes from unpinned to pinned, or vice-versa.
 */

#pragma once

#include "buffer/replacer.h"
#include "hash/extendible_hash.h"
#include <map>
#include <mutex>

namespace scudb {

template <typename T> class LRUReplacer : public Replacer<T> {

public:
  // do not change public interface
  LRUReplacer();

  ~LRUReplacer();

  void Insert(const T &value);

  bool Victim(T &value);

  bool Erase(const T &value);

  size_t Size();


private:
  // add your member variables here
  //双向链表节点
  class ListNode
  {
  public:
      ListNode(T v):value(v){}
      T value;
      std::shared_ptr<ListNode> preNode;
      std::shared_ptr<ListNode> nextNode;
  };
    std::shared_ptr<ListNode> head;//头结点是最近被使用过的
    std::shared_ptr<ListNode> tail;//尾结点是最近未被使用过的
    std::mutex mutex;
    std::map<T,std::shared_ptr<ListNode>> index;//作为索引，不需要遍历链表
    int listLength=0;
};

} // namespace scudb
