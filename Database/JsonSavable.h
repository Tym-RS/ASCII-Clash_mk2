#ifndef ASCII_CLASH_JSONSAVABLE_H
#define ASCII_CLASH_JSONSAVABLE_H
#include <nlohmann/json.hpp>

template<typename T>
class JsonSavable {
protected:
#ifndef NDEBUG
#include <type_traits>

    JsonSavable() {
        static_assert(
            std::is_invocable_r_v<T *, decltype(&T::FromJson), nlohmann::json>,
            "Derived class must implement: static T* FromJson(nlohmann::json j)"
        );
    }
#endif

public:
    virtual ~JsonSavable() = default;

    virtual nlohmann::json ToJson() = 0;
};
#endif
