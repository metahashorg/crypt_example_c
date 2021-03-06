#include "settings.h"

//#include <iostream>
//#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "log.h"
#include "string_utils.h"

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/program_options.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace   pt      = boost::property_tree;
namespace	asio    = boost::asio;
namespace	ip      = boost::asio::ip;
using		tcp     = boost::asio::ip::tcp;

namespace settings
{
    // service
    bool service::any_conns = {false};
    unsigned short service::port = {9999};
    int service::threads = {4};
    bool service::keep_alive = {false};
    std::vector<std::string> service::access;
    bool service::auth_enable = {false};
    std::string service::auth_key;

    std::string statistic::statisticNetwork;
    std::string statistic::statisticGroup;
    std::string statistic::statisticServer;
    std::string statistic::latencyFile;
    
    // server
    std::string server::torName;
    std::string server::proxyName;
    ATOMIC_PROP_IMPL(std::string, tor);
    ATOMIC_PROP_IMPL(std::string, proxy);
    
    // system
    std::string system::wallet_stotage = { "./wallet" };
    std::string system::leveldbFolder = "leveldb/";
    std::string system::blocksFolder = "blocks/";
    bool system::validateBlocks = false;
    bool system::useLocalDatabase = true;
    bool system::allowStateBlocks = false;
    unsigned int system::jrpc_conn_timeout = 1000;
    unsigned int system::jrpc_timeout = 60000;
    unsigned int system::jrpc_attempts_count = 2;
    bool system::conn_pool_enable = false;
    unsigned int system::conn_pool_ttl = 60; // SECONDS!
    unsigned int system::conn_pool_capacity = 100;
    unsigned int system::blocks_cache_ver = 1;
    bool system::blocks_cache_enable = false;
    bool system::blocks_cache_force = true;
    unsigned int system::blocks_cache_init_count = 50000;
    unsigned int system::blocks_cache_recv_data_size = 5;
    unsigned int system::blocks_cache_recv_count = 100;
    bool system::blocks_cache_block_verification = true;
    std::vector<std::string> system::cores;
    bool system::history_cache_enable = {false};
    std::vector<std::string> system::history_cache_addrs;
    bool system::validate_request = true;
    bool system::validate_methods = true;

    void read(const std::string &pathToConfig) {
        pt::ptree tree;
        
        std::string path;
        if (pathToConfig.empty()) {
            path.append("./settings.json");
        } else {
            path = pathToConfig;
        }
        
        pt::read_json(path.c_str(), tree);

        service::port = tree.get<unsigned short>("service.port", 9999);
        service::threads = tree.get<int>("service.threads", 4);
        service::keep_alive = tree.get<bool>("service.keep_alive", false);
        service::auth_enable = tree.get<bool>("service.auth_enable", false);
        service::auth_key = tree.get<std::string>("service.auth_key", "");
        if (service::auth_key.empty()) {
            service::auth_enable = false;
        }

        asio::io_context ctx;
        tcp::resolver resolver(ctx);
        boost::property_tree::ptree access;
        access = tree.get_child("service.access", access);
        for (auto &v : access) {
            boost::system::error_code er;
            auto eps = resolver.resolve({v.second.data(), ""}, er);
            if (er) {
                LOGWARN << "Couldn't resolve " << v.second.data() << " : " << er.message();
                continue;
            }
            for (auto &e : eps) {
                service::access.push_back(e.endpoint().address().to_string());
            }
        }

        server::torName     = tree.get<std::string>("server.tor", "127.0.0.1");
        server::proxyName   = tree.get<std::string>("server.proxy", "127.0.0.1");
        
        system::wallet_stotage = tree.get<std::string>("system.wallets-storage", "./wallet");
        system::jrpc_conn_timeout = tree.get<unsigned int>("system.jrpc_conn_timeout", 1000);
        system::jrpc_timeout = tree.get<unsigned int>("system.jrpc_timeout", 60000);
        system::jrpc_attempts_count = tree.get<unsigned int>("system.jrpc_attempts_count", 2);
        system::leveldbFolder = tree.get<std::string>("system.leveldb_folder");
        system::blocksFolder = tree.get<std::string>("system.blocks_folder");
        system::validateBlocks = tree.get<bool>("system.validate_blocks");
        system::useLocalDatabase = tree.get<bool>("system.use_local_database");
        system::allowStateBlocks = tree.get<bool>("system.allow_state_blocks", false);

        system::conn_pool_enable = tree.get<bool>("system.conn_pool_enable", false);
        system::conn_pool_ttl = tree.get<unsigned int>("system.conn_pool_ttl", 60);
        system::conn_pool_capacity = tree.get<unsigned int>("system.conn_pool_capacity", 100);

        system::blocks_cache_ver = tree.get<unsigned int>("system.blocks_cache_ver", 2);
        system::blocks_cache_enable = tree.get<bool>("system.blocks_cache_enable", false);
        system::blocks_cache_force = tree.get<bool>("system.blocks_cache_force", true);
        system::blocks_cache_init_count = tree.get<unsigned int>("system.blocks_cache_init_count", 50000);
        system::blocks_cache_recv_data_size = tree.get<unsigned int>("system.blocks_cache_recv_data_size", 5);
        system::blocks_cache_recv_count = tree.get<unsigned int>("system.blocks_cache_recv_count", 100);
        system::blocks_cache_block_verification = tree.get<bool>("system.blocks_cache_block_verification", true);

        boost::property_tree::ptree cores;
        cores = tree.get_child("system.cores", cores);
        for (auto &v : cores) {
            system::cores.emplace_back(string_utils::to_lower(v.second.data()));
        }

        system::history_cache_enable = tree.get<bool>("system.history_cache_enable", false);

        boost::property_tree::ptree hist_addrs;
        hist_addrs = tree.get_child("system.history_cache_addresses", hist_addrs);
        for (auto &v : hist_addrs) {
            system::history_cache_addrs.emplace_back(string_utils::to_lower(v.second.data()));
        }
        if (system::history_cache_addrs.empty()) {
            system::history_cache_enable = false;
        }

        system::validate_request = tree.get<bool>("system.validate_request", true);
        system::validate_methods = tree.get<bool>("system.validate_methods", true);

        if (tree.find("statistic") != tree.not_found()) {
            statistic::statisticNetwork = tree.get<std::string>("statistic.network");
            statistic::statisticGroup = tree.get<std::string>("statistic.group");
            statistic::statisticServer = tree.get<std::string>("statistic.server");
            statistic::latencyFile = tree.get<std::string>("statistic.latency_file");
        }
    }

    std::string getConfigPath(boost::program_options::variables_map& vm) {
        if (vm.count("any")) {
            service::any_conns = true;
        }
        
        if (vm.count("config")) {
            return vm["config"].as<std::string>();
        }
        return "";
    }
}
