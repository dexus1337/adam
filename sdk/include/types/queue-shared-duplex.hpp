#pragma once

/**
 * @file    queue-shared-duplex.hpp
 * @author  dexus1337
 * @brief   Defines a bidirectional interprocess queue for any request/response datatype, based on shared memory
 * @version 1.0
 * @date    28.04.2026
 */

 
#include "api/api.hpp"

#include <atomic>
#include <cstddef>

#include "types/string-hashed.hpp"
#include "types/queue-shared.hpp"

namespace adam 
{
    /**
     * @class queue_shared_duplex
     * @brief Defines a bidirectional interprocess queue for any request/response datatype, based on shared memory
     */
    template<typename request_type, typename response_type>
    class queue_shared_duplex
    {
    public:

        /** @brief Constructs a new queue_shared_duplex object.*/
        queue_shared_duplex(const string_hashed& name = string_hashed());

        /** @brief Destroys the queue_shared_duplex and cleans up resources.*/
        ~queue_shared_duplex();

        queue_shared<request_type>&     request_queue()     { return m_request_queue;   }
        queue_shared<response_type>&    response_queue()    { return m_response_queue;  }

        /** @brief Gives the amount of items being able to be queued simoultanesously */
        uint32_t get_max_items() const { return m_request_queue.get_max_items(); }

        /** @brief Gets the memorys active flag. Can be as loop condition for threads .*/
        bool is_active() const { return m_request_queue.is_active() && m_response_queue.is_active(); };

        /** @brief Sets the name of the queue */
        void set_name(const string_hashed& new_name);

        /** @brief Creates the queue and the underlying shared_memory for managing a max amount of items given */
        bool create(uint32_t max_items);

        /** @brief Opens an existing queue */
        bool open();

        /** @brief Unsets the active flag. Can be used to stop waiting threads. */
        void disable() { m_request_queue.disable(); m_response_queue.disable(); }

        /** @brief Destroys the queue and free all resources */
        bool destroy();

        /** @brief Pushes a request to the request queue and waits for the response */
        bool post_request(const request_type& req, response_type& resp, int32_t timeout = -1);

        /** @brief Pushes a request to the request queue and waits for the response . Compatible with std::chrono::duration */
        template <typename rep, typename period>
        bool post_request(const request_type& req, response_type& resp, std::chrono::duration<rep, period> timeout = std::chrono::duration<rep, period>::max()) { return post_request(req, resp, timeout == std::chrono::duration<rep, period>::max() ? -1 : std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()); }

    protected:

        queue_shared<request_type>  m_request_queue;
        queue_shared<response_type> m_response_queue;
    };

    template<typename request_type, typename response_type>
    queue_shared_duplex<request_type,response_type>::queue_shared_duplex(const string_hashed& name)
     :  m_request_queue(),
        m_response_queue()
    {
        set_name(name);
    }

    template<typename request_type, typename response_type>
    queue_shared_duplex<request_type,response_type>::~queue_shared_duplex()
    {
        destroy();
    }

    template<typename request_type, typename response_type>
    void queue_shared_duplex<request_type,response_type>::set_name(const string_hashed& new_name)
    { 
        m_request_queue.set_name(string_hashed(new_name + std::string("_request"))); 
        m_response_queue.set_name(string_hashed(new_name + std::string("_response"))); 
    }

    template<typename request_type, typename response_type>
    bool queue_shared_duplex<request_type,response_type>::create(uint32_t max_items)
    {
        if (!m_request_queue.create(max_items))
        {
            destroy();
            return false;
        }

        if (!m_response_queue.create(max_items))
        {
            destroy();
            return false;
        }
        
        return true;
    }

    template<typename request_type, typename response_type>
    bool queue_shared_duplex<request_type,response_type>::open()
    {
        return m_request_queue.open() && m_response_queue.open();
    }

    template<typename request_type, typename response_type>
    bool queue_shared_duplex<request_type,response_type>::destroy()
    {
        return m_request_queue.destroy() && m_response_queue.destroy();
    }

    template<typename request_type, typename response_type>
    bool queue_shared_duplex<request_type,response_type>::post_request(const request_type& req, response_type& resp, int32_t timeout)
    {
        if (!m_request_queue.push(req))
            return false;

        return m_response_queue.pop(resp, timeout);
    }

}
