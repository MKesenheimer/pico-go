#pragma once
#include <string>

namespace detail {
    template<class Integer, typename Enable = void>
    struct convert_to_number {
        Integer operator()(std::string const &str) const {
            return std::stoi(str);
        }
    };

    // special cases
    template<class Integer>
    struct convert_to_number<Integer, std::enable_if_t<std::is_same<long, Integer>::value> > {
        Integer operator()(std::string const &str) const {
            return std::stol(str);
        }
    };

    // special cases
    template<class Float>
    struct convert_to_number<Float, std::enable_if_t<std::is_same<float, Float>::value> > {
        Float operator()(std::string const &str) const {
            return std::stof(str);
        }
    };

    // special cases
    template<class Float>
    struct convert_to_number<Float, std::enable_if_t<std::is_same<double, Float>::value> > {
        Float operator()(std::string const &str) const {
            return std::stod(str);
        }
    };
}

class parser {
public:
    template<class _T, class StringLike>
    static _T to_number(StringLike&& str) {
        using type = std::decay_t<_T>;
        return detail::convert_to_number<type>()(str);
    };

    static bool contains(const std::string& str, const std::string& match) {
        size_t len = str.length();
        size_t pos = str.find(match);
        return pos >= 0 && pos < len;
    }

    static std::string parse_string(const std::string& str, const std::string& match1, const std::string& match2) {
        size_t len = str.length();
        size_t first = str.find(match1);
        first = first + match1.length();
        size_t last = str.find(match2, first);
        //std::cout << "first = " << first << ", last = " << last << std::endl;
        if (first >= 0 && first < len && last > first && last < len) {
            std::string value = str.substr(first, last - first);
            return value;
        }
        return std::string();
    }

    template<typename _T>
    static _T parse_string_cast_value(const std::string& str, const std::string& match1, const std::string& match2) {
        std::string value = parse_string(str, match1, match2);
        if (value != std::string()) {
            _T number = to_number<_T>(value);
            return number;
        }
        return 0;
    }

    static bool is_percent(const std::string& str) {
        return contains(str, "%");
    }

    static bool is_delta_minus(const std::string& str) {
        return contains(str, "-");
    }

    static bool is_delta_plus(const std::string& str) {
        return contains(str, "+");
    }

    static bool is_delta(const std::string& str) {
        return is_delta_minus(str) || is_delta_plus(str);
    }
};
