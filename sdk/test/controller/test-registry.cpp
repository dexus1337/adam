#include <gtest/gtest.h>
#include "controller/registry.hpp"
#include "controller/controller.hpp"
#include "configuration/parameters/configuration-parameter-string.hpp"
#include "configuration/parameters/configuration-parameter-integer.hpp"
#include "configuration/parameters/configuration-parameter-list.hpp"
#include "configuration/parameters/configuration-parameter-list-sorted.hpp"
#include "configuration/parameters/configuration-parameter-reference.hpp"
#include "data/port/port-input.hpp"
#include "data/port/port-output.hpp"
#include "data/port/port-internal.hpp"
#include "data/processors/filter.hpp"
#include "data/processors/converter.hpp"
#include "module/module.hpp"
#include "data/connection.hpp"
#include "data/format.hpp"
#include "memory/buffer/buffer-manager.hpp"

#include <filesystem>
#include <fstream>

using namespace adam::string_hashed_ct_literals;

namespace adam::test
{
    /** @brief A local controller instance to isolate tests from the global singleton. */
    class local_controller : public adam::controller
    {
    public:
        local_controller() : adam::controller() {}
    };

    struct loaded_modules_tag 
    {
        using type = adam::registry_module_manager::map_loaded_modules adam::registry_module_manager::*;
        friend type get_private(loaded_modules_tag);
    };

    template <typename Tag, typename Tag::type Member>
    struct private_accessor 
    {
        friend typename Tag::type get_private(Tag) 
        {
            return Member;
        }
    };
}

template struct adam::test::private_accessor<adam::test::loaded_modules_tag, &adam::registry_module_manager::m_loaded_modules>;

namespace adam::test
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

        bool handle_data(adam::buffer*& buffer) override
        {
            return true;
        }

        void set_input_format(const adam::data_format* fmt) { m_format_input = fmt; }
        void set_output_format(const adam::data_format* fmt) { m_format_output = fmt; }
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

        void register_data_format(const adam::string_hashed& name, const adam::data_format* format)
        {
            m_data_formats[name] = format;
        }

        void register_processor_factory(const adam::string_hashed& type_name, adam::factory<adam::processor>* factory_ptr,
                                        adam::string_hash input_format = 0, adam::string_hash input_format_module = 0,
                                        adam::string_hash output_format = 0, adam::string_hash output_format_module = 0)
        {
            m_processor_factories[type_name] = adam::registry::factory_data_processor(factory_ptr, nullptr, input_format, input_format_module, output_format, output_format_module);
        }
    };

    class mock_module_injector
    {
    public:
        mock_module_injector(adam::registry& reg, adam::module* mod)
            : m_reg(reg), m_name(mod->get_name())
        {
            auto& loaded_modules = reg.modules().*get_private(loaded_modules_tag{});
            loaded_modules.emplace(m_name, mod);
        }

        ~mock_module_injector()
        {
            auto& loaded_modules = m_reg.modules().*get_private(loaded_modules_tag{});
            loaded_modules.erase(m_name);
        }

    private:
        adam::registry& m_reg;
        adam::string_hashed m_name;
    };

    static mock_processor_factory global_mock_proc_factory;
}

class registry_test : public ::testing::Test
{
protected:
    const std::string test_filepath = "test_adam_registry.bin";
    const std::string adam_config_filepath = "adam-config.bin";

    void SetUp() override
    {
        adam::buffer_manager::get().initialize();
        if (std::filesystem::exists(test_filepath))
            std::filesystem::remove(test_filepath);
        if (std::filesystem::exists(adam_config_filepath))
            std::filesystem::remove(adam_config_filepath);
    }

    void TearDown() override
    {
        if (std::filesystem::exists(test_filepath))
            std::filesystem::remove(test_filepath);
        if (std::filesystem::exists(adam_config_filepath))
            std::filesystem::remove(adam_config_filepath);
        adam::buffer_manager::get().destroy();
    }
};

/** @brief Tests saving and loading a completely empty registry. */
TEST_F(registry_test, empty_save_load)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    EXPECT_TRUE(reg.save(test_filepath));
    EXPECT_TRUE(std::filesystem::exists(test_filepath));
    
    EXPECT_TRUE(reg.load(test_filepath));
}

