#pragma once
/**
 * @file BasicEvent.h
 * @brief Base interface for all events. Handles JSON serializing for frontend and stats storing and output.
 */

#include <json/json.h>
#include <type_traits>


/**
 * @brief Base class for events; provides default meters hook and JSON array helpers.
 */
class BasicEvent
{
public:
    // Add meters here if stats required.
    // Three types of threadsafe meters are available.
    // - IntSumMeter: for anything where update involves adding an int to a running total.
    //      E.g.
    //      inline static IntSumMeter eventTotalWin{};
    // - IntMaxMeter: for a max value across all simulations.
    //   Also tracks number of hits on the max value.
    //      E.g.
    //      inline static IntMaxMeter eventMaxWin{};
    // - DistributionMeter: Uses an unordered_map<int, uint64_t> on the backend.
    //   Updates involve adding something to one of the values and creating it if necessary.
    //      E.g.
    //      inline static DistributionMeter totalWinningsByNumScatters{};
    virtual ~BasicEvent() = default;

    /**
     * @brief Default metrics hook for events with no meters.
     * @details updateMeters must take exactly the same arguments as the Event constructor even if unused.
     */
    static void updateMeters()
    {
        // E.g. (using example meters above)
        // eventTotalWin++;
        // eventTotalWin += win;
        // eventMaxWin.update(win);
        // totalWinningsByNumScatters[numScatters] += winnings;
        // etc.
    }

    /**
     * @brief Serialize this event to JSON. Override in derived classes if frontend visualization required.
     * @details If overridden, must return a Json::Value with a "eventType" property.
     */
    virtual Json::Value toJson() const
    {
        return Json::nullValue;
    };

    virtual std::string prettyPrint(const std::string& baseIndent)
    {
        Json::Value jsonEvent = toJson();
        if (jsonEvent.isNull()) return "";
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  "; // two spaces (default is 3)


        std::string json = Json::writeString(builder, jsonEvent);

        std::ostringstream out;
        std::istringstream in(json);
        std::string line;

        while (std::getline(in, line))
        {
            out << baseIndent << line << '\n';
        }

        return out.str();
    }

protected:
    template <
        typename Iterable,
        typename T = std::decay_t<decltype(*std::begin(std::declval<Iterable&>()))>,
        std::enable_if_t<
            (std::is_convertible_v<T, Json::Value>) || (std::is_integral_v<T>),
            int> = 0
    >
    /**
     * @brief Convert a 1D iterable to a JSON array.
     * @tparam Iterable Container whose elements are integral or convertible to Json::Value.
     * @param items Items to serialize.
     */
    static Json::Value toJsonArray(const Iterable& items)
    {
        Json::Value arr(Json::arrayValue);

        for (const auto& item : items)
        {
            if constexpr (std::is_integral_v<T>)
            {
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
            !std::is_convertible_v<Row, Json::Value> && // avoid clashing with 1D for e.g. std::string
            (std::is_convertible_v<T, Json::Value> || std::is_integral_v<T>),
            int> = 0
    >
    /**
     * @brief Convert a 2D iterable (rows/cols) to a JSON array of arrays.
     * @tparam Iterable Outer container whose rows are iterable of integrals or Json-convertible items.
     * @param rows Rows to serialize.
     */
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
