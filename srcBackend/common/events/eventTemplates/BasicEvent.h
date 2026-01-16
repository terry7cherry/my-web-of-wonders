#pragma once

#include <json/json.h>
#include <string_view>
#include <type_traits>


class BasicEvent
{
public:
	virtual ~BasicEvent() = default;

	static void updateMeters()
	{
	}

	virtual Json::Value toJson() const
	{
		return Json::nullValue;
	};

protected:
 template <
        typename Iterable,
        typename T = std::decay_t<decltype(*std::begin(std::declval<Iterable&>()))>,
        std::enable_if_t<
            (std::is_convertible<T, Json::Value>::value) || (std::is_integral_v<T>),
            int> = 0
    >
    static Json::Value toJsonArray(const Iterable& items)
    {
        Json::Value arr(Json::arrayValue);

        for (const auto& item : items)
        {
            if constexpr (std::is_integral_v<T>)
            {
                // Be explicit for portability across compilers/platforms
                arr.append(Json::Value(static_cast<Json::UInt64>(item)));
            }
            else
            {
                arr.append(Json::Value(item)); // implicit conversion to Json::Value
            }
        }

        return arr;
    };

 template <
        typename Iterable,
        typename Row = std::decay_t<decltype(*std::begin(std::declval<Iterable&>()))>,
        typename T = std::decay_t<decltype(*std::begin(std::declval<Row&>()))>,
        std::enable_if_t<
            !std::is_convertible<Row, Json::Value>::value && // avoid clashing with 1D for e.g. std::string
            (std::is_convertible<T, Json::Value>::value || std::is_integral_v<T>),
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
                if constexpr (std::is_integral_v<T>)
                {
                    inner.append(Json::Value(static_cast<Json::UInt64>(elem)));
                }
                else
                {
                    inner.append(Json::Value(elem));
                }
            }
            outer.append(std::move(inner));
        }

        return outer;
    }
};
