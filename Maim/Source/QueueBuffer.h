/*
  ==============================================================================

    QueueBuffer.h
    Created: 4 Apr 2023 9:24:24am
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <algorithm>
#include <vector>

// Simple Queue with fixed max-size. Not a super full implementation of a queue,
// but it does what I need for this specific task.

template <class T> class QueueBuffer {
public:
    QueueBuffer(const int siz, const T blank) {
        null_option = blank;
        max_size = siz;
        buffer.resize(siz);
        read_pos = 0;
        write_pos = 0;
    }
    
    void enqueue(const T input) {
        buffer[write_pos] = input;
        write_pos++;
        write_pos %= max_size;
        
        // If we over-fill the queue, we want to keep the most recent <max_size> elements,
        // while discarding the old ones.
        if (write_pos == read_pos) {
            read_pos++;
            read_pos %= max_size;
        }
    }
    
    int num_items() {
        return (write_pos + max_size - read_pos) % max_size;
    }
    
    T dequeue() {
        // If the queue is empty, we return a blank. (For Fish, this is 0.0)
        if (read_pos == write_pos) {
            return null_option;
        }
        T result = buffer[read_pos];
        read_pos++;
        read_pos %= max_size;
        return result;
    }

private:
    T null_option;
    int read_pos;
    int write_pos;
    int max_size;
    std::vector<T> buffer;
};
