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

#include "string/string-hashed.hpp"
#include "queue/queue-shared.hpp"

namespace adam 
{
    /**
     * @class queue_shared_duplex
     * @brief Defines a bidirectional interprocess queue for any request/response datatype, based on shared memory
     */
    template<typename request_type, typename response_type>
    class ADAM_SDK_API queue_shared_duplex
    {
    public:

        /** @brief Constructs a new queue_shared_duplex object.*/
        queue_shared_duplex(const string_hashed& name);

        /** @brief Destroys the template<typename request_type, typename response_type> object and cleans up resources.*/
        ~queue_shared_duplex();

        queue_shared<request_type>&     request_queue()     { return m_request_queue;   }
        queue_shared<response_type>&    response_queue()    { return m_response_queue;  }

        /** @brief Gives the amount of items being able to be queued simoultanesously */
        uint32_t get_max_items() const { return m_request_queue.get_max_items(); }

        /** @brief Creates the queue_type queue and the underlying shared_memory for managing a max amount of items given */
        bool create(uint32_t max_items);

        /** @brief Opens an existing queue_type queue */
        bool open();

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
     :  m_request_queue(string_hashed(name + "_request")),
        m_response_queue(string_hashed(name + "_response"))
    {
    }

    template<typename request_type, typename response_type>
    queue_shared_duplex<request_type,response_type>::~queue_shared_duplex()
    {
        destroy();
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