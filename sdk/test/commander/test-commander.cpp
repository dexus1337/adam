#include <gtest/gtest.h>
#include <commander/commander.hpp>
#include <controller/controller.hpp>
#include <commander/messages/command.hpp>
#include <commander/messages/event.hpp>
#include <commander/messages/message-structs.hpp>
#include <version/version.hpp>
#include <module/module.hpp>
#include <data/connection.hpp>
#include <data/port.hpp>
#include <data/port-types/port-internal.hpp>
#include <memory/buffer/buffer-manager.hpp>
#include <algorithm>
#include <commander/registry-view.hpp>
#include <configuration/parameters/configuration-parameter-list.hpp>
#include <configuration/parameters/configuration-parameter-integer.hpp>
#include <configuration/parameters/configuration-parameter-string.hpp>
#include <configuration/parameters/configuration-parameter-boolean.hpp>
#include <configuration/parameters/configuration-parameter-double.hpp>

using namespace adam::string_hashed_ct_literals;

namespace cmdr_test
{
    class mock_processor : public adam::processor
    {
    public:
        mock_processor(const adam::string_hashed& item_name)
            : adam::processor(item_name)
        {
        }

        const adam::string_hashed_ct& get_type_name() const override
        {
            static adam::string_hashed_ct type = "mock-processor-type"_ct;
            return type;
        }

        bool handle_data(adam::buffer*& buf) override
        {
            return true;
        }
    };

    class mock_processor_factory : public adam::factory<adam::processor>
    {
    public:
        std::unique_ptr<adam::processor> create(const adam::string_hashed& name) const override
        {
            return std::make_unique<mock_processor>(name);
        }
    };

    class mock_module : public adam::module
    {
    public:
        mock_module(const adam::string_hashed& name)
            : adam::module(name, adam::make_version(1, 0, 0), adam::sdk_version)
        {
        }

        void register_processor_factory(const adam::string_hashed& type_name, adam::factory<adam::processor>* factory_ptr)
        {
            m_processor_factories[type_name] = adam::registry::factory_data_processor(factory_ptr, nullptr, 0, 0, 0, 0);
        }
    };

    struct loaded_modules_tag_commander 
    {
        using type = adam::registry_module_manager::map_loaded_modules adam::registry_module_manager::*;
        friend type get_private(loaded_modules_tag_commander);
    };

    template <typename Tag, typename Tag::type Member>
    struct private_accessor_commander 
    {
        friend typename Tag::type get_private(Tag) 
        {
            return Member;
        }
    };
}

template struct cmdr_test::private_accessor_commander<cmdr_test::loaded_modules_tag_commander, &adam::registry_module_manager::m_loaded_modules>;

namespace cmdr_test
{
    class mock_module_injector
    {
    public:
        mock_module_injector(adam::registry& reg, adam::module* mod)
            : m_reg(reg), m_name(mod->get_name())
        {
            auto& loaded_modules = reg.modules().*get_private(loaded_modules_tag_commander{});
            loaded_modules.emplace(m_name, mod);
        }

        ~mock_module_injector()
        {
            auto& loaded_modules = m_reg.modules().*get_private(loaded_modules_tag_commander{});
            loaded_modules.erase(m_name);
        }

    private:
        adam::registry& m_reg;
        adam::string_hashed m_name;
    };

    static mock_processor_factory global_mock_proc_factory;
}

class commander_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
        // Ensure a completely clean state for the controller's registry across tests
        adam::controller::get().get_registry().clear();
        // Boot the controller asynchronously before each test
        adam::controller::get().run(true);
    }

    void TearDown() override
    {
        adam::controller::get().get_registry().clear();
        // Tear down the master queue and clean up shared memory
        adam::controller::get().destroy();
        // Force a complete wipe of the buffer memory pools AFTER all background threads have exited and dumped their caches
        adam::buffer_manager::get().destroy();
    }
};

/** @brief Tests commander is_active after a successful connection. */
TEST_F(commander_test, is_active_on_success)
{
    adam::commander cmdr;
    
    EXPECT_FALSE(cmdr.is_active());
    
    // Connect to the controller through the master queue
    EXPECT_TRUE(cmdr.connect());
    EXPECT_TRUE(cmdr.is_active());
    
    // Disconnect and clean up
    EXPECT_TRUE(cmdr.destroy());
    EXPECT_FALSE(cmdr.is_active());
}

