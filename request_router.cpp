#include "request_router.h"

namespace router
{
	static request_verb_e classify_verb_string(string verb_raw)
	{
		if(verb_raw == "GET")
			return RV_GET;
		else if(verb_raw == "POST")
			return RV_POST;
		else if(verb_raw == "DELETE")
			return RV_DELETE;
		else if(verb_raw == "PUT")
			return RV_PUT;
		else
			return _RV_NULL;
	}

	static StringPair extract_pair(string url, int pos,
								   string::size_type length)
	{
		string pair_string(url, pos, length);
		string::size_type e_at = pair_string.find('=');
	
		string key = pair_string.substr(0, e_at);
		string value = (e_at != string::npos ? pair_string.substr(e_at + 1) :
				                               string());

		StringPair pair(key, value);
		return pair;
	}

	map<string, string> get_query_parameters(string url)
	{
		map<string, string> query_parameters;

		string::size_type q_at = url.find('?');
		string::size_type found_at = -1;
		string::size_type last_start_at = q_at + 1;
		string::size_type check_from_pos = last_start_at;

		if(q_at != string::npos && q_at < (url.size() - 1))
		{
			while((found_at = url.find('&', check_from_pos)) != string::npos)
			{
				// Keep going if we've encountered an HTML entity, and not a 
				// separating ampersand. We define an HTML entity as an
				// ampersand followed by a semi-colon prior to the next
				// ampersand within ten-characters of each other.
				string::size_type s_at = url.find(';', found_at + 1);
				string::size_type a_lookahead = url.find('&', found_at + 1);
				if(s_at != string::npos &&
				   a_lookahead != string::npos &&
				   s_at < a_lookahead &&
				   (s_at - found_at) < 10)
				{
					check_from_pos = found_at + 1;
					continue;
				}

				StringPair pair = extract_pair(url,
						                       last_start_at,
						                       found_at - last_start_at);

				query_parameters[pair.get_key()] = pair.get_value();

				last_start_at = found_at + 1;
				check_from_pos = last_start_at;
			}

			StringPair pair = extract_pair(url,
					                       last_start_at,
					                       url.size() - last_start_at);

			query_parameters[pair.get_key()] = pair.get_value();
		}

		return query_parameters;
	}

	Response request_router(void *server_info_raw, string resource_path,
						     vector<Header> headers, string verb_raw,
						     string body)
	{
		// Route the incoming request using the mappings that have been
		// previously configured. We expect an instance of
		// request_router_data_t in the request_handler_data.

		ServerInfo *server_info = (ServerInfo *)server_info_raw;
		RequestRouterData *request_router_data =
				(RequestRouterData *)server_info->get_request_handler_data();

		ActionDescriptor action_descriptor;
		string resource_path_parameters;
		
		RouteMappings *mappings = request_router_data->get_mappings();
		if(mappings->TryMap(string(resource_path),
							action_descriptor,
							resource_path_parameters) == false)
			return get_404_response(resource_path);

		action_handler_t action_handler =
				(action_handler_t)action_descriptor.get_action_handler();

		request_verb_e verb = classify_verb_string(verb_raw);

		if(verb == 0)
			return get_general_response(405,
										resource_path,
										"Verb/method "
											"[" + string(verb_raw) + "] "
											"is not allowed.");

		map<string, string> parameters = get_query_parameters(resource_path);

		return action_handler(server_info_raw,
							  action_descriptor,
							  string(resource_path),
							  parameters,
							  headers,
							  verb,
							  body);
	}

	RouteMappings::RouteMappings()
	{
	
	}

	bool RouteMappings::Add(int route_id, const char *route_prefix,
							action_handler_t action_handler)
	{
		ActionDescriptor action_descriptor;
		
		action_descriptor.set_id(route_id);
		action_descriptor.set_resource_path_prefix(route_prefix);
		action_descriptor.set_action_handler((void *)action_handler);

		mappings.push_back(action_descriptor);

		return true;
	}

	bool RouteMappings::TryMap(string resource_path,
							   ActionDescriptor &action_descriptor,
							   string resource_path_suffix)
	{
		vector<ActionDescriptor>::iterator it = mappings.begin();
		while(it != mappings.end())
		{
			int prefix_length = it->get_resource_path_prefix().size();
			if(resource_path.substr(0, prefix_length) ==
					it->get_resource_path_prefix())
			{
				action_descriptor.load_from(*it);
				resource_path_suffix = resource_path.substr(prefix_length);
				return true;
			}

			it++;
		}

		return false;
	}

	ActionDescriptor::ActionDescriptor()
	{
		this->id = 0;
		this->resource_path_prefix = "";
		this->action_handler = NULL;
	}

	ActionDescriptor::ActionDescriptor(int id, string resource_path_prefix,
									   void *action_handler)
	{
		this->id = id;
		this->resource_path_prefix = resource_path_prefix;
		this->action_handler = action_handler;
	}

	void ActionDescriptor::load_from(ActionDescriptor original)
	{
		id = original.get_id();
		resource_path_prefix = original.get_resource_path_prefix();
		action_handler = original.get_action_handler();
	}

	RequestRouterData::RequestRouterData()
	{
		this->mappings = new RouteMappings();
		allocated = true;
	}

	RequestRouterData::~RequestRouterData()
	{
		if(allocated)
			delete mappings;
	}

	RequestRouterData::RequestRouterData(RouteMappings *mappings)
	{
		this->mappings = mappings;
		allocated = false;
	}
}
