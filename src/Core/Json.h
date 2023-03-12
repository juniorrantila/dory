#pragma once
#include "File.h"
#include "Ty/Forward.h"
#include <Ty/Assert.h>
#include <Ty/LinearMap.h>
#include <Ty/Vector.h>

namespace Core {

struct Json;
struct JsonValue;
using JsonObject = LinearMap<StringView, JsonValue>;
using JsonArray = Vector<JsonValue>;

struct JsonValue {
    enum Type : u8 {
        Array,
        Bool,
        Null,
        Number,
        Object,
        String,
    };

    constexpr JsonValue(bool value)
        : m_bool(value)
        , m_type(Bool)
    {
    }

    constexpr JsonValue(nullptr_t)
        : m_type(Null)
    {
    }

    constexpr JsonValue(double value)
        : m_number(value)
        , m_type(Number)
    {
    }

    constexpr JsonValue(Id<JsonArray> value)
        : m_array(value)
        , m_type(Array)
    {
    }

    constexpr JsonValue(Id<JsonObject> value)
        : m_object(value)
        , m_type(Object)
    {
    }

    constexpr JsonValue(StringView value)
        : m_string(value)
        , m_type(String)
    {
    }

    constexpr Type type() const { return m_type; }

    constexpr bool unsafe_as_bool() const { return m_bool; }
    constexpr double unsafe_as_number() const { return m_number; }
    constexpr Id<JsonObject> unsafe_as_object() const
    {
        return m_object;
    }
    constexpr Id<JsonArray> unsafe_as_array() const
    {
        return m_array;
    }

    constexpr StringView unsafe_as_string() const
    {
        return m_string;
    }

    constexpr ErrorOr<bool> as_bool() const
    {
        ASSERT(m_type == Bool);
        return m_bool;
    }

    constexpr ErrorOr<double> as_number() const
    {
        ASSERT(m_type == Number);
        return m_number;
    }
    constexpr ErrorOr<Id<JsonObject>> as_object() const
    {
        ASSERT(m_type == Object);
        return m_object;
    }
    constexpr ErrorOr<Id<JsonArray>> as_array() const
    {
        ASSERT(m_type == Array);
        return m_array;
    }
    constexpr ErrorOr<StringView> as_string() const
    {
        ASSERT(m_type == String);
        return m_string;
    }

private:
    union {
        Id<JsonArray> m_array;
        Id<JsonObject> m_object;
        StringView m_string;
        bool m_bool;
        double m_number;
    };
    Type m_type;
};

struct Json {
    static ErrorOr<Json> create_from(StringView);

    constexpr JsonValue root() const { return m_root; }

    constexpr JsonObject const& operator[](Id<JsonObject> id) const
    {
        return m_objects[id];
    }

    constexpr JsonArray const& operator[](Id<JsonArray> id) const
    {
        return m_arrays[id];
    }

private:
    constexpr Json(JsonValue root, Vector<JsonObject>&& objects,
        Vector<JsonArray>&& arrays);

    JsonValue m_root;
    Vector<JsonObject> m_objects;
    Vector<JsonArray> m_arrays;
};

}

template <>
struct Ty::Formatter<Core::JsonValue::Type> {
    template <typename U>
    requires Writable<U>
    static constexpr ErrorOr<u32> write(U& to,
        Core::JsonValue::Type type)
    {
        auto type_name = [](Core::JsonValue::Type type) {
            switch (type) {
            case Core::JsonValue::Array: return "Array"sv;
            case Core::JsonValue::Bool: return "Bool"sv;
            case Core::JsonValue::Null: return "Null"sv;
            case Core::JsonValue::Number: return "Number"sv;
            case Core::JsonValue::Object: return "Object"sv;
            case Core::JsonValue::String: return "String"sv;
            }
        };
        return TRY(
            to.write("Core::JsonValue::Type::"sv, type_name(type)));
    }
};