/** @brief Tests fully populating the registry, modifying values, and verifying original values are restored upon reload. */
TEST_F(registry_test, save_modify_reload_verify)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    // 1. Modify the existing default 'language' parameter
    auto* lang_param = static_cast<adam::configuration_parameter_integer*>(reg.get_parameters().get("language"_ct));
    ASSERT_NE(lang_param, nullptr);
    lang_param->set_value(adam::language_german);
    
    // 2. Create a port
    auto port_name = "my_input_port"_ct;
    adam::port* created_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_internal::type_name(), 0, &created_port), adam::registry::status_success);
    ASSERT_NE(created_port, nullptr);

    // Change a parameter in the port to verify it restores correctly
    auto* active_param = static_cast<adam::configuration_parameter_boolean*>(created_port->get_parameters().get("started"_ct));
    ASSERT_NE(active_param, nullptr);
    active_param->set_value(true);

    // Save the populated registry
    EXPECT_TRUE(reg.save(test_filepath));
    EXPECT_TRUE(std::filesystem::exists(test_filepath));
    
    // 3. Modify existing parameters to ensure we load fresh values from file
    lang_param->set_value(adam::language_english);
    active_param->set_value(false);

    // 4. Reload from the binary file into a fresh registry to verify persistence
    adam::test::local_controller loaded_ctrl;
    adam::registry& loaded_reg = loaded_ctrl.get_registry();
    EXPECT_TRUE(loaded_reg.load(test_filepath));
    
    // 5. Verify the values are restored correctly
    auto* loaded_lang_param = loaded_reg.get_parameters().get("language"_ct);
    ASSERT_NE(loaded_lang_param, nullptr);
    EXPECT_EQ(loaded_lang_param->get_type(), adam::configuration_parameter::type_integer);
    EXPECT_EQ(static_cast<adam::configuration_parameter_integer*>(loaded_lang_param)->get_value(), adam::language_german);

    // Verify port was recreated and parameters are restored
    EXPECT_EQ(loaded_reg.ports().size(), 1u);
    EXPECT_TRUE(loaded_reg.ports().contains(port_name));
    
    adam::port* loaded_port = loaded_reg.ports().at(port_name).get();
    ASSERT_NE(loaded_port, nullptr);
    EXPECT_EQ(loaded_port->get_type_name(), adam::port_internal::type_name());
    
    auto* loaded_active_param = static_cast<adam::configuration_parameter_boolean*>(loaded_port->get_parameters().get("started"_ct));
    ASSERT_NE(loaded_active_param, nullptr);
    EXPECT_EQ(loaded_active_param->get_value(), true);
}

/** @brief Tests that file I/O operations fail gracefully with bad paths or invalid file formats. */
TEST_F(registry_test, invalid_file_io)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    // Saving to an invalid path should fail gracefully
    EXPECT_FALSE(reg.save("/invalid_path_1337/registry.bin"));
    
    // Loading a non-existent file should fail gracefully
    EXPECT_FALSE(reg.load("non_existent_registry.bin"));
    
    // Create a corrupted/invalid binary file
    {
        std::ofstream ofs(test_filepath, std::ios::binary);
        ofs << "This is definitely not a valid ADAM configuration format :)";
        ofs.close();
    }
    
    // Loading an invalid binary file should identify the bad magic number/version and fail
    EXPECT_FALSE(reg.load(test_filepath));
}

/** @brief Tests creating and removing ports via the registry. */
TEST_F(registry_test, create_and_remove_port)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();

    auto port_name = "test_port_1"_ct;

    // Attempt to create a port with an invalid type should gracefully fail
    adam::port* invalid_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, "non_existent_type"_ct, 0, &invalid_port), adam::registry::status_error_factory_not_found);
    EXPECT_EQ(invalid_port, nullptr);

    // Create the port using the internal factory
    adam::port* created_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_internal::type_name(), 0, &created_port), adam::registry::status_success);
    ASSERT_NE(created_port, nullptr);
    EXPECT_EQ(reg.ports().size(), 1u);
    EXPECT_TRUE(reg.ports().contains(port_name));

    // Attempt to create a duplicate port should gracefully fail
    adam::port* duplicate_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_internal::type_name(), 0, &duplicate_port), adam::registry::status_error_port_already_exists);
    EXPECT_EQ(duplicate_port, nullptr);
    EXPECT_EQ(reg.ports().size(), 1u);

    // Remove the port
    EXPECT_EQ(reg.destroy_port(port_name), adam::registry::status_success);
    EXPECT_EQ(reg.ports().size(), 0u);
    EXPECT_FALSE(reg.ports().contains(port_name));
    
    // Attempt to remove a non-existent port should gracefully fail
    EXPECT_EQ(reg.destroy_port(port_name), adam::registry::status_error_port_not_found);
}

/** @brief Tests clearing the registry and verifying state reset. */
TEST_F(registry_test, clear_registry)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    auto port_name = "test_port_clear"_ct;
    
    EXPECT_EQ(reg.create_port(port_name, adam::port_internal::type_name(), 0), adam::registry::status_success);
    EXPECT_EQ(reg.ports().size(), 1u);
    
    reg.clear();
    
    EXPECT_TRUE(reg.ports().empty());
    EXPECT_TRUE(reg.processors().empty());
    EXPECT_TRUE(reg.connections().empty());
}

