#ifndef __RINGBUF_H__
#define __RINGBUF_H__

template<typename T, int S> class RingBuf
{
public:
  typedef T      value_type;

  RingBuf() { clear(); }
  ~RingBuf() {}
    
  inline size_t size()        const { return size_; }
  inline size_t capacity()    const { return S; }
  inline bool isFull()        const { return size_ >= S; }
  inline bool isEmpty()       const { return size_ <= 0; }
  inline bool hasElements()   const { return size_ >  0; }
    
  value_type& head() { return buffer_[head_]; }
  value_type& last() { return buffer_[last_]; }
    
  void clear() {
    size_ = 0;
    head_ = 0;
    last_ = S - 1;
  }
    
  void push(const value_type& x) {
    next(last_);
    if( size_ >= S )
      next(head_);
    else
      size_ = min(size_ + 1, S);
    last() = x;
  }
    
  value_type& pop() {
    if( size_ > 0  ) {
      size_ = max(size_ - 1, 0);
      next(head_);
    }
    return head();
  }

private:    
  value_type buffer_[S];    
  size_t size_;
  size_t head_;
  size_t last_;

  inline void next(size_t& x) { x = (x + 1) % S; }
};

#endif

