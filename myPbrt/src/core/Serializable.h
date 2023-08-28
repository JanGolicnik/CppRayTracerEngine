#pragma once

#include <json/json.h>

namespace MyPBRT{

	class Serializable
	{
	public:
		virtual void DeSerialize(const Json::Value& node) = 0;
		virtual Json::Value Serialize() const = 0;
		virtual std::string GetType() const { return ""; };
	};

}

