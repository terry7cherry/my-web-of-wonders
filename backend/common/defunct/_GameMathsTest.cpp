#include <string>
#include <unordered_map>
#include <typeindex>
#include <stdexcept>
#include <type_traits>

class __GameMaths {
public:
    template<typename T>
    void add(const std::string &key, T *ptr);

    template<typename T>
    const T *tryGet(const std::string &key) const;

    template<typename T>
    const T &get(const std::string &key) const;

    // factory for a node view
    class GameMathsNode node(const std::string &prefix);

private:
    using InnerMap = std::unordered_map<std::string, void *>;
    std::unordered_map<std::type_index, InnerMap> storage_;
};

class GameMathsNode {
public:
    GameMathsNode(std::string prefix, __GameMaths &gameMaths)
        : prefix_(std::move(prefix)), gameMaths_(gameMaths) {
    }

    template<typename T>
    const T &get(const std::string &key) const {
        std::string fullKey = prefix_ + " " + key;
        return gameMaths_.get<T>(fullKey);
    }

private:
    std::string prefix_;
    const __GameMaths &gameMaths_;
};

// ---- GameMaths template definitions ----

template<typename T>
void __GameMaths::add(const std::string &key, T *ptr) {
    auto &inner = storage_[std::type_index(typeid(T))];
    inner[key] = static_cast<void *>(ptr);
}

template<typename T>
const T *__GameMaths::tryGet(const std::string &key) const {
    auto typeIt = storage_.find(std::type_index(typeid(T)));
    if (typeIt == storage_.end()) return nullptr;

    const auto &inner = typeIt->second;
    auto it = inner.find(key);
    if (it == inner.end()) return nullptr;

    return static_cast<const T *>(it->second);
}

template<typename T>
const T &__GameMaths::get(const std::string &key) const {
    const T *p = tryGet<T>(key);
    if (!p) {
        throw std::out_of_range("Key/type not found in GameMaths");
    }
    return *p;
}

// factory method (non-template)
inline GameMathsNode __GameMaths::node(const std::string &prefix) {
    return GameMathsNode(prefix, *this);
}
