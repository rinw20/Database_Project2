#include "buffer/lru_replacer.h"
#include "page/page.h"

namespace scudb {

    template <typename T> LRUReplacer<T>::LRUReplacer() {}

    template <typename T> LRUReplacer<T>::~LRUReplacer() {}

    template <typename T> void LRUReplacer<T>::insertAtHead(std::shared_ptr<ListNode> node) {
        if (node == nullptr) {
            return;
        }
        node->preNode = nullptr;
        node->nextNode = head;

        if (head != nullptr) {
            head->preNode = node;
        }
        head = node;
        if (tail == nullptr) {
            tail = node;
        }

        index[node->value] = node;
        listLength++;
    }

/*
 * Insert value into LRU
 */
    template <typename T> void LRUReplacer<T>::Insert(const T &value) {
        std::lock_guard<std::mutex> guard(mutex);
        auto getKey = index.find(value);
        //注意：要先判断该元素是否已经存在在链表中，如果在的话需要将其删除，再在表头添加！
        if (getKey != index.end()) //查询该值是否在链表中
        {
            erase(value);
        }
        auto newNode = std::make_shared<ListNode>(value);
        insertAtHead(newNode);
    }

/* If LRU is non-empty, pop the head member from LRU to argument "value", and
 * return true. If LRU is empty, return false
 */
    template <typename T> bool LRUReplacer<T>::Victim(T &value)//LRU算法计算并弹出最近久未被使用的元素
    {
        std::lock_guard<std::mutex> guard(mutex);

        if (listLength == 0) //链表为空，不删除
        {
            return false;
        }
        if (head == tail) {
            value = head->value;
            head = nullptr;
            tail = nullptr;

            index.erase(value);
            listLength--;
            return true;
        }
        value = tail->value;
        auto discard = tail;
        discard->preNode->nextNode = nullptr;
        tail = discard->preNode;
        discard->preNode = nullptr;

        index.erase(value);
        listLength--;
        return true;
    }

/*
 * Remove value from LRU. If removal is successful, return true, otherwise
 * return false
 */
    template <typename T> bool LRUReplacer<T>::Erase(const T &value) //删除掉某个元素
    {
        std::lock_guard<std::mutex> guard(mutex);

        return erase(value);
    }

    template <typename T> size_t LRUReplacer<T>::Size() { return listLength; }

    template <typename T> bool LRUReplacer<T>::erase(const T &value) {
        auto getKey = index.find(value);//查询该值是否在链表中
        if (getKey == index.end()) {
            return false;
        }

        auto currentNode = getKey->second;
        if(currentNode == head)//欲删除的结点是头结点
        {
            currentNode->nextNode->preNode = nullptr;
            head = currentNode->nextNode;
        }
        else if(currentNode==tail)//欲删除的结点是尾结点
        {
            currentNode->preNode->nextNode=nullptr;
            tail=currentNode->preNode;
        }
        else if(currentNode==head && currentNode==tail)//链表中只有一个元素
        {
            head=nullptr;
            tail=nullptr;
        }
        else
        {
            currentNode->nextNode->preNode=currentNode->preNode;
            currentNode->preNode->nextNode=currentNode->nextNode;
        }
        currentNode->preNode = nullptr;
        currentNode->nextNode = nullptr;

        index.erase(value);
        listLength--;
        return true;
    }

    template class LRUReplacer<Page *>;
// test only
    template class LRUReplacer<int>;

} // namespace cmudb