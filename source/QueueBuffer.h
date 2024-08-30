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

    void enqueue(const T* inputs, const int numInputs) {
        if (numInputs > max_size) {
            enqueue(inputs + (numInputs - max_size), max_size);
            return;
        }
        if (max_size - write_pos >= numInputs) {
            std::memcpy(&buffer[write_pos], inputs, numInputs * sizeof (T));
        } else {
            auto firstSegment = max_size - write_pos;
            std::memcpy(&buffer[write_pos], inputs, firstSegment * sizeof (T));
            std::memcpy(&buffer[0], inputs + firstSegment, numInputs - firstSegment * sizeof (T));
        }
        write_pos += numInputs;
        write_pos %= max_size;
    }

    int num_items() {
        return (write_pos + max_size - read_pos) % max_size;
    }
    
    T dequeue() {
        // If the queue is empty, we return a blank. (For this plugin, this is 0.0)
        if (read_pos == write_pos) {
            return null_option;
        }
        T result = buffer[read_pos];
        read_pos++;
        read_pos %= max_size;
        return result;
    }

    void clear() {
        std::fill(buffer.begin(), buffer.end(), null_option);
        read_pos = 0;
        write_pos = 0;
    }

private:
    T null_option;
    int read_pos;
    int write_pos;
    int max_size;
    std::vector<T> buffer;
};
