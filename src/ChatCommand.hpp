#pragma once
#include <string>
#include <vector>
#include <functional>

class ChatCommand {
public:
    using Callback = std::function<void()>;

    static ChatCommand* create(
        const std::string& name, // Name of the command
        const std::vector<std::string>& args, // Array of argument names in order
        int price, // Channel point cost
        Callback execute // Lambda expression to execute command
    ) {
        return new ChatCommand(name, args, price, execute);
    };

    void runExecute() {
        if (m_execute) m_execute();
    };

private:
    ChatCommand(const std::string& name,
                const std::vector<std::string>& args,
                int price,
                Callback execute)
        : m_name(name), m_args(args), m_price(price), m_execute(execute) {}

    std::string m_name;
    std::vector<std::string> m_args;
    int m_price;
    Callback m_execute;
};
