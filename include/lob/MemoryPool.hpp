#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept>

namespace lob {

template <typename T, size_t BlockSize = 100'000>
class MemoryPool {
public:
    MemoryPool() {
        grow();
    }

    ~MemoryPool() {
        for (auto* block : blocks_) {
            delete[] block;
        }
    }

    // Allocation en O(1) sans appel système
    T* allocate() noexcept {
        if (!free_list_) {
            grow();
        }
        Node* node = free_list_;
        free_list_ = free_list_->next;
        return reinterpret_cast<T*>(node);
    }

    // Recyclage en O(1)
    void deallocate(T* ptr) noexcept {
        auto* node = reinterpret_cast<Node*>(ptr);
        node->next = free_list_;
        free_list_ = node;
    }

private:
    union Node {
        alignas(alignof(T)) char storage[sizeof(T)];
        Node* next;
    };

    void grow() {
        auto* new_block = new Node[BlockSize];
        blocks_.push_back(new_block);
        for (size_t i = 0; i < BlockSize - 1; ++i) {
            new_block[i].next = &new_block[i + 1];
        }
        new_block[BlockSize - 1].next = free_list_;
        free_list_ = new_block;
    }

    Node* free_list_{nullptr};
    std::vector<Node*> blocks_;
};

} // namespace lob