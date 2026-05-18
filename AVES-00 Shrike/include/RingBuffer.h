#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>

template <typename T, size_t MaxSize>
class RingBuffer {
    private:
        T m_buffer[MaxSize];
        size_t m_head = 0;
        size_t m_len = 0;

    public:
        void push(T item) {
            m_buffer[m_head] = item;
            m_head = (m_head + 1) % MaxSize;
            if (m_len < MaxSize) {
                m_len++;
            }
        }

        size_t size() const {
            return m_len;
        }

        double getAverage() const {
            if (m_len == 0) return 0.0;
            
            double sum = 0;
            for (size_t i = 0; i < m_len; i++) {
                sum += m_buffer[i];
            }
            return sum / m_len;
        }
};

#endif