/** @brief Tests that port properties like type and module_name are persisted. */
TEST_F(registry_test, port_type_and_module_persistence)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    auto port_name = "type_test_port"_ct;

    adam::port* created_port = nullptr;
    EXPECT_EQ(reg.create_port(port_name, adam::port_internal::type_name(), 0, &created_port), adam::registry::status_success);
    ASSERT_NE(created_port, nullptr);

    // Verify type was populated correctly in the constructor
    auto* type_param = static_cast<adam::configuration_parameter_string*>(created_port->get_parameters().get("type"_ct));
    ASSERT_NE(type_param, nullptr);
    EXPECT_EQ(type_param->get_value(), adam::port_internal::type_name());

    EXPECT_TRUE(reg.save(test_filepath));

    adam::test::local_controller loaded_ctrl;
    adam::registry& loaded_reg = loaded_ctrl.get_registry();
    EXPECT_TRUE(loaded_reg.load(test_filepath));

    // Verify the port was correctly recreated from the saved type
    EXPECT_TRUE(loaded_reg.ports().contains(port_name));
    adam::port* loaded_port = loaded_reg.ports().at(port_name).get();
    ASSERT_NE(loaded_port, nullptr);
    
    auto* loaded_type_param = static_cast<adam::configuration_parameter_string*>(loaded_port->get_parameters().get("type"_ct));
    ASSERT_NE(loaded_type_param, nullptr);
    EXPECT_EQ(loaded_type_param->get_value(), adam::port_internal::type_name());
}

/** @brief Tests adding and removing module paths in the registry. */
TEST_F(registry_test, add_and_remove_module_paths)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    auto new_path = "/custom/registry/path"_ct;

    // Add the path
    uint32_t added_idx = 0;
    EXPECT_TRUE(reg.add_module_path(new_path, &added_idx));
    
    // Adding the same path again should fail
    EXPECT_FALSE(reg.add_module_path(new_path));

    // Verify it was added correctly
    auto* paths_list = reg.get_module_paths();
    ASSERT_NE(paths_list, nullptr);
    
    bool found = false;
    for (const auto& [name, param] : paths_list->get_children())
    {
        if (auto* str_param = dynamic_cast<adam::configuration_parameter_string*>(param.get()))
        {
            if (str_param->get_value() == new_path)
            {
                found = true;
                break;
            }
        }
    }
    EXPECT_TRUE(found);

    // Remove the path
    EXPECT_TRUE(reg.remove_module_path(added_idx));
    
    // Removing it again should fail
    EXPECT_FALSE(reg.remove_module_path(added_idx));

    // Verify it was removed (using the internal get method to prove it's gone from the config)
    for (const auto& [name, param] : paths_list->get_children())
    {
        if (auto* str_param = dynamic_cast<adam::configuration_parameter_string*>(param.get()))
        {
            EXPECT_NE(str_param->get_value(), new_path);
        }
    }
}

/** @brief Tests restoring and syncing of unavailable ports. */
TEST_F(registry_test, unavailable_port_retry)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    // 1. Manually add an unavailable port to the configuration
    auto upi = std::make_unique<adam::port::unavailable_info>("my_unavail_port"_ct);
    upi->type = "some_missing_type"_ct.get_hash();
    upi->type_module = "missing_module"_ct.get_hash();
    
    auto type_param = std::make_unique<adam::configuration_parameter_string>("type"_ct);
    type_param->set_value("some_missing_type"_ct);
    upi->parameters().add(std::move(type_param));
    
    auto mod_param = std::make_unique<adam::configuration_parameter_string>("type_origin_module"_ct);
    mod_param->set_value("missing_module"_ct);
    upi->parameters().add(std::move(mod_param));
    
    reg.unavailable_ports()["my_unavail_port"_ct.get_hash()] = std::move(upi);
    
    // 2. Add it to a connection
    adam::connection* conn = nullptr;
    EXPECT_EQ(reg.create_connection("my_conn"_ct, &conn), adam::registry::status_success);
    conn->unavailable_inputs().push_back("my_unavail_port"_ct);
    
    if (auto* inputs_list = dynamic_cast<adam::configuration_parameter_list*>(conn->get_parameters().get("inputs"_ct)))
    {
        auto param = std::make_unique<adam::configuration_parameter_reference>("0"_ct);
        param->set_target("my_unavail_port"_ct);
        inputs_list->add(std::move(param));
    }
    
    // 3. Save and reload to ensure it persists
    EXPECT_TRUE(reg.save(test_filepath));
    
    adam::test::local_controller loaded_ctrl;
    adam::registry& loaded_reg = loaded_ctrl.get_registry();
    EXPECT_TRUE(loaded_reg.load(test_filepath));
    
    EXPECT_TRUE(loaded_reg.get_unavailable_ports().contains("my_unavail_port"_ct.get_hash()));
    EXPECT_EQ(loaded_reg.connections().at("my_conn"_ct.get_hash())->unavailable_inputs().size(), 1u);
    EXPECT_EQ(loaded_reg.ports().size(), 0u);
    
    // 4. Now let's pretend the module "missing_module" is loaded.
    // We clear the unavailable ports and insert a valid internal port mock to test the retry mechanism natively
    loaded_reg.unavailable_ports().clear();
    auto upi3 = std::make_unique<adam::port::unavailable_info>("test_retry_port"_ct);
    upi3->type = adam::port_internal::type_name().get_hash();
    upi3->type_module = 0; 
    
    auto type_param3 = std::make_unique<adam::configuration_parameter_string>("type"_ct);
    type_param3->set_value(adam::port_internal::type_name());
    upi3->parameters().add(std::move(type_param3));
    
    loaded_reg.unavailable_ports()["test_retry_port"_ct.get_hash()] = std::move(upi3);
    
    loaded_reg.connections().at("my_conn"_ct.get_hash())->unavailable_inputs()[0] = "test_retry_port"_ct;
    
    loaded_reg.retry_unavailable_ports(0);
    
    // Check if it's now available
    EXPECT_TRUE(loaded_reg.get_unavailable_ports().empty());
    EXPECT_EQ(loaded_reg.ports().size(), 1u);
    EXPECT_TRUE(loaded_reg.ports().contains("test_retry_port"_ct.get_hash()));
    
    auto* loaded_conn = loaded_reg.connections().at("my_conn"_ct.get_hash()).get();
    EXPECT_EQ(loaded_conn->unavailable_inputs().size(), 0u);
    
    loaded_conn->ports_input().iterate([&](const auto& inputs) 
    {
        EXPECT_EQ(inputs.size(), 1u);
        if (!inputs.empty())
        {
            EXPECT_EQ(inputs[0]->get_name(), "test_retry_port"_ct);
        }
    });
}

