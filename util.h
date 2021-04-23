#pragma once

#include <string>


#ifdef TESTTOOL
#define ps_realloc realloc
#endif

template <typename T>
class PSVector {
    T *ptr = nullptr;
    size_t count = 0;
    size_t capacity = 0;

  public:
    PSVector() {}

    PSVector(const PSVector&) = delete;

    PSVector(PSVector&& a) noexcept {
        ptr = a.ptr;
        count = a.count;
        capacity = a.capacity;

        a.ptr = nullptr;
        a.count = 0;
        a.capacity = 0;
    }

    ~PSVector() noexcept {
        free(ptr);
    }

    void push_back(T item) {
        if (count == capacity) {
            capacity = (capacity * 3 / 2) + 1024;
            ptr = (T*)ps_realloc(ptr, capacity * sizeof(T));
        }

        ptr[count] = item;
        count++;
    }

    void pop_back() {
        count--;
    }

    void clear() {
        if (ptr) free(ptr);
        ptr = nullptr;
        count = capacity = 0;
    }

    size_t size() const {
        return count;
    }

    T& back() const {
        return ptr[count - 1];
    }

    T& operator[](size_t ind) const {
        return ptr[ind];
    }

    T *begin() const {
        return ptr;
    }

    T *end() const {
        return ptr + count;
    }
};



static std::string abbreviate(std::string &inp, int width) {
    auto found = inp.find_last_of('/');
    std::string temp = inp.substr(found+1);
    if (temp.size() > width) return temp.substr(0, width-3) + "...";
    else return temp;
}
