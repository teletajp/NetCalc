#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace buffers
{
template<std::size_t RING_SIZE, std::size_t ONE_ELEM_CAPACITY>
class strings_ring_t
{
    static_assert(RING_SIZE > 1, "Impossible create ring with size less then 2");
    std::size_t r_pos_;
    std::size_t w_pos_;
    std::size_t size_;
    std::size_t max_size_;
    bool overflow_;
    std::vector<std::string> ring_;
public:
    strings_ring_t():r_pos_(0), w_pos_(0), size_(0), max_size_(0), overflow_(false), ring_(RING_SIZE)
    {
        for(auto& el:ring_)
            el.reserve(ONE_ELEM_CAPACITY);
    };
    ~strings_ring_t() = default;
    /**
     * @brief swap free element of ring with new element
     * 
     * @param new_el new element
     * @return true 
     * @return false 
     */
    bool push(std::string&new_el)
    {
        if (overflow_) return false;
        if (max_size_ < new_el.size()) max_size_ = new_el.size();
        ring_[w_pos_].swap(new_el);
        w_pos_ = (w_pos_+1)%RING_SIZE;
        if (w_pos_ == r_pos_) overflow_ = true;
        ++size_;
        return true;
    }
    bool push(const std::string&new_el)
    {
        if (overflow_) return false;
        if (max_size_ < new_el.size()) max_size_ = new_el.size();
        ring_[w_pos_] = new_el;
        w_pos_ = (w_pos_+1)%RING_SIZE;
        if (w_pos_ == r_pos_) overflow_ = true;
        ++size_;
        return true;
    }
    /**
     * @brief swap oldest element in ring with parameter's element
     * 
     * @param el element to save value
     * @return true if ok
     * @return false if ring is empty
     * 
     * 
     */
    bool pop(std::string&el)
    {
        if (r_pos_ == w_pos_ && !overflow_)//empty
            return false;
        
        ring_[r_pos_].swap(el);
        ring_[r_pos_].clear();
        r_pos_ = (r_pos_+1)%RING_SIZE;
        if (overflow_)
            overflow_ = false;
        --size_;
        return true;
    }
    bool pop()
    {
        if (r_pos_ == w_pos_ && !overflow_)//empty
            return false;
        
        ring_[r_pos_].clear();
        r_pos_ = (r_pos_+1)%RING_SIZE;
        if (overflow_)
            overflow_ = false;
        --size_;
        return true;
    }
    void swap(strings_ring_t<RING_SIZE, ONE_ELEM_CAPACITY> &other)
    {
        std::swap(r_pos_, other.r_pos_);
        std::swap(w_pos_ ,other.w_pos_);
        std::swap(size_, other.size_);
        std::swap(max_size_ ,other.max_size_);
        std::swap(overflow_, other.overflow_);
        ring_.swap(other.ring_);
    }
    /**
     * @brief Reset ring to begining state
     * 
     */
    void reset()
    {
        r_pos_ = w_pos_ = size_ = max_size_ = 0;
        overflow_ = false;
        for(auto& el:ring_)
            el.reserve(ONE_ELEM_CAPACITY);
    }
    std::size_t size(){return size_;}
    bool empty(){return (r_pos_ == w_pos_ && !overflow_);}
    std::size_t max_size(){return max_size_;}
    strings_ring_t(const strings_ring_t& other) = delete;
    strings_ring_t(strings_ring_t&& other) = delete;
    const strings_ring_t& operator=(const strings_ring_t& other) = delete;
    const strings_ring_t& operator=(strings_ring_t&& other) = delete;
};
}