/** @brief Tests valid_chain calculation on connections based on formats and ports. */
TEST_F(registry_test, connection_valid_chain)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    adam::port* in_port = nullptr;
    adam::port* out_port = nullptr;
    reg.create_port("in_port"_ct, adam::port_internal::type_name(), 0, &in_port);
    reg.create_port("out_port"_ct, adam::port_internal::type_name(), 0, &out_port);
    
    adam::connection* conn = nullptr;
    reg.create_connection("test_valid_conn"_ct, &conn);
    
    // Empty connection -> invalid
    EXPECT_FALSE(conn->check_valid_chain());
    
    reg.connection_add_port("test_valid_conn"_ct.get_hash(), "in_port"_ct.get_hash(), true);
    reg.connection_add_port("test_valid_conn"_ct.get_hash(), "out_port"_ct.get_hash(), false);
    
    // Transparent to transparent -> valid
    EXPECT_TRUE(conn->check_valid_chain());
    
    // Dummy formats
    adam::data_format dummy_fmt1("fmt1"_ct);
    adam::data_format dummy_fmt2("fmt2"_ct);
    
    conn->set_input_format(&dummy_fmt1);
    EXPECT_FALSE(conn->check_valid_chain()); // fmt1 != transparent
    
    conn->set_output_format(&dummy_fmt1);
    EXPECT_TRUE(conn->check_valid_chain()); // fmt1 == fmt1
    
    conn->set_output_format(&dummy_fmt2);
    EXPECT_FALSE(conn->check_valid_chain()); // fmt1 != fmt2

    // Add a processor mock module and processor
    adam::test::mock_module mock_mod("my_mod"_ct);
    mock_mod.register_processor_factory("my-proc-type"_ct, &adam::test::global_mock_proc_factory);
    adam::test::mock_module_injector injector(reg, &mock_mod);

    // Reset connection formats to transparent
    conn->set_input_format(&adam::data_format_transparent);
    conn->set_output_format(&adam::data_format_transparent);
    EXPECT_TRUE(conn->check_valid_chain());

    adam::processor* proc = nullptr;
    EXPECT_EQ(reg.create_processor("proc1"_ct, "my-proc-type"_ct, "my_mod"_ct, false, &proc), adam::registry::status_success);
    ASSERT_NE(proc, nullptr);

    // Add processor to connection
    EXPECT_EQ(reg.connection_add_processor("test_valid_conn"_ct.get_hash(), "proc1"_ct.get_hash()), adam::registry::status_success);

    // Default formats (transparent) -> valid
    EXPECT_TRUE(conn->check_valid_chain());

    // Cast processor to mock_processor to set its input/output formats
    auto* mock_proc = static_cast<adam::test::mock_processor*>(proc);
    
    // Connection: transparent -> mock_proc (fmt1 -> transparent) -> transparent -> invalid
    mock_proc->set_input_format(&dummy_fmt1);
    EXPECT_FALSE(conn->check_valid_chain());

    // Connection: fmt1 -> mock_proc (fmt1 -> transparent) -> transparent -> valid
    conn->set_input_format(&dummy_fmt1);
    EXPECT_TRUE(conn->check_valid_chain());

    // Connection: fmt1 -> mock_proc (fmt1 -> fmt2) -> transparent -> invalid
    mock_proc->set_output_format(&dummy_fmt2);
    EXPECT_FALSE(conn->check_valid_chain());

    // Connection: fmt1 -> mock_proc (fmt1 -> fmt2) -> fmt2 -> valid
    conn->set_output_format(&dummy_fmt2);
    EXPECT_TRUE(conn->check_valid_chain());
}

/** @brief Tests marking and retrying unavailable connections. */
TEST_F(registry_test, unavailable_connections)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    adam::connection* conn = nullptr;
    reg.create_connection("unavail_conn"_ct, &conn);
    
    // Set dummy module parameter to trigger unavailability when module is marked missing
    if (auto* mod_param = dynamic_cast<adam::configuration_parameter_string*>(conn->get_parameters().get("input_format_module"_ct)))
        mod_param->set_value("my_mod"_ct);
    
    if (auto* in_fmt_param = dynamic_cast<adam::configuration_parameter_string*>(conn->get_parameters().get("input_format"_ct)))
        in_fmt_param->set_value("my_fmt"_ct);

    EXPECT_EQ(reg.connections().size(), 1u);
    EXPECT_EQ(reg.get_unavailable_connections().size(), 0u);

    reg.mark_connections_unavailable("my_mod"_ct.get_hash());

    EXPECT_EQ(reg.connections().size(), 0u);
    EXPECT_EQ(reg.get_unavailable_connections().size(), 1u);
    
    // Simulate removing the module dependency by falling back to transparent format to safely test retry behavior natively
    auto* uconn = reg.get_unavailable_connections().at("unavail_conn"_ct.get_hash()).get();
    auto* fmt_p = dynamic_cast<adam::configuration_parameter_string*>(uconn->get_parameters().get("input_format"_ct));
    fmt_p->set_value(""_ct);
    
    auto* fmt_p_mod = dynamic_cast<adam::configuration_parameter_string*>(uconn->get_parameters().get("input_format_module"_ct));
    fmt_p_mod->set_value(""_ct);
    
    reg.retry_unavailable_connections("my_mod"_ct.get_hash());
    
    EXPECT_EQ(reg.connections().size(), 1u);
    EXPECT_EQ(reg.get_unavailable_connections().size(), 0u);
}

/** @brief Tests restoring and syncing of unavailable processors. */
TEST_F(registry_test, unavailable_processor_retry)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    
    // 1. Manually add an unavailable processor to the configuration
    auto upi = std::make_unique<adam::processor::unavailable_info>("my_unavail_proc"_ct);
    upi->type = "mock-converter"_ct.get_hash();
    upi->type_module = "mock_mod"_ct.get_hash();
    upi->is_filter = false;
    
    auto type_param = std::make_unique<adam::configuration_parameter_string>("type"_ct);
    type_param->set_value("mock-converter"_ct);
    upi->parameters().add(std::move(type_param));
    
    auto mod_param = std::make_unique<adam::configuration_parameter_string>("type_origin_module"_ct);
    mod_param->set_value("mock_mod"_ct);
    upi->parameters().add(std::move(mod_param));
    
    auto filter_param = std::make_unique<adam::configuration_parameter_boolean>("is_filter"_ct);
    filter_param->set_value(false);
    upi->parameters().add(std::move(filter_param));
    
    auto user_params = std::make_unique<adam::configuration_parameter_list_sorted>();
    user_params->set_name("user_parameters"_ct);
    upi->parameters().add(std::move(user_params));
    
    reg.unavailable_processors()["my_unavail_proc"_ct.get_hash()] = std::move(upi);
    
    // 2. Add it to a connection
    adam::connection* conn = nullptr;
    EXPECT_EQ(reg.create_connection("my_conn"_ct, &conn), adam::registry::status_success);
    conn->unavailable_processors().push_back("my_unavail_proc"_ct);
    
    if (auto* procs_list = dynamic_cast<adam::configuration_parameter_list_sorted*>(conn->get_parameters().get("processors"_ct)))
    {
        auto param = std::make_unique<adam::configuration_parameter_reference>("0"_ct);
        param->set_target("my_unavail_proc"_ct);
        procs_list->add(std::move(param));
    }
    
    // 3. Save and reload to ensure it persists
    EXPECT_TRUE(reg.save(test_filepath));
    
    adam::test::local_controller loaded_ctrl;
    adam::registry& loaded_reg = loaded_ctrl.get_registry();
    EXPECT_TRUE(loaded_reg.load(test_filepath));
    
    EXPECT_TRUE(loaded_reg.get_unavailable_processors().contains("my_unavail_proc"_ct.get_hash()));
    EXPECT_EQ(loaded_reg.connections().at("my_conn"_ct.get_hash())->unavailable_processors().size(), 1u);
    EXPECT_EQ(loaded_reg.processors().size(), 0u);
    
    // 4. Now let's register the mock module and retry/resolve the processor
    adam::test::mock_module mock_mod("mock_mod"_ct);
    mock_mod.register_processor_factory("mock-converter"_ct, &adam::test::global_mock_proc_factory);
    adam::test::mock_module_injector injector(loaded_reg, &mock_mod);
    
    loaded_reg.retry_unavailable_processors("mock_mod"_ct.get_hash());
    
    // Check if it's now available
    EXPECT_TRUE(loaded_reg.get_unavailable_processors().empty());
    EXPECT_EQ(loaded_reg.processors().size(), 1u);
    EXPECT_TRUE(loaded_reg.processors().contains("my_unavail_proc"_ct.get_hash()));
    
    auto* loaded_conn = loaded_reg.connections().at("my_conn"_ct.get_hash()).get();
    EXPECT_EQ(loaded_conn->unavailable_processors().size(), 0u);
    
    loaded_conn->processors().iterate([&](const auto& procs) 
    {
        EXPECT_EQ(procs.size(), 1u);
        if (!procs.empty())
        {
            EXPECT_EQ(procs[0]->get_name(), "my_unavail_proc"_ct);
        }
    });
}

/** @brief Tests marking and retrying unavailable processors. */
TEST_F(registry_test, unavailable_processors)
{
    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();

    // 1. Setup mock module with mock processor
    adam::test::mock_module mock_mod("mock_mod"_ct);

    adam::data_format dummy_fmt1("fmt1"_ct, nullptr, nullptr, &mock_mod);
    adam::data_format dummy_fmt2("fmt2"_ct, nullptr, nullptr, &mock_mod);

    mock_mod.register_processor_factory("mock-converter"_ct, &adam::test::global_mock_proc_factory, dummy_fmt1.get_name(), 0, dummy_fmt2.get_name(), 0);
    adam::test::mock_module_injector injector(reg, &mock_mod);

    // 2. Create input port, output port, processor, and connection
    adam::port* in_port = nullptr;
    adam::port* out_port = nullptr;
    EXPECT_EQ(reg.create_port("in_port"_ct, adam::port_internal::type_name(), 0, &in_port), adam::registry::status_success);
    EXPECT_EQ(reg.create_port("out_port"_ct, adam::port_internal::type_name(), 0, &out_port), adam::registry::status_success);

    adam::processor* proc = nullptr;
    EXPECT_EQ(reg.create_processor("proc1"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &proc), adam::registry::status_success);
    ASSERT_NE(proc, nullptr);

    dynamic_cast<adam::test::mock_processor*>(proc)->set_input_format(&dummy_fmt1);
    dynamic_cast<adam::test::mock_processor*>(proc)->set_output_format(&dummy_fmt2);

    adam::connection* conn = nullptr;
    EXPECT_EQ(reg.create_connection("conn1"_ct, &conn), adam::registry::status_success);
    ASSERT_NE(conn, nullptr);

    conn->set_input_format(&dummy_fmt1);
    conn->set_output_format(&dummy_fmt2);

    // 3. Link them to the connection
    EXPECT_EQ(reg.connection_add_port("conn1"_ct.get_hash(), "in_port"_ct.get_hash(), true), adam::registry::status_success);
    EXPECT_EQ(reg.connection_add_port("conn1"_ct.get_hash(), "out_port"_ct.get_hash(), false), adam::registry::status_success);
    
    // Without the processor, chain should be invlaid;
    EXPECT_FALSE(conn->check_valid_chain());

    EXPECT_EQ(reg.connection_add_processor("conn1"_ct.get_hash(), "proc1"_ct.get_hash()), adam::registry::status_success);

    // Now, the data chain should be valid
    EXPECT_TRUE(conn->check_valid_chain());

    // 4. Start the connection
    EXPECT_TRUE(conn->start());
    EXPECT_TRUE(conn->is_started());

    // 5. Mark the processor unavailable (unload the mock module)
    reg.mark_processors_unavailable("mock_mod"_ct.get_hash());

    // 6. Verify processor was moved to unavailable, connection stops, and connection chain is invalid
    EXPECT_FALSE(reg.processors().contains("proc1"_ct));
    EXPECT_TRUE(reg.get_unavailable_processors().contains("proc1"_ct.get_hash()));
    EXPECT_FALSE(conn->check_valid_chain()); // chain should be invalid because of unavailable processor

    // Double buffer should no longer contain the processor
    conn->processors().iterate([&](const auto& procs) 
    {
        EXPECT_TRUE(procs.empty());
    });
    // It should be in unavailable_processors list of connection
    EXPECT_EQ(conn->unavailable_processors().size(), 1u);
    EXPECT_EQ(conn->unavailable_processors()[0], "proc1"_ct);

    // 7. Retry/resolve the processor (reload the module)
    reg.retry_unavailable_processors("mock_mod"_ct.get_hash());

    // 8. Verify the processor is back, the connection is still stopped, but the chain is valid again!
    EXPECT_TRUE(reg.processors().contains("proc1"_ct));
    EXPECT_TRUE(reg.get_unavailable_processors().empty());
    EXPECT_FALSE(conn->is_started()); // shouldn't auto-start, just valid chain
    EXPECT_TRUE(conn->check_valid_chain()); // valid again because processor is available

    // Double buffer should contain the processor again
    conn->processors().iterate([&](const auto& procs) 
    {
        EXPECT_EQ(procs.size(), 1u);
        if (!procs.empty())
        {
            EXPECT_EQ(procs[0]->get_name(), "proc1"_ct);
        }
    });
    EXPECT_TRUE(conn->unavailable_processors().empty());
}


/** @brief Tests basic processor creation, renaming, and destruction. */
TEST_F(registry_test, create_rename_destroy_processor)
{
    adam::test::mock_module mock_mod("mock_mod"_ct);
    mock_mod.register_processor_factory("mock-converter"_ct, &adam::test::global_mock_proc_factory);

    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    adam::test::mock_module_injector injector(reg, &mock_mod);

    // Attempt to create a processor with an invalid type should fail
    adam::processor* invalid_proc = nullptr;
    EXPECT_EQ(reg.create_processor("test_proc_1"_ct, "non_existent_type"_ct, "mock_mod"_ct, false, &invalid_proc), adam::registry::status_error_factory_not_found);
    EXPECT_EQ(invalid_proc, nullptr);

    // Create a valid processor
    adam::processor* created_proc = nullptr;
    EXPECT_EQ(reg.create_processor("test_proc_1"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &created_proc), adam::registry::status_success);
    ASSERT_NE(created_proc, nullptr);
    EXPECT_EQ(reg.processors().size(), 1u);
    EXPECT_TRUE(reg.processors().contains("test_proc_1"_ct));

    // Attempt to create a duplicate processor should fail
    adam::processor* duplicate_proc = nullptr;
    EXPECT_EQ(reg.create_processor("test_proc_1"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &duplicate_proc), adam::registry::status_error_port_already_exists);
    EXPECT_EQ(duplicate_proc, nullptr);

    // Rename the processor
    EXPECT_EQ(reg.rename_processor("test_proc_1"_ct.get_hash(), "test_proc_new"_ct), adam::registry::status_success);
    EXPECT_EQ(reg.processors().size(), 1u);
    EXPECT_FALSE(reg.processors().contains("test_proc_1"_ct));
    EXPECT_TRUE(reg.processors().contains("test_proc_new"_ct));
    EXPECT_EQ(reg.processors().at("test_proc_new"_ct.get_hash())->get_name(), "test_proc_new"_ct);

    // Destroy the processor
    EXPECT_EQ(reg.destroy_processor("test_proc_new"_ct.get_hash()), adam::registry::status_success);
    EXPECT_EQ(reg.processors().size(), 0u);
    EXPECT_FALSE(reg.processors().contains("test_proc_new"_ct));
}

/** @brief Tests adding and removing a processor to/from a connection. */
TEST_F(registry_test, connection_processor_management)
{
    adam::test::mock_module mock_mod("mock_mod"_ct);
    mock_mod.register_processor_factory("mock-converter"_ct, &adam::test::global_mock_proc_factory);

    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    adam::test::mock_module_injector injector(reg, &mock_mod);

    adam::processor* proc = nullptr;
    EXPECT_EQ(reg.create_processor("proc1"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &proc), adam::registry::status_success);
    ASSERT_NE(proc, nullptr);

    // Create a connection
    adam::connection* conn = nullptr;
    EXPECT_EQ(reg.create_connection("conn1"_ct, &conn), adam::registry::status_success);
    ASSERT_NE(conn, nullptr);

    // Add processor to connection
    EXPECT_EQ(reg.connection_add_processor("conn1"_ct.get_hash(), "proc1"_ct.get_hash()), adam::registry::status_success);
    
    // Check it was added to connection double buffer
    bool found = false;
    conn->processors().iterate([&](const auto& procs) {
        EXPECT_EQ(procs.size(), 1u);
        if (!procs.empty() && procs[0] == proc) found = true;
    });
    EXPECT_TRUE(found);

    // Check configuration parameters
    auto* procs_list = dynamic_cast<adam::configuration_parameter_list*>(conn->get_parameters().get("processors"_ct));
    ASSERT_NE(procs_list, nullptr);
    EXPECT_EQ(procs_list->get_children().size(), 1u);
    auto* ref = dynamic_cast<adam::configuration_parameter_reference*>(procs_list->get("0"_ct));
    ASSERT_NE(ref, nullptr);
    EXPECT_EQ(ref->get_target(), "proc1"_ct);

    // Remove processor from connection
    EXPECT_EQ(reg.connection_remove_processor("conn1"_ct.get_hash(), "proc1"_ct.get_hash()), adam::registry::status_success);
    
    // Check it was removed from double buffer
    found = false;
    conn->processors().iterate([&](const auto& procs) {
        if (procs.empty()) found = true;
    });
    EXPECT_TRUE(found);
    EXPECT_EQ(procs_list->get_children().size(), 0u);
}

/** @brief Tests that destroying a processor cleans up any references in connections. */
TEST_F(registry_test, destroy_processor_updates_connections)
{
    adam::test::mock_module mock_mod("mock_mod"_ct);
    mock_mod.register_processor_factory("mock-converter"_ct, &adam::test::global_mock_proc_factory);

    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    adam::test::mock_module_injector injector(reg, &mock_mod);

    adam::processor* proc = nullptr;
    EXPECT_EQ(reg.create_processor("proc_to_del"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &proc), adam::registry::status_success);

    // Create a connection and add processor
    adam::connection* conn = nullptr;
    EXPECT_EQ(reg.create_connection("conn_with_proc"_ct, &conn), adam::registry::status_success);
    EXPECT_EQ(reg.connection_add_processor("conn_with_proc"_ct.get_hash(), "proc_to_del"_ct.get_hash()), adam::registry::status_success);

    // Verify presence in connection double buffer
    bool found = false;
    conn->processors().iterate([&](const auto& procs) 
    {
        if (!procs.empty() && procs[0] == proc) found = true;
    });
    EXPECT_TRUE(found);

    // Destroy processor directly
    EXPECT_EQ(reg.destroy_processor("proc_to_del"_ct.get_hash()), adam::registry::status_success);

    // Verify it was removed from connection double buffer
    found = false;
    conn->processors().iterate([&](const auto& procs) 
    {
        if (procs.empty()) found = true;
    });
    EXPECT_TRUE(found);
}

