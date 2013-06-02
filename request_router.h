#ifndef __REQUESTROUTER_H
#define __REQUESTROUTER_H

#include <vector>
#include <string>
#include <map>

#include "web.h"

using namespace std;

namespace router
{
	Response request_router(void *server_info, string resource_path,
							 vector<Header> headers, string verb,
							 string body);

	typedef enum { _RV_NULL,
		           RV_GET,
		           RV_POST,
		           RV_DELETE,
		           RV_PUT } request_verb_e;

	class ActionDescriptor
	{
		int id;
		string resource_path_prefix;
		void *action_handler;

		public:
			ActionDescriptor();
			ActionDescriptor(int id, string resource_path_prefix,
							 void *action_handler);

			void load_from(ActionDescriptor original);

			int get_id() const { return id; }
			string get_resource_path_prefix() const
				{ return resource_path_prefix; }
			void *get_action_handler() const { return action_handler; }

			void set_id(int value) { id = value; }
			void set_resource_path_prefix(string value)
				{ resource_path_prefix = value; }
			void set_action_handler(void *value) { action_handler = value; }
	};

	typedef Response (*action_handler_t)(void *server_info,
										  ActionDescriptor action,
										  string resource_path,
										  map<string, string> parameters,
										  vector<Header> headers,
										  request_verb_e verb,
										  string body);

	class RouteMappings
	{
		vector<ActionDescriptor> mappings;

		public:
			RouteMappings();
			bool Add(int route_id, const char *route_prefix,
					 action_handler_t action_handler);

			bool TryMap(string resource_path,
						ActionDescriptor &action_descriptor,
						string resource_path_suffix);
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
		string key;
		string value;

		public:
			StringPair(string key, string value) // : BaseObject("StringPair")
			{
				this->key = key;
				this->value = value;
			}

			string get_key() const { return key; }
			string get_value() const { return value; }

			void set_key(string value) { key = value; }
			void set_value(string value_) { value = value_; }
	};

	map<string, string> get_query_parameters(string url);
}

#endif
