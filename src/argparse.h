#pragma once

#include <string>

template<typename T>
T convertStringTo(const char* s);

template<>
bool convertStringTo(const char* s) {
    return std::string(s) == "true";
}

template<typename T>
T convertStringTo(const char* s) {
    std::stringstream convert(s);
    T value;
    convert >> value;
    return value;
}

template<typename T>
T getArgValue(const char* name, int argc, const char **argv, T defaultValue) {
    std::string name1 = name;
    std::string name2 = std::string("--") + name;
    for (int i = 0; i < argc - 1; i++) {
        if (std::string(argv[i]) == name1 || std::string(argv[i]) == name2) {
            return convertStringTo<T>(argv[i + 1]);
        }
    }
    return defaultValue;
}

#define getArg(name, defaultValue) getArgValue<>(name, argc, argv, defaultValue)
