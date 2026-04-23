#ifndef ASCII_CLASH_JSONSAVABLE_H
#define ASCII_CLASH_JSONSAVABLE_H
#include <nlohmann/json.hpp>


class JsonSavable {
public:
    virtual ~JsonSavable() = default;

    [[nodiscard]] virtual nlohmann::json ToJson() const = 0;
};
#endif