/** @brief Tests processor serialization (saving and loading) and ordering works correctly. */
TEST_F(registry_test, processor_load_save_persistence)
{
    adam::test::mock_module mock_mod("mock_mod"_ct);
    mock_mod.register_processor_factory("mock-converter"_ct, &adam::test::global_mock_proc_factory);

    adam::test::local_controller ctrl;
    adam::registry& reg = ctrl.get_registry();
    adam::test::mock_module_injector injector(reg, &mock_mod);

    adam::processor* proc1 = nullptr;
    adam::processor* proc2 = nullptr;
    adam::processor* proc3 = nullptr;
    EXPECT_EQ(reg.create_processor("proc_first"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &proc1), adam::registry::status_success);
    EXPECT_EQ(reg.create_processor("proc_second"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &proc2), adam::registry::status_success);
    EXPECT_EQ(reg.create_processor("proc_third"_ct, "mock-converter"_ct, "mock_mod"_ct, false, &proc3), adam::registry::status_success);

    adam::connection* conn = nullptr;
    EXPECT_EQ(reg.create_connection("persisted_conn"_ct, &conn), adam::registry::status_success);
    EXPECT_EQ(reg.connection_add_processor("persisted_conn"_ct.get_hash(), "proc_first"_ct.get_hash()), adam::registry::status_success);
    EXPECT_EQ(reg.connection_add_processor("persisted_conn"_ct.get_hash(), "proc_second"_ct.get_hash()), adam::registry::status_success);
    EXPECT_EQ(reg.connection_add_processor("persisted_conn"_ct.get_hash(), "proc_third"_ct.get_hash()), adam::registry::status_success);

    EXPECT_TRUE(reg.save(test_filepath));

    // Load from binary in a fresh registry
    adam::test::local_controller loaded_ctrl;
    adam::registry& loaded_reg = loaded_ctrl.get_registry();
    adam::test::mock_module_injector loaded_injector(loaded_reg, &mock_mod);
    
    EXPECT_TRUE(loaded_reg.load(test_filepath)); // TODO: load will remove all currently loaded modules, so we need another workaround

    // Verify processors were recreated
    EXPECT_EQ(loaded_reg.processors().size(), 3u);
    EXPECT_TRUE(loaded_reg.processors().contains("proc_first"_ct));
    EXPECT_TRUE(loaded_reg.processors().contains("proc_second"_ct));
    EXPECT_TRUE(loaded_reg.processors().contains("proc_third"_ct));

    // Verify connection and its double-buffered processor list
    EXPECT_EQ(loaded_reg.connections().size(), 1u);
    EXPECT_TRUE(loaded_reg.connections().contains("persisted_conn"_ct));
    
    adam::connection* loaded_conn = loaded_reg.connections().at("persisted_conn"_ct.get_hash()).get();
    
    // 1. Verify the double buffer order
    loaded_conn->processors().iterate([&](const auto& procs) {
        ASSERT_EQ(procs.size(), 3u);
        EXPECT_EQ(procs[0]->get_name(), "proc_first"_ct);
        EXPECT_EQ(procs[1]->get_name(), "proc_second"_ct);
        EXPECT_EQ(procs[2]->get_name(), "proc_third"_ct);
    });

    // 2. Verify the configuration parameter order (sorted list order)
    auto* procs_list = dynamic_cast<adam::configuration_parameter_list_sorted*>(loaded_conn->get_parameters().get("processors"_ct));
    ASSERT_NE(procs_list, nullptr);
    const auto& order = procs_list->get_order();
    ASSERT_EQ(order.size(), 3u);
    
    auto* ref0 = dynamic_cast<adam::configuration_parameter_reference*>(procs_list->get("0"_ct));
    auto* ref1 = dynamic_cast<adam::configuration_parameter_reference*>(procs_list->get("1"_ct));
    auto* ref2 = dynamic_cast<adam::configuration_parameter_reference*>(procs_list->get("2"_ct));
    
    ASSERT_NE(ref0, nullptr);
    ASSERT_NE(ref1, nullptr);
    ASSERT_NE(ref2, nullptr);
    
    EXPECT_EQ(ref0->get_target(), "proc_first"_ct);
    EXPECT_EQ(ref1->get_target(), "proc_second"_ct);
    EXPECT_EQ(ref2->get_target(), "proc_third"_ct);

    // Verify the insertion order matches
    EXPECT_EQ(order[0], "0"_ct.get_hash());
    EXPECT_EQ(order[1], "1"_ct.get_hash());
    EXPECT_EQ(order[2], "2"_ct.get_hash());
}