/** @brief Tests if a single commander can receive an event broadcasted by the controller. */
TEST_F(commander_test, event_broadcast_and_receive)
{
    adam::commander cmdr;
    
    std::atomic<bool> event_received{false};
    adam::event_type received_type = adam::event_type::invalid;

    cmdr.dispatcher().register_handler(adam::event_type::language_changed, [&](const adam::event* events, size_t, adam::event_context&)
    {
        received_type = events[0].get_type();
        event_received = true;
    });

    ASSERT_TRUE(cmdr.connect());

    adam::event evt(adam::event_type::language_changed);
    adam::controller::get().broadcast_event(evt);

    // Wait for event to propagate across the IPC queue
    auto start = std::chrono::steady_clock::now();
    while (!event_received && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_TRUE(event_received.load());
    EXPECT_EQ(received_type, adam::event_type::language_changed);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the full flow of setting port parameters and receiving the updated states via event. */
TEST_F(commander_test, request_port_parameter_set_flow)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Create a port in the controller's registry and add user parameters of all types
    adam::port* test_port = nullptr;
    EXPECT_EQ(ctrl.get_registry().create_port("param_test_port"_ct, adam::port_internal::type_name(), 0, &test_port), adam::registry::status_success);
    ASSERT_NE(test_port, nullptr);
    
    auto* user_params = test_port->parameters().get<adam::configuration_parameter_list>("user_parameters"_ct);

    ASSERT_NE(user_params, nullptr);

    user_params->add(std::make_unique<adam::configuration_parameter_integer>("test_int"_ct, 10));
    user_params->add(std::make_unique<adam::configuration_parameter_double>("test_double"_ct, 1.0));
    user_params->add(std::make_unique<adam::configuration_parameter_boolean>("test_bool"_ct, false));
    user_params->add(std::make_unique<adam::configuration_parameter_string>("test_str"_ct, "default"_ct));

    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    auto port_hash = ("param_test_port"_ct).get_hash();
    ASSERT_TRUE(cmdr.get_registry().get_ports().contains(port_hash));

    auto* pview = cmdr.get_registry().get_ports().at(port_hash).get();
    
    // Since we are using an internal port without a real factory for this test,
    // we must manually mirror the parameter structure into the commander's view
    pview->user_params.add(std::make_unique<adam::configuration_parameter_integer>("test_int"_ct, 10));
    pview->user_params.add(std::make_unique<adam::configuration_parameter_double>("test_double"_ct, 1.0));
    pview->user_params.add(std::make_unique<adam::configuration_parameter_boolean>("test_bool"_ct, false));
    pview->user_params.add(std::make_unique<adam::configuration_parameter_string>("test_str"_ct, "default"_ct));

    auto* c_int = pview->user_params.get<adam::configuration_parameter_integer>("test_int"_ct);
    ASSERT_NE(c_int, nullptr);
    EXPECT_EQ(c_int->get_value(), 10);
    
    auto* c_dbl = pview->user_params.get<adam::configuration_parameter_double>("test_double"_ct);
    ASSERT_NE(c_dbl, nullptr);
    EXPECT_DOUBLE_EQ(c_dbl->get_value(), 1.0);
    
    auto* c_bool = pview->user_params.get<adam::configuration_parameter_boolean>("test_bool"_ct);
    ASSERT_NE(c_bool, nullptr);
    EXPECT_EQ(c_bool->get_value(), false);
    
    auto* c_str = pview->user_params.get<adam::configuration_parameter_string>("test_str"_ct);
    ASSERT_NE(c_str, nullptr);
    EXPECT_EQ(c_str->get_value(), "default"_ct);

    // Request to change parameters
    EXPECT_EQ(cmdr.request_port_parameter_set(port_hash, "test_int"_ct, static_cast<int64_t>(42)), adam::response_status::success);
    EXPECT_EQ(cmdr.request_port_parameter_set(port_hash, "test_double"_ct, 3.14), adam::response_status::success);
    EXPECT_EQ(cmdr.request_port_parameter_set(port_hash, "test_bool"_ct, true), adam::response_status::success);
    EXPECT_EQ(cmdr.request_port_parameter_set(port_hash, "test_str"_ct, "updated"_ct), adam::response_status::success);

    // Wait for events to propagate over the IPC queue
    auto start = std::chrono::steady_clock::now();
    while ((c_int->get_value() == 10 || c_dbl->get_value() == 1.0 || c_bool->get_value() == false || c_str->get_value() == "default"_ct) && 
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(1000))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Verify commander's view is updated successfully
    EXPECT_EQ(c_int->get_value(), 42);
    EXPECT_DOUBLE_EQ(c_dbl->get_value(), 3.14);
    EXPECT_EQ(c_bool->get_value(), true);
    EXPECT_EQ(c_str->get_value(), "updated"_ct);

    // Verify controller's port parameter list is updated successfully
    auto* ctrl_user_params = dynamic_cast<adam::configuration_parameter_list*>(test_port->get_parameters().get("user_parameters"_ct));
    ASSERT_NE(ctrl_user_params, nullptr);
    EXPECT_EQ(ctrl_user_params->get<adam::configuration_parameter_integer>("test_int"_ct)->get_value(), 42);
    EXPECT_DOUBLE_EQ(ctrl_user_params->get<adam::configuration_parameter_double>("test_double"_ct)->get_value(), 3.14);
    EXPECT_EQ(ctrl_user_params->get<adam::configuration_parameter_boolean>("test_bool"_ct)->get_value(), true);
    EXPECT_EQ(ctrl_user_params->get<adam::configuration_parameter_string>("test_str"_ct)->get_value(), "updated"_ct);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the full flow of setting processor parameters and receiving the updated states via event. */
TEST_F(commander_test, request_processor_parameter_set_flow)
{
    adam::controller& ctrl = adam::controller::get();
    
    // 1. Setup mock module with mock processor factory
    cmdr_test::mock_module mock_mod("mock_mod"_ct);
    mock_mod.register_processor_factory("mock-converter"_ct, &cmdr_test::global_mock_proc_factory);
    cmdr_test::mock_module_injector injector(ctrl.get_registry(), &mock_mod);

    // 2. Create the processor in the controller's registry
    adam::processor* test_proc = nullptr;
    EXPECT_EQ(ctrl.get_registry().create_processor("param_test_proc"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &test_proc), adam::registry::status_success);
    ASSERT_NE(test_proc, nullptr);
    
    // 3. Add user parameters of all types
    auto* user_params = test_proc->parameters().get<adam::configuration_parameter_list>("user_parameters"_ct);
    ASSERT_NE(user_params, nullptr);

    user_params->add(std::make_unique<adam::configuration_parameter_integer>("test_int"_ct, 10));
    user_params->add(std::make_unique<adam::configuration_parameter_double>("test_double"_ct, 1.0));
    user_params->add(std::make_unique<adam::configuration_parameter_boolean>("test_bool"_ct, false));
    user_params->add(std::make_unique<adam::configuration_parameter_string>("test_str"_ct, "default"_ct));

    // 4. Connect commander
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    auto proc_hash = ("param_test_proc"_ct).get_hash();
    ASSERT_TRUE(cmdr.get_registry().get_processors().contains(proc_hash));

    auto* proc_view = cmdr.get_registry().get_processors().at(proc_hash).get();
    
    // Since we are using an internal mock without a real scan/scan_for_modules,
    // we must manually mirror the parameter structure into the commander's view
    proc_view->user_params.add(std::make_unique<adam::configuration_parameter_integer>("test_int"_ct, 10));
    proc_view->user_params.add(std::make_unique<adam::configuration_parameter_double>("test_double"_ct, 1.0));
    proc_view->user_params.add(std::make_unique<adam::configuration_parameter_boolean>("test_bool"_ct, false));
    proc_view->user_params.add(std::make_unique<adam::configuration_parameter_string>("test_str"_ct, "default"_ct));

    auto* c_int = proc_view->user_params.get<adam::configuration_parameter_integer>("test_int"_ct);
    ASSERT_NE(c_int, nullptr);
    EXPECT_EQ(c_int->get_value(), 10);
    
    auto* c_dbl = proc_view->user_params.get<adam::configuration_parameter_double>("test_double"_ct);
    ASSERT_NE(c_dbl, nullptr);
    EXPECT_DOUBLE_EQ(c_dbl->get_value(), 1.0);
    
    auto* c_bool = proc_view->user_params.get<adam::configuration_parameter_boolean>("test_bool"_ct);
    ASSERT_NE(c_bool, nullptr);
    EXPECT_EQ(c_bool->get_value(), false);
    
    auto* c_str = proc_view->user_params.get<adam::configuration_parameter_string>("test_str"_ct);
    ASSERT_NE(c_str, nullptr);
    EXPECT_EQ(c_str->get_value(), "default"_ct);

    // 5. Request to change parameters
    EXPECT_EQ(cmdr.request_processor_parameter_set(proc_hash, "test_int"_ct, static_cast<int64_t>(42)), adam::response_status::success);
    EXPECT_EQ(cmdr.request_processor_parameter_set(proc_hash, "test_double"_ct, 3.14), adam::response_status::success);
    EXPECT_EQ(cmdr.request_processor_parameter_set(proc_hash, "test_bool"_ct, true), adam::response_status::success);
    EXPECT_EQ(cmdr.request_processor_parameter_set(proc_hash, "test_str"_ct, "updated"_ct), adam::response_status::success);

    // Wait for events to propagate over the IPC queue
    auto start = std::chrono::steady_clock::now();
    while ((c_int->get_value() == 10 || c_dbl->get_value() == 1.0 || c_bool->get_value() == false || c_str->get_value() == "default"_ct) && 
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(1000))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Verify commander's view is updated successfully
    EXPECT_EQ(c_int->get_value(), 42);
    EXPECT_DOUBLE_EQ(c_dbl->get_value(), 3.14);
    EXPECT_EQ(c_bool->get_value(), true);
    EXPECT_EQ(c_str->get_value(), "updated"_ct);

    // Verify controller's processor parameter list is updated successfully
    auto* ctrl_user_params = dynamic_cast<adam::configuration_parameter_list*>(test_proc->get_parameters().get("user_parameters"_ct));
    ASSERT_NE(ctrl_user_params, nullptr);
    EXPECT_EQ(ctrl_user_params->get<adam::configuration_parameter_integer>("test_int"_ct)->get_value(), 42);
    EXPECT_DOUBLE_EQ(ctrl_user_params->get<adam::configuration_parameter_double>("test_double"_ct)->get_value(), 3.14);
    EXPECT_EQ(ctrl_user_params->get<adam::configuration_parameter_boolean>("test_bool"_ct)->get_value(), true);
    EXPECT_EQ(ctrl_user_params->get<adam::configuration_parameter_string>("test_str"_ct)->get_value(), "updated"_ct);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the deserialization logic of user_parameters synced from the controller. */
TEST_F(commander_test, parameter_deserialization)
{
    // 1. Set up a target configuration_parameter_list with some baseline parameters
    adam::configuration_parameter_list user_params("user_parameters"_ct);
    user_params.add(std::make_unique<adam::configuration_parameter_integer>("test_int"_ct, 10));
    user_params.add(std::make_unique<adam::configuration_parameter_string>("test_str"_ct, "default"_ct));
    user_params.add(std::make_unique<adam::configuration_parameter_boolean>("test_bool"_ct, false));
    user_params.add(std::make_unique<adam::configuration_parameter_double>("test_double"_ct, 1.0));

    // 2. Construct serialized byte stream manually mimicking the controller payload
    std::vector<uint8_t> payload;
    auto append_bytes = [&](const void* data, size_t size) 
    {
        const uint8_t* ptr = static_cast<const uint8_t*>(data);
        payload.insert(payload.end(), ptr, ptr + size);
    };

    adam::configuration_parameter_integer::view int_view;
    int_view.var_type = adam::configuration_parameter::type_integer;
    int_view.name = ("test_int"_ct).get_hash();
    int_view.value = 42;
    append_bytes(&int_view, sizeof(int_view));

    adam::configuration_parameter_string::view str_view;
    str_view.var_type = adam::configuration_parameter::type_string;
    str_view.name = ("test_str"_ct).get_hash();
    std::string str_val = "updated";
    str_view.length = static_cast<uint16_t>(str_val.length());
    append_bytes(&str_view, sizeof(str_view));
    append_bytes(str_val.c_str(), str_val.length());

    adam::configuration_parameter_boolean::view bool_view;
    bool_view.var_type = adam::configuration_parameter::type_boolean;
    bool_view.name = ("test_bool"_ct).get_hash();
    bool_view.value = true;
    append_bytes(&bool_view, sizeof(bool_view));

    adam::configuration_parameter_double::view dbl_view;
    dbl_view.var_type = adam::configuration_parameter::type_double;
    dbl_view.name = ("test_double"_ct).get_hash();
    dbl_view.value = 3.14;
    append_bytes(&dbl_view, sizeof(dbl_view));

    // 3. Fake the message architecture for the deserializer
    std::vector<adam::event> events;
    size_t offset = 0;
    while (offset < payload.size() || events.empty())
    {
        events.emplace_back(adam::event_type::port_created); 
        size_t available = adam::event::get_max_data_length();
        size_t to_copy = std::min(payload.size() - offset, available);
        if (to_copy > 0) std::memcpy(events.back().data_as<uint8_t>(), payload.data() + offset, to_copy);
        offset += to_copy;
        if (offset < payload.size()) events.back().set_extended(true);
    }

    // 4. Run the deserializer
    size_t evt_idx = 0;
    size_t unused_off = 0;
    size_t unused_size = adam::event::get_max_data_length();
    adam::detail::message_deserializer<adam::event> deserializer(events.data(), events.size(), evt_idx, unused_off, unused_size);
    adam::detail::deserialize_user_parameters(4, deserializer, user_params);

    // 5. Verify the updates applied correctly
    EXPECT_EQ(user_params.get<adam::configuration_parameter_integer>("test_int"_ct)->get_value(), 42);
    EXPECT_EQ(user_params.get<adam::configuration_parameter_string>("test_str"_ct)->get_value(), "updated"_ct);
    EXPECT_EQ(user_params.get<adam::configuration_parameter_boolean>("test_bool"_ct)->get_value(), true);
    EXPECT_DOUBLE_EQ(user_params.get<adam::configuration_parameter_double>("test_double"_ct)->get_value(), 3.14);
}

/** @brief Tests if a single commander can receive extended events correctly. */
TEST_F(commander_test, receive_extended_events)
{
    adam::commander cmdr;
    
    std::atomic<bool> event_received{false};
    size_t received_event_count = 0;

    const adam::event_type custom_evt_type = static_cast<adam::event_type>(9999);

    cmdr.dispatcher().register_handler(custom_evt_type, [&](const adam::event* events, size_t count, adam::event_context&)
    {
        (void)events;
        received_event_count = count;
        event_received = true;
    });

    ASSERT_TRUE(cmdr.connect());

    adam::event e1(custom_evt_type); e1.set_extended(true);
    adam::event e2(custom_evt_type); e2.set_extended(true);
    adam::event e3(custom_evt_type); e3.set_extended(false);

    adam::controller::get().broadcast_event(e1);
    adam::controller::get().broadcast_event(e2);
    adam::controller::get().broadcast_event(e3);

    auto start = std::chrono::steady_clock::now();
    while (!event_received && std::chrono::steady_clock::now() - start < std::chrono::seconds(2))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_TRUE(event_received.load());
    EXPECT_EQ(received_event_count, 3ull);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests initial data synchronization of modules between controller and commander. */
TEST_F(commander_test, initial_data_module_sync)
{
    adam::controller& ctrl = adam::controller::get();
    
    adam::commander cmdr;
    
    // Override the initial data handler to return some mock modules
    ctrl.dispatcher().register_handler(adam::command_type::acquire_initial_data, [](const adam::command*, size_t, adam::command_context& ctx)
    {
        ctx.set_single_response_status(adam::response_status::success);
        auto* data = ctx.responses.front().data_as<adam::messages::initial_data_header>();
        data->lang_info.lang = adam::language_english;
        
        data->mod_info.module_paths = 1;
        data->mod_info.available_modules = 1;
        data->mod_info.unavailable_modules = 1;
        data->mod_info.loaded_modules = 0;
        data->conn_info.ports = 0;
        data->conn_info.processors = 0;
        data->conn_info.connections = 1;
        
        // module paths
        ctx.responses.front().set_extended(true);
        ctx.responses.emplace_back();
        auto* path_info = ctx.responses[1].data_as<adam::messages::module_path_data>();
        path_info->setup("/mock/registry/path", 0);

        // available module
        ctx.responses[1].set_extended(true);
        ctx.responses.emplace_back();
        auto* mod_info1 = ctx.responses[2].data_as<adam::module::basic_info>();
        mod_info1->setup(adam::module::basic_info::available, "mock_avail", "/mock/path/avail.so", adam::make_version(1, 0, 0));
        
        // unavailable module
        ctx.responses[2].set_extended(true);
        ctx.responses.emplace_back();
        auto* mod_info2 = ctx.responses[3].data_as<adam::module::basic_info>();
        mod_info2->setup(adam::module::basic_info::unavailable, "mock_unavail", "/mock/path/unavail.so", adam::make_version(2, 0, 0));
        
        // connections
        ctx.responses[3].set_extended(true);
        ctx.responses.emplace_back();
        auto* conn_info = ctx.responses[4].data_as<adam::connection::basic_info>();
        conn_info->setup("mock_connection"_ct);
        conn_info->input_count = 0;
        conn_info->processor_count = 0;
        conn_info->output_count = 0;
        conn_info->started = false;
        conn_info->valid_chain = false;
    });

    ASSERT_TRUE(cmdr.connect()); // Implicitly requests initial data and populates caches

    EXPECT_EQ(cmdr.get_modules().get_paths().size(), 1u);
    EXPECT_EQ(cmdr.get_modules().get_paths()[0], "/mock/registry/path"_ct);

    EXPECT_EQ(cmdr.get_modules().get_available().size(), 1u);
    EXPECT_TRUE(cmdr.get_modules().get_available().contains("mock_avail"_ct.get_hash()));
    EXPECT_EQ(cmdr.get_modules().get_available().at("mock_avail"_ct).first, adam::make_version(1, 0, 0));
    EXPECT_EQ(cmdr.get_modules().get_available().at("mock_avail"_ct).second, "/mock/path/avail.so"_ct);

    EXPECT_EQ(cmdr.get_modules().get_unavailable().size(), 1u);
    EXPECT_TRUE(cmdr.get_modules().get_unavailable().contains("mock_unavail"_ct.get_hash()));

    EXPECT_EQ(cmdr.get_registry().get_connections().size(), 1u);
    EXPECT_TRUE(cmdr.get_registry().get_connections().contains(("mock_connection"_ct).get_hash()));

    // Restore default handler to not break other tests on the shared controller singleton
    ctrl.dispatcher().register_default_handlers();
    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests event synchronization of new modules getting added in the commander caches. */
TEST_F(commander_test, module_events_sync)
{
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    // Initially caches are empty (assuming no modules loaded in the test environment)
    size_t initial_available = cmdr.get_modules().get_available().size();
    size_t initial_unavailable = cmdr.get_modules().get_unavailable().size();

    // 1. Broadcast module_available event
    adam::event evt_avail(adam::event_type::module_available);
    auto* mod_info1 = evt_avail.data_as<adam::module::basic_info>();
    mod_info1->setup(adam::module::basic_info::available, "evt_mock_avail", "/mock/path/evt_avail.so", adam::make_version(1, 0, 0));

    adam::controller::get().broadcast_event(evt_avail);

    // 2. Broadcast module_unavailable event
    adam::event evt_unavail(adam::event_type::module_unavailable);
    auto* mod_info2 = evt_unavail.data_as<adam::module::basic_info>();
    mod_info2->setup(adam::module::basic_info::unavailable, "evt_mock_unavail", "/mock/path/evt_unavail.so", adam::make_version(1, 0, 0));

    adam::controller::get().broadcast_event(evt_unavail);

    // Give events time to propagate over the IPC queue and be dispatched by the thread
    auto start = std::chrono::steady_clock::now();
    while (cmdr.get_modules().get_unavailable().size() == initial_unavailable && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(cmdr.get_modules().get_available().size(), initial_available + 1);
    EXPECT_TRUE(cmdr.get_modules().get_available().contains("evt_mock_avail"_ct.get_hash()));
    
    EXPECT_EQ(cmdr.get_modules().get_unavailable().size(), initial_unavailable + 1);
    EXPECT_TRUE(cmdr.get_modules().get_unavailable().contains("evt_mock_unavail"_ct.get_hash()));

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the full flow of requesting a language change and receiving the updated state via event. */
TEST_F(commander_test, request_language_change_flow)
{
    adam::controller::get().set_language(adam::language_english); // Ensure we start from a known language state
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    // Default language should be english after connect (initial data received)
    EXPECT_EQ(cmdr.get_language(), adam::language_english);

    // Request language change
    adam::response_status status = cmdr.request_language_change(adam::language_german);
    EXPECT_EQ(status, adam::response_status::success);

    // Wait for the event to propagate and update the commander's language state
    auto start = std::chrono::steady_clock::now();
    while (cmdr.get_language() != adam::language_german && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Verify the event updated the internal state successfully
    EXPECT_EQ(cmdr.get_language(), adam::language_german);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the full flow of adding and removing a module path via commander and verifying the event synchronization. */
TEST_F(commander_test, request_module_path_add_remove_flow)
{
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    size_t initial_path_count = cmdr.get_modules().get_paths().size();
    adam::string_hashed_ct test_path = "/my/test/module/path";

    // 1. Request to add a module path
    adam::response_status status = cmdr.request_module_path_add(test_path);
    EXPECT_EQ(status, adam::response_status::success);

    // Wait for the event to propagate and update the commander's paths list
    auto start = std::chrono::steady_clock::now();
    while (cmdr.get_modules().get_paths().size() == initial_path_count && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Verify the path was added
    EXPECT_EQ(cmdr.get_modules().get_paths().size(), initial_path_count + 1);
    EXPECT_TRUE(std::find(cmdr.get_modules().get_paths().begin(), cmdr.get_modules().get_paths().end(), test_path) != cmdr.get_modules().get_paths().end());

    // 2. Request to remove the module path
    status = cmdr.request_module_path_remove(static_cast<uint32_t>(initial_path_count));
    EXPECT_EQ(status, adam::response_status::success);

    // Wait for the event to propagate
    start = std::chrono::steady_clock::now();
    while (cmdr.get_modules().get_paths().size() > initial_path_count && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Verify the path was removed
    EXPECT_EQ(cmdr.get_modules().get_paths().size(), initial_path_count);
    EXPECT_TRUE(std::find(cmdr.get_modules().get_paths().begin(), cmdr.get_modules().get_paths().end(), test_path) == cmdr.get_modules().get_paths().end());

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the full flow of requesting a module scan. */
TEST_F(commander_test, request_module_scan_flow)
{
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    adam::response_status status = cmdr.request_module_scan();
    EXPECT_EQ(status, adam::response_status::success);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the behavior when a shutdown event is received from the controller. */
TEST_F(commander_test, shutdown_event_flow)
{
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());
    EXPECT_TRUE(cmdr.is_active());

    // Broadcast the shutdown event
    adam::event evt(adam::event_type::shutdown);
    adam::controller::get().broadcast_event(evt);

    // Wait for the event to propagate and the commander to destroy itself
    auto start = std::chrono::steady_clock::now();
    while (cmdr.is_active() && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Verify the commander is no longer active
    EXPECT_FALSE(cmdr.is_active());
    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests synchronization of an unavailable port changing its state after module load. */
TEST_F(commander_test, sync_unavailable_port)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Setup an unavailable port in the controller's registry
    auto upi = std::make_unique<adam::port::unavailable_info>("cmd_unavail_port"_ct);
    upi->type = ("some_type"_ct).get_hash();
    upi->type_module = ("missing_mod"_ct).get_hash();
    ctrl.get_registry().unavailable_ports()[("cmd_unavail_port"_ct).get_hash()] = std::move(upi);
    
    adam::connection* conn = nullptr;
    ctrl.get_registry().create_connection("cmd_conn"_ct, &conn);
    conn->unavailable_inputs().push_back("cmd_unavail_port"_ct);

    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    // Verify it's in the registry view as unavailable
    const auto& ports = cmdr.get_registry().get_ports();
    EXPECT_EQ(ports.size(), 1u);
    
    auto port_hash = ("cmd_unavail_port"_ct).get_hash();
    EXPECT_TRUE(ports.contains(port_hash));
    EXPECT_TRUE(ports.at(port_hash)->is_unavailable);
    
    const auto& conns = cmdr.get_registry().get_connections();
    EXPECT_EQ(conns.size(), 1u);
    EXPECT_EQ(conns.at(("cmd_conn"_ct).get_hash())->inputs.size(), 1u);
    EXPECT_EQ(conns.at(("cmd_conn"_ct).get_hash())->inputs[0], port_hash);

    // Trigger retry by broadcasting a port_available event
    adam::event evt(adam::event_type::port_available);
    auto* evt_data = evt.data_as<adam::port::basic_info>();
    evt_data->setup("cmd_unavail_port"_ct, ("some_type"_ct).get_hash(), ("missing_mod"_ct).get_hash(), false);
    evt_data->dir = adam::port::direction_inout;
    evt_data->started = false;
    ctrl.broadcast_event(evt);

    // Wait for event to clear the is_unavailable flag
    auto start = std::chrono::steady_clock::now();
    while (cmdr.get_registry().get_ports().at(port_hash)->is_unavailable && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_FALSE(ports.at(port_hash)->is_unavailable);
    EXPECT_EQ(ports.at(port_hash)->direction, adam::port::direction_inout);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests synchronization of an unavailable processor changing its state after module load. */
TEST_F(commander_test, sync_unavailable_processor)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Setup an unavailable processor in the controller's registry
    auto upi = std::make_unique<adam::processor::unavailable_info>("cmd_unavail_proc"_ct);
    upi->type = ("some_type"_ct).get_hash();
    upi->type_module = ("missing_mod"_ct).get_hash();
    upi->is_filter = false;
    ctrl.get_registry().unavailable_processors()[("cmd_unavail_proc"_ct).get_hash()] = std::move(upi);
    
    adam::connection* conn = nullptr;
    ctrl.get_registry().create_connection("cmd_conn"_ct, &conn);
    conn->unavailable_processors().push_back("cmd_unavail_proc"_ct);

    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    // Verify it's in the registry view as unavailable
    const auto& processors = cmdr.get_registry().get_processors();
    EXPECT_EQ(processors.size(), 1u);
    
    auto proc_hash = ("cmd_unavail_proc"_ct).get_hash();
    EXPECT_TRUE(processors.contains(proc_hash));
    EXPECT_TRUE(processors.at(proc_hash)->is_unavailable);
    
    const auto& conns = cmdr.get_registry().get_connections();
    EXPECT_EQ(conns.size(), 1u);
    EXPECT_EQ(conns.at(("cmd_conn"_ct).get_hash())->processors.size(), 1u);
    EXPECT_EQ(conns.at(("cmd_conn"_ct).get_hash())->processors[0], proc_hash);

    // Trigger retry by broadcasting a processor_available event
    adam::event evt(adam::event_type::processor_available);
    auto* evt_data = evt.data_as<adam::processor::basic_info>();
    evt_data->setup("cmd_unavail_proc"_ct, ("some_type"_ct).get_hash(), ("missing_mod"_ct).get_hash(), false);
    ctrl.broadcast_event(evt);

    // Wait for event to clear the is_unavailable flag
    auto start = std::chrono::steady_clock::now();
    while (cmdr.get_registry().get_processors().at(proc_hash)->is_unavailable && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_FALSE(processors.at(proc_hash)->is_unavailable);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the full flow of creating and destroying a connection via commander and verifying the event synchronization. */
TEST_F(commander_test, connection_create)
{
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    auto test_connection = "test_connection"_ct;
    size_t initial_conn_count = cmdr.get_registry().get_connections().size();

    // 1. Request to create a connection
    adam::response_status status = cmdr.request_connection_create(test_connection);
    EXPECT_EQ(status, adam::response_status::success);

    // Wait for the event to propagate and update the commander's connections list
    auto start = std::chrono::steady_clock::now();
    while (cmdr.get_registry().get_connections().size() == initial_conn_count && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Verify the connection was created
    EXPECT_EQ(cmdr.get_registry().get_connections().size(), initial_conn_count + 1);
    EXPECT_TRUE(cmdr.get_registry().get_connections().contains(test_connection.get_hash()));

    auto conn_hash = test_connection.get_hash();
    const auto& conn_view = cmdr.get_registry().get_connections().at(conn_hash);
    EXPECT_EQ(conn_view->name.get_hash(), conn_hash);
    EXPECT_NE(conn_view->created, 0ull);

    // 2. Request to destroy the connection
    status = cmdr.request_connection_destroy(conn_hash);
    EXPECT_EQ(status, adam::response_status::success);

    // Wait for the event to propagate
    start = std::chrono::steady_clock::now();
    while (cmdr.get_registry().get_connections().size() > initial_conn_count && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Verify the connection was destroyed
    EXPECT_EQ(cmdr.get_registry().get_connections().size(), initial_conn_count);
    EXPECT_FALSE(cmdr.get_registry().get_connections().contains(conn_hash));

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests synchronization of an unavailable connection changing its state after module load. */
TEST_F(commander_test, sync_unavailable_connection)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Setup an available connection in the controller's registry
    adam::connection* conn = nullptr;
    ctrl.get_registry().create_connection("cmd_unavail_conn"_ct, &conn);

    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    auto conn_hash = ("cmd_unavail_conn"_ct).get_hash();
    const auto& conns = cmdr.get_registry().get_connections();
    
    EXPECT_TRUE(conns.contains(conn_hash));
    EXPECT_FALSE(conns.at(conn_hash)->is_unavailable);

    // Broadcast unavailable event
    adam::event evt_unavail(adam::event_type::connection_unavailable);
    evt_unavail.data_as<adam::messages::connection_action_data>()->connection = conn_hash;
    ctrl.broadcast_event(evt_unavail);

    auto start = std::chrono::steady_clock::now();
    while (!conns.at(conn_hash)->is_unavailable && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_TRUE(conns.at(conn_hash)->is_unavailable);

    // Trigger retry by broadcasting a connection_available event
    adam::event evt(adam::event_type::connection_available);
    auto* evt_data = evt.data_as<adam::connection::basic_info>();
    evt_data->setup("cmd_unavail_conn"_ct);
    evt_data->input_format = ("some_missing_fmt"_ct).get_hash();
    evt_data->input_format_module = ("missing_mod"_ct).get_hash();
    evt_data->is_unavailable = false;
    evt_data->valid_chain = true;
    ctrl.broadcast_event(evt);

    start = std::chrono::steady_clock::now();
    while (conns.at(conn_hash)->is_unavailable && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_FALSE(conns.at(conn_hash)->is_unavailable);
    EXPECT_TRUE(conns.at(conn_hash)->valid_chain);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests synchronization of connection data format changes including valid_chain state. */
TEST_F(commander_test, connection_data_format_changed_sync)
{
    adam::controller& ctrl = adam::controller::get();
    adam::connection* conn = nullptr;
    ctrl.get_registry().create_connection("fmt_conn"_ct, &conn);
    
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    auto conn_hash = ("fmt_conn"_ct).get_hash();
    EXPECT_TRUE(cmdr.get_registry().get_connections().contains(conn_hash));

    cmdr.modules().lock();
    adam::module_info minfo;
    minfo.name = "new_mod"_ct;
    minfo.data_formats.push_back("new_fmt"_ct);
    cmdr.modules().database()["new_mod"_ct] = minfo;
    cmdr.modules().unlock();

    // Trigger input format change event
    adam::event evt(adam::event_type::connection_input_data_format_changed);
    auto* evt_data = evt.data_as<adam::messages::connection_data_format_data>();
    evt_data->connection = conn_hash;
    evt_data->format = ("new_fmt"_ct).get_hash();
    evt_data->format_module = ("new_mod"_ct).get_hash();
    evt_data->valid_chain = true;
    ctrl.broadcast_event(evt);

    auto start = std::chrono::steady_clock::now();
    while (cmdr.get_registry().get_connections().at(conn_hash)->input_format.get_hash() != ("new_fmt"_ct).get_hash() && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    const auto& conn_view = cmdr.get_registry().get_connections().at(conn_hash);
    EXPECT_EQ(conn_view->input_format.get_hash(), ("new_fmt"_ct).get_hash());
    EXPECT_EQ(conn_view->input_format_module.get_hash(), ("new_mod"_ct).get_hash());
    EXPECT_TRUE(conn_view->valid_chain);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the full flow of adding and removing a port to a connection via commander and verifying event synchronization. */
TEST_F(commander_test, request_connection_port_add_remove_flow)
{
    adam::controller& ctrl = adam::controller::get();
    
    // Create a port in the controller's registry
    adam::port* test_port = nullptr;
    EXPECT_EQ(ctrl.get_registry().create_port("my_test_port"_ct, adam::port_internal::type_name(), 0, &test_port), adam::registry::status_success);
    ASSERT_NE(test_port, nullptr);
    
    // Create a connection in the controller's registry
    adam::connection* conn = nullptr;
    EXPECT_EQ(ctrl.get_registry().create_connection("my_test_conn"_ct, &conn), adam::registry::status_success);
    ASSERT_NE(conn, nullptr);

    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    auto conn_hash = ("my_test_conn"_ct).get_hash();
    auto port_hash = ("my_test_port"_ct).get_hash();

    // Verify it is initially empty on both sides
    const auto& conn_view = cmdr.get_registry().get_connections().at(conn_hash);
    EXPECT_TRUE(conn_view->inputs.empty());
    EXPECT_TRUE(conn_view->outputs.empty());

    // 1. Request to add as input
    EXPECT_EQ(cmdr.request_connection_port_add(conn_hash, port_hash, true), adam::response_status::success);

    // Wait for the event to propagate and update the commander's connection view
    auto start = std::chrono::steady_clock::now();
    while (conn_view->inputs.empty() && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(conn_view->inputs.size(), 1u);
    EXPECT_EQ(conn_view->inputs[0], port_hash);
    EXPECT_TRUE(conn_view->outputs.empty());

    // 2. Request to remove from input
    EXPECT_EQ(cmdr.request_connection_port_remove(conn_hash, port_hash, true), adam::response_status::success);

    // Wait for the event to propagate
    start = std::chrono::steady_clock::now();
    while (!conn_view->inputs.empty() && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_TRUE(conn_view->inputs.empty());

    // Recreate the port (since it gets auto-destroyed when removed from its last connection)
    EXPECT_EQ(ctrl.get_registry().create_port("my_test_port"_ct, adam::port_internal::type_name(), 0, &test_port), adam::registry::status_success);
    ASSERT_NE(test_port, nullptr);

    // 3. Request to add as output
    EXPECT_EQ(cmdr.request_connection_port_add(conn_hash, port_hash, false), adam::response_status::success);

    // Wait for the event to propagate
    start = std::chrono::steady_clock::now();
    while (conn_view->outputs.empty() && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(conn_view->outputs.size(), 1u);
    EXPECT_EQ(conn_view->outputs[0], port_hash);
    EXPECT_TRUE(conn_view->inputs.empty());

    // 4. Request to remove from output
    EXPECT_EQ(cmdr.request_connection_port_remove(conn_hash, port_hash, false), adam::response_status::success);

    // Wait for the event to propagate
    start = std::chrono::steady_clock::now();
    while (!conn_view->outputs.empty() && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_TRUE(conn_view->outputs.empty());

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the full flow of creating, renaming, and destroying a processor via commander. */
TEST_F(commander_test, request_processor_lifecycle_flow)
{
    adam::controller& ctrl = adam::controller::get();

    // Setup mock module with mock processor factory
    cmdr_test::mock_module mock_mod("mock_mod"_ct);
    mock_mod.register_processor_factory("mock-converter"_ct, &cmdr_test::global_mock_proc_factory);
    cmdr_test::mock_module_injector injector(ctrl.get_registry(), &mock_mod);

    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    // 1. Request processor creation
    adam::response_status status = cmdr.request_processor_create("test_proc"_ct, "mock-converter"_ct, "mock_mod"_ct, false);
    EXPECT_EQ(status, adam::response_status::success);

    auto proc_hash = ("test_proc"_ct).get_hash();

    // Wait for the event to propagate and update commander's registry view
    auto start = std::chrono::steady_clock::now();
    while (!cmdr.get_registry().get_processors().contains(proc_hash) && 
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ASSERT_TRUE(cmdr.get_registry().get_processors().contains(proc_hash));
    EXPECT_EQ(cmdr.get_registry().get_processors().at(proc_hash)->name.get_hash(), proc_hash);

    // 2. Request processor rename
    status = cmdr.request_processor_rename(proc_hash, "renamed_proc"_ct);
    EXPECT_EQ(status, adam::response_status::success);

    auto new_proc_hash = ("renamed_proc"_ct).get_hash();

    // Wait for rename event
    start = std::chrono::steady_clock::now();
    while (!cmdr.get_registry().get_processors().contains(new_proc_hash) && 
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_TRUE(cmdr.get_registry().get_processors().contains(new_proc_hash));
    EXPECT_FALSE(cmdr.get_registry().get_processors().contains(proc_hash));

    // 3. Request processor destroy
    status = cmdr.request_processor_destroy(new_proc_hash);
    EXPECT_EQ(status, adam::response_status::success);

    // Wait for destroy event
    start = std::chrono::steady_clock::now();
    while (cmdr.get_registry().get_processors().contains(new_proc_hash) && 
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_FALSE(cmdr.get_registry().get_processors().contains(new_proc_hash));

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests attaching, reordering, and removing processors on a connection via commander. */
TEST_F(commander_test, request_connection_processor_flow)
{
    adam::controller& ctrl = adam::controller::get();

    // Setup mock module with mock processor factory
    cmdr_test::mock_module mock_mod("mock_mod"_ct);
    mock_mod.register_processor_factory("mock-converter"_ct, &cmdr_test::global_mock_proc_factory);
    cmdr_test::mock_module_injector injector(ctrl.get_registry(), &mock_mod);

    // Create processors and connection in controller registry
    adam::processor* proc1 = nullptr;
    adam::processor* proc2 = nullptr;
    adam::connection* conn = nullptr;
    ASSERT_EQ(ctrl.get_registry().create_processor("p1"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &proc1), adam::registry::status_success);
    ASSERT_EQ(ctrl.get_registry().create_processor("p2"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &proc2), adam::registry::status_success);
    ASSERT_EQ(ctrl.get_registry().create_connection("test_conn"_ct, &conn), adam::registry::status_success);

    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    auto conn_hash = ("test_conn"_ct).get_hash();
    auto p1_hash = ("p1"_ct).get_hash();
    auto p2_hash = ("p2"_ct).get_hash();

    const auto& conn_view = cmdr.get_registry().get_connections().at(conn_hash);
    EXPECT_TRUE(conn_view->processors.empty());

    // 1. Add p1 to connection
    EXPECT_EQ(cmdr.request_connection_processor_add(conn_hash, p1_hash), adam::response_status::success);
    auto start = std::chrono::steady_clock::now();
    while (conn_view->processors.size() < 1 && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    ASSERT_EQ(conn_view->processors.size(), 1u);
    EXPECT_EQ(conn_view->processors[0], p1_hash);

    // 2. Add p2 to connection
    EXPECT_EQ(cmdr.request_connection_processor_add(conn_hash, p2_hash), adam::response_status::success);
    start = std::chrono::steady_clock::now();
    while (conn_view->processors.size() < 2 && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    ASSERT_EQ(conn_view->processors.size(), 2u);
    EXPECT_EQ(conn_view->processors[0], p1_hash);
    EXPECT_EQ(conn_view->processors[1], p2_hash);

    // 3. Reorder processors (move p1 to index 1, making order [p2, p1])
    EXPECT_EQ(cmdr.request_connection_processor_reorder(conn_hash, p1_hash, 1), adam::response_status::success);
    start = std::chrono::steady_clock::now();
    while (conn_view->processors[0] != p2_hash && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(conn_view->processors[0], p2_hash);
    EXPECT_EQ(conn_view->processors[1], p1_hash);

    // 4. Remove p2 from connection
    EXPECT_EQ(cmdr.request_connection_processor_remove(conn_hash, p2_hash), adam::response_status::success);
    start = std::chrono::steady_clock::now();
    while (conn_view->processors.size() > 1 && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_EQ(conn_view->processors.size(), 1u);
    EXPECT_EQ(conn_view->processors[0], p1_hash);

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests the lifecycle of creating, starting, stopping, renaming, and destroying a port via commander. */
TEST_F(commander_test, request_port_lifecycle_flow)
{
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    // 1. Request port creation
    adam::response_status status = cmdr.request_port_create("test_port"_ct, adam::port_internal::type_name(), 0);
    EXPECT_EQ(status, adam::response_status::success);

    auto port_hash = ("test_port"_ct).get_hash();

    // Wait for creation event
    auto start = std::chrono::steady_clock::now();
    while (!cmdr.get_registry().get_ports().contains(port_hash) && 
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ASSERT_TRUE(cmdr.get_registry().get_ports().contains(port_hash));
    const auto& port_view = cmdr.get_registry().get_ports().at(port_hash);
    EXPECT_FALSE(port_view->started);

    // 2. Request port start
    status = cmdr.request_port_start(port_hash);
    EXPECT_EQ(status, adam::response_status::success);

    // Wait for start event
    start = std::chrono::steady_clock::now();
    while (!port_view->started && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_TRUE(port_view->started);

    // 3. Request port stop
    status = cmdr.request_port_stop(port_hash);
    EXPECT_EQ(status, adam::response_status::success);

    // Wait for stop event
    start = std::chrono::steady_clock::now();
    while (port_view->started && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_FALSE(port_view->started);

    // 4. Request port rename
    status = cmdr.request_port_rename(port_hash, "renamed_port"_ct);
    EXPECT_EQ(status, adam::response_status::success);

    auto new_port_hash = ("renamed_port"_ct).get_hash();

    // Wait for rename event
    start = std::chrono::steady_clock::now();
    while (!cmdr.get_registry().get_ports().contains(new_port_hash) && 
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_TRUE(cmdr.get_registry().get_ports().contains(new_port_hash));
    EXPECT_FALSE(cmdr.get_registry().get_ports().contains(port_hash));

    // 5. Request port destroy
    status = cmdr.request_port_destroy(new_port_hash);
    EXPECT_EQ(status, adam::response_status::success);

    // Wait for destroy event
    start = std::chrono::steady_clock::now();
    while (cmdr.get_registry().get_ports().contains(new_port_hash) && 
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_FALSE(cmdr.get_registry().get_ports().contains(new_port_hash));

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests connection property modifications (rename, sorting index, color, format) via commander. */
TEST_F(commander_test, request_connection_properties_flow)
{
    adam::controller& ctrl = adam::controller::get();
    adam::connection* conn = nullptr;
    ASSERT_EQ(ctrl.get_registry().create_connection("prop_conn"_ct, &conn), adam::registry::status_success);

    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());

    auto conn_hash = ("prop_conn"_ct).get_hash();

    // 1. Request connection rename
    EXPECT_EQ(cmdr.request_connection_rename(conn_hash, "renamed_conn"_ct), adam::response_status::success);

    auto new_conn_hash = ("renamed_conn"_ct).get_hash();

    auto start = std::chrono::steady_clock::now();
    while (!cmdr.get_registry().get_connections().contains(new_conn_hash) && 
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ASSERT_TRUE(cmdr.get_registry().get_connections().contains(new_conn_hash));
    const auto& new_conn_view = cmdr.get_registry().get_connections().at(new_conn_hash);

    // 2. Sorting index change
    EXPECT_EQ(cmdr.request_connection_sorting_index_change(new_conn_hash, 13), adam::response_status::success);
    start = std::chrono::steady_clock::now();
    while (new_conn_view->sorting_index != 13 && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(new_conn_view->sorting_index, 13u);

    // 3. Color change
    EXPECT_EQ(cmdr.request_connection_color_change(new_conn_hash, 0x00FF00), adam::response_status::success);
    start = std::chrono::steady_clock::now();
    while (new_conn_view->color != 0x00FF00 && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(new_conn_view->color, 0x00FF00u);

    // 4. Data formats
    EXPECT_EQ(cmdr.request_connection_set_input_data_format(new_conn_hash, "transparent"_ct.get_hash(), 0), adam::response_status::success);
    EXPECT_EQ(cmdr.request_connection_set_output_data_format(new_conn_hash, "transparent"_ct.get_hash(), 0), adam::response_status::success);
    
    start = std::chrono::steady_clock::now();
    while ((new_conn_view->input_format.get_hash() != "transparent"_ct.get_hash() || 
            new_conn_view->output_format.get_hash() != "transparent"_ct.get_hash()) && 
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_EQ(new_conn_view->input_format.get_hash(), "transparent"_ct.get_hash());
    EXPECT_EQ(new_conn_view->output_format.get_hash(), "transparent"_ct.get_hash());

    EXPECT_TRUE(cmdr.destroy());
}

/** @brief Tests that disable() shuts down connection queues. */
TEST_F(commander_test, commander_disable_flow)
{
    adam::commander cmdr;
    ASSERT_TRUE(cmdr.connect());
    EXPECT_TRUE(cmdr.is_active());

    EXPECT_TRUE(cmdr.disable());
    EXPECT_FALSE(cmdr.is_active());

    EXPECT_TRUE(cmdr.destroy());
}
