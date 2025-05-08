#include <any>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>
#include <optional>

/**
 * ArgParse: Enhanced command parser for command-line interfaces.
 *
 * Supports:
 * - Commands with typed arguments
 * - Flags (e.g., --verbose)
 * - Custom type converters
 * - Automatic help generation
 */
class ArgParse {
private:

    // Represents a command handler with metadata
    struct Command {
        std::function<void(std::vector<std::string>)> execute;  // Function to execute
        std::function<bool(std::vector<std::string>)> check;    // Function to check arguments
        std::string description;                               // Description of command
        size_t required_args;                                  // Number of required arguments
    };

    // All registered commands
    std::unordered_map<std::string, Command> commands;

    // Type converters: map from typeid -> string to typed converter
    std::unordered_map<std::type_index, std::function<std::any(std::string)>> converters {
        {typeid(int), [](std::string s) { return std::stoi(s); }},
        {typeid(float), [](std::string s) { return std::stof(s); }},
        {typeid(double), [](std::string s) { return std::stod(s); }},
        {typeid(std::string), [](std::string s) { return s; }}
    };

    // Flags (boolean switches)
    std::unordered_map<std::string, bool> flags;

    // Converts a string to any type using converters
    template<typename T>
    std::any convert(std::string s) {
        return converters[typeid(T)](s);
    }

    // Convert argument vector to tuple (internal helper)
    template<typename ...Args, std::size_t ...I>
    std::tuple<Args...> to_tuple(std::vector<std::string>& args, std::index_sequence<I...>) {
        return std::make_tuple(std::any_cast<Args>(convert<Args>(args[I]))...);
    }

    // Converts argument vector to typed tuple
    template<typename ...Args>
    std::tuple<Args...> parse_args(std::vector<std::string>& args) {
        return to_tuple<Args...>(args, std::make_index_sequence<sizeof...(Args)>());
    }

public:

    // Add a command with description and handler function
    template<typename ...Args>
    void add_command(const std::string& name, const std::string& desc, void(*func)(Args...)) {
    commands[name].required_args = sizeof...(Args);
    commands[name].description = desc;

    commands[name].check = [this](std::vector<std::string> args) {
        return args.size() >= sizeof...(Args);
    };

    commands[name].execute = [this, func](std::vector<std::string> args) {
        auto tuple = parse_args<Args...>(args);
        std::apply(func, tuple);
    };
}

    // Add custom type converter
    template<typename T>
    void add_conversion(std::function<T(std::string)> f) {
        converters[typeid(T)] = [f](std::string s) { return std::any(f(s)); };
    }

    // Add a boolean flag (e.g. --verbose)
    void add_flag(std::string flag_name) {
        flags[flag_name] = false;
    }

    // Parse and execute command-line arguments
    void parse(int argc, char* argv[]) {
        if (argc < 2) {
            print_help();
            return;
        }

        std::string cmd = argv[1];
        std::vector<std::string> args;

        // Collect arguments and detect flags
        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg.rfind("--", 0) == 0) {
                flags[arg] = true;
            } else {
                args.push_back(arg);
            }
        }

        // Check if command exists
        if (commands.find(cmd) == commands.end()) {
            std::cout << "Invalid command: " << cmd << std::endl;
            print_help();
            return;
        }

        auto& c = commands[cmd];
        if (!c.check(args)) {
            std::cout << "Not enough arguments!" << std::endl;
            print_help();
            return;
        }

        // Execute command
        c.execute(args);
    }

    // Check if a flag was provided
    bool has_flag(std::string flag_name) {
        return flags[flag_name];
    }

    // Print help for all commands and flags
    void print_help() {
        std::cout << "Available commands:\n";
        for (auto& [name, cmd] : commands) {
            std::cout << "  " << name << " - " << cmd.description << std::endl;
        }

        if (!flags.empty()) {
            std::cout << "Available flags:\n";
            for (auto& [name, _] : flags) {
                std::cout << "  " << std::string(name) << std::endl;
            }
        }
    }
};



// Example of custom type
struct Point {
    int x, y;
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << "(" << p.x << ", " << p.y << ")";
    }
};

void foo(int x) {
    std::cout << "foo called with " << x << std::endl;
}

void bar(std::string name, float value) {
    std::cout << "bar called with " << name << " and " << value << std::endl;
}

void show_point(Point p) {
    std::cout << "Point is " << p << std::endl;
}

int main(int argc, char* argv[]) {
    ArgParse parser;

    // Add commands
    parser.add_command<Point>("show_point", "Show Point coordinates", show_point);
    parser.add_command<std::string, float>("bar", "Test bar command", bar);
    parser.add_command<Point>("show_point", "Show Point coordinates", show_point);

    // Add custom type converter for Point
    parser.add_conversion<Point>([](std::string s) {
        Point p;
        sscanf(s.c_str(), "%d,%d", &p.x, &p.y);
        return p;
    });

    // Add flags
    parser.add_flag("--verbose");

    // Parse command-line
    parser.parse(argc, argv);

    // Check flag
    if (parser.has_flag("--verbose")) {
        std::cout << "[VERBOSE] Command executed successfully." << std::endl;
    }

    return 0;
}
