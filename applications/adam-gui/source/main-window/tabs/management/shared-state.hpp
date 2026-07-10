#pragma once

/**
 * @file    shared-state.hpp
 * @author  dexus1337
 * @brief   Header containing structural and helper declarations for management tab component components.
 * @version 1.0
 * @date    12.06.2026
 */

#include <adam-sdk.hpp>
#include <imgui.h>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <utility>
#include <functional>

namespace adam::gui
{
    struct inject_buffer_state
    {
        std::string text_buffer;
        std::string last_parsed_text;
        std::vector<uint8_t> parsed_bytes;
        bool is_valid;

        inject_buffer_state() : is_valid(true)
        {
            text_buffer.resize(1024, '\0');
        }
    };

    struct connection_pin_data
    {
        ImVec2 pos;
        ImColor col;
        std::string format_name;
    };

    enum node_type : uint8_t
    {
        node_type_input,
        node_type_output,
        node_type_processor,
        node_type_converter
    };

    struct expanded_port_draw_info
    {
        node_type type;
        int stage;
        adam::string_hash port_hash;
        uint64_t unique_node_id;
        ImVec2 p_max;
        float current_node_w;
        ImColor captured_color;
        float this_expanded_h;
        adam::string_hash hash;
        float header_w;
    };

    struct port_display_info
    {
        std::string module_name;
        adam::string_hash module_hash;
        std::string port_name;
        adam::string_hash port_hash;
        adam::port::direction direction;
        std::string type_name;
        bool is_unavailable = false;
    };

    struct processor_display_info
    {
        std::string module_name;
        adam::string_hash module_hash;
        std::string proc_name;
        adam::string_hash proc_hash;
        bool is_filter;
        std::string type_name;
        bool is_unavailable = false;
    };

    struct DragProcessorPayload
    {
        adam::string_hash connection;
        adam::string_hash processor;
    };

    // Shared State Globals
    extern adam::string_hashed g_connection_to_delete;
    extern bool g_request_delete_popup;
    extern adam::string_hash g_port_to_delete_hash;
    extern adam::string_hash g_port_to_delete_conn_hash;
    extern bool g_port_to_delete_is_input;
    extern bool g_request_delete_port_popup;
    extern ImVec2 g_connection_drag_offset;

    extern adam::string_hashed g_target_connection;
    extern adam::port::direction g_target_direction;
    extern bool g_request_port_popup;
    extern char g_port_popup_title[adam::max_name_length];

    extern bool g_request_processor_popup;
    extern char g_processor_popup_title[adam::max_name_length];

    extern adam::string_hash g_active_drag_hash;
    extern size_t g_active_drag_target_index;

    extern bool g_is_dragging_processor;
    extern adam::string_hash g_dragged_processor_conn_hash;
    extern adam::string_hash g_dragged_processor_hash;
    extern int g_active_processor_drag_target_index;
    extern ImVec2 g_processor_drag_offset;
    extern int g_dragged_processor_original_index;

    extern std::unordered_set<uint64_t> g_expanded_nodes;
    extern std::unordered_set<uint64_t> g_expanded_inject_nodes;
    extern std::unordered_set<uint64_t> g_expanded_param_nodes;
    extern std::unordered_set<uint64_t> g_expanded_stats_nodes;
    extern std::unordered_set<adam::string_hash> g_expanded_inspector_ports;
    extern std::unordered_set<adam::string_hash> g_pending_inspector_ports;

    extern std::unordered_set<adam::string_hash> g_expanded_inspector_connections_input;
    extern std::unordered_set<adam::string_hash> g_pending_inspector_connections_input;
    
    extern std::unordered_set<adam::string_hash> g_expanded_inspector_connections_output;
    extern std::unordered_set<adam::string_hash> g_pending_inspector_connections_output;

    extern std::unordered_map<uint64_t, float> g_expanded_node_heights;
    
    extern bool g_request_open_inspector;
    extern adam::string_hash g_port_to_expand_in_inspector;
    extern adam::string_hash g_connection_input_to_expand_in_inspector;
    extern adam::string_hash g_connection_output_to_expand_in_inspector;

    extern std::map<uint64_t, inject_buffer_state> g_inject_data_buffers;

    extern std::vector<std::vector<connection_pin_data>> g_stage_pins_in_normal, g_stage_pins_out_normal;
    extern std::vector<std::vector<connection_pin_data>> g_stage_pins_in_preview, g_stage_pins_out_preview;

    // Helper functions
    static inline uint64_t get_unique_node_id(uint64_t port_hash, uint64_t conn_hash, int stage, node_type type)
    {
        uint64_t id = port_hash;
        id ^= (conn_hash + 0x9e3779b97f4a7c15ULL + (id << 6) + (id >> 2));
        id ^= (static_cast<uint64_t>(stage) + 0x517cc1b727220a95ULL + (id << 6) + (id >> 2));
        id ^= (static_cast<uint64_t>(type) + 0x123456789abcdefULL + (id << 6) + (id >> 2));
        if (static_cast<ImGuiID>(id) == 0)
            id ^= 0x1337;
        return id;
    }

    int inject_text_resize_callback(ImGuiInputTextCallbackData* data);
    std::pair<bool, std::vector<uint8_t>> parse_hex_bytes(const std::string& input);
    void fill_hex_preview
    (
        const uint8_t* data,
        size_t data_len,
        size_t max_bytes,
        char* hex_buf,
        size_t hex_buf_size,
        char* ascii_buf,
        size_t ascii_buf_size
    );
    void draw_direction_badge(adam::port::direction dir, bool is_used, adam::language lang);

    std::function<void(adam::buffer*)> make_inspector_buffer_callback(adam::string_hash port_hash);
    std::function<void(adam::buffer*)> make_inspector_connection_input_buffer_callback(adam::string_hash conn_hash);
    std::function<void(adam::buffer*)> make_inspector_connection_output_buffer_callback(adam::string_hash conn_hash);
}
