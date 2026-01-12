#pragma once

#include <json/json.h>
#include <string_view>


class BasicEvent
{
public:
	virtual ~BasicEvent() = default;

	static void updateMeters()
	{
	}

	Json::Value toJson() const
	{
		Json::Value eventAsJson(Json::arrayValue);
		eventAsJson[0] = std::string(getEventType());
		eventAsJson[1] = getDetailsAsJson();
		return eventAsJson;
	};

protected:
	virtual std::string_view getEventType() const = 0;

	virtual Json::Value getDetailsAsJson() const
	{
		Json::Value eventDetails;
		return eventDetails;
	};

	template <
		typename Iterable,
		typename T = std::decay_t<decltype(*std::begin(std::declval<Iterable&>()))>,
		std::enable_if_t<std::is_convertible<T, Json::Value>::value, int> = 0
	>
	static Json::Value toJsonArray(const Iterable& items)
	{
		Json::Value arr(Json::arrayValue);

		for (const auto& item : items)
		{
			arr.append(Json::Value(item)); // implicit conversion to Json::Value
		}

		return arr;
	};

	template <
		typename Iterable,
		typename Row = std::decay_t<decltype(*std::begin(std::declval<Iterable&>()))>,
		typename T = std::decay_t<decltype(*std::begin(std::declval<Row&>()))>,
		std::enable_if_t<
			!std::is_convertible<Row, Json::Value>::value && // avoid clashing with 1D for e.g. std::string
			std::is_convertible<T, Json::Value>::value,
			int> = 0
	>
	static Json::Value toJsonArray(const Iterable& rows)
	{
		Json::Value outer(Json::arrayValue);

		for (const auto& row : rows)
		{
			Json::Value inner(Json::arrayValue);
			for (const auto& elem : row)
			{
				inner.append(Json::Value(elem));
			}
			outer.append(std::move(inner));
		}

		return outer;
	}
};
