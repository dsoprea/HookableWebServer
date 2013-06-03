#ifndef __REQUESTROUTER_H
#define __REQUESTROUTER_H

#include <vector>
#include <string>
#include <map>

#include "web.h"

namespace router
{
    web::Response request_router(void *server_info, std::string resource_path,
                                 std::vector<web::Header> headers,
                                 std::string verb, std::string body);

    typedef enum { _RV_NULL,
                   RV_GET,
                   RV_POST,
                   RV_DELETE,
                   RV_PUT } request_verb_e;

    class ActionDescriptor
    {
        int id;
        std::string resource_path_prefix;
        void *action_handler;

        public:
            ActionDescriptor();
            ActionDescriptor(int id, std::string resource_path_prefix,
                             void *action_handler);

            void load_from(ActionDescriptor original);

            int get_id() const { return id; }
            std::string get_resource_path_prefix() const
                { return resource_path_prefix; }
            void *get_action_handler() const { return action_handler; }

            void set_id(int value) { id = value; }
            void set_resource_path_prefix(std::string value)
                { resource_path_prefix = value; }
            void set_action_handler(void *value) { action_handler = value; }
    };

    typedef web::Response (*action_handler_t)(void *server_info,
                                                ActionDescriptor action,
                                                std::string resource_path,
                                                std::map<std::string, std::string> parameters,
                                                std::vector<web::Header> headers,
                                                request_verb_e verb,
                                                std::string body);

    class RouteMappings
    {
        std::vector<ActionDescriptor> mappings;

        public:
            RouteMappings();
            bool Add(int route_id, const char *route_prefix,
                     action_handler_t action_handler);

            bool TryMap(std::string resource_path,
                        ActionDescriptor &action_descriptor,
                        std::string resource_path_suffix);
    };

    class RequestRouterData
    {
        RouteMappings *mappings;
        bool allocated;

        public:
            RequestRouterData();
            RequestRouterData(RouteMappings *mappings);
            ~RequestRouterData();

            RouteMappings *get_mappings() { return mappings; }
    };

    class StringPair
    {
        std::string key;
        std::string value;

        public:
            StringPair(std::string key, std::string value)
            {
                this->key = key;
                this->value = value;
            }

            std::string get_key() const { return key; }
            std::string get_value() const { return value; }

            void set_key(std::string value) { key = value; }
            void set_value(std::string value_) { value = value_; }
    };

    std::map<std::string, std::string> get_query_parameters(std::string url);
}

#endif
