#pragma once

/**
 * @file    message-serializer.hpp
 * @author  dexus1337
 * @brief   Defines message serializer and deserializer utilities
 * @version 1.0
 * @date    13.06.2026
 */

#include <vector>
#include <algorithm>
#include <cstring>
#include <cstddef>
#include <cstdint>

namespace adam
{
    namespace detail
    {
        template<typename msg_type>
        struct message_serializer
        {
            std::vector<msg_type>& messages;
            size_t& msg_idx;
            size_t& unused_off;
            size_t& unused_size;

            message_serializer(std::vector<msg_type>& msgs, size_t& initial_idx, size_t& initial_off, size_t& initial_size)
                : messages(msgs), msg_idx(initial_idx), unused_off(initial_off), unused_size(initial_size) 
            {
            }

            void ensure_space(size_t needed_size) 
            {
                if (unused_size < needed_size) 
                {
                    messages[msg_idx].set_extended(true);
                    msg_idx++;
                    auto type = messages[0].get_type();
                    if (msg_idx >= messages.size()) 
                        messages.emplace_back(type);
                    unused_off = 0;
                    unused_size = msg_type::get_max_data_length();
                }
            }

            void write_bytes(const void* data, size_t size) 
            {
                const uint8_t* ptr = static_cast<const uint8_t*>(data);
                size_t remaining = size;
                while (remaining > 0) 
                {
                    if (unused_size == 0) 
                    {
                        messages[msg_idx].set_extended(true);
                        msg_idx++;
                        auto type = messages[0].get_type();
                        if (msg_idx >= messages.size()) 
                            messages.emplace_back(type);
                        unused_off = 0;
                        unused_size = msg_type::get_max_data_length();
                    }
                    size_t to_write = std::min(remaining, unused_size);
                    std::memcpy(messages[msg_idx].template data_as<uint8_t>() + unused_off, ptr, to_write);
                    unused_off += to_write;
                    unused_size -= to_write;
                    ptr += to_write;
                    remaining -= to_write;
                }
            }
            
            template<typename view_type>
            view_type* allocate_view() 
            {
                ensure_space(sizeof(view_type));
                view_type* view = reinterpret_cast<view_type*>(messages[msg_idx].template data_as<uint8_t>() + unused_off);
                unused_off += sizeof(view_type);
                unused_size -= sizeof(view_type);
                return view;
            }
        };

        template<typename MessageType>
        struct message_deserializer
        {
            const MessageType* messages;
            size_t max_messages;
            size_t& msg_idx;
            size_t& unused_off;
            size_t& unused_size;

            message_deserializer(const MessageType* msgs, size_t max_msgs, size_t& initial_idx, size_t& initial_off, size_t& initial_size)
                : messages(msgs), max_messages(max_msgs), msg_idx(initial_idx), unused_off(initial_off), unused_size(initial_size) 
            {
            }

            void read_bytes(void* dest, size_t size) 
            {
                uint8_t* ptr = static_cast<uint8_t*>(dest);
                size_t remaining = size;
                while (remaining > 0) 
                {
                    if (unused_size == 0) 
                    {
                        msg_idx++;
                        if (msg_idx >= max_messages) return;
                        unused_off = 0;
                        unused_size = MessageType::get_max_data_length();
                    }
                    size_t to_read = std::min(remaining, unused_size);
                    std::memcpy(ptr, messages[msg_idx].template get_data_as<uint8_t>() + unused_off, to_read);
                    unused_off += to_read;
                    unused_size -= to_read;
                    ptr += to_read;
                    remaining -= to_read;
                }
            }
            
            template<typename view_type>
            view_type read_view() 
            {
                view_type view;
                read_bytes(&view, sizeof(view_type));
                return view;
            }
        };
    }
}
