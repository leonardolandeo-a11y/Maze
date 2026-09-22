#pragma once

#include <string_view>

namespace MenuArt {

    inline constexpr std::string_view MAZE_LOGO = R"(
███╗   ███╗ █████╗ ███████╗███████╗
████╗ ████║██╔══██╗╚══███╔╝██╔════╝
██╔████╔██║███████║  ███╔╝ █████╗
██║╚██╔╝██║██╔══██║ ███╔╝  ██╔══╝
██║ ╚═╝ ██║██║  ██║███████╗███████╗
╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝                                   ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
)";

 inline constexpr std::string_view START_GAME = R"(
┏━┓╺┳╸┏━┓┏━┓╺┳╸   ┏━╸┏━┓┏┳┓┏━╸
┗━┓ ┃ ┣━┫┣┳┛ ┃    ┃╺┓┣━┫┃┃┃┣╸
┗━┛ ╹ ╹ ╹╹┗╸ ╹    ┗━┛╹ ╹╹ ╹┗━╸
)";
 inline constexpr std::string_view SCENARIO = R"(
┏━┓┏━╸┏━╸┏┓╻┏━┓┏━┓╻┏━┓
┗━┓┃  ┣╸ ┃┗┫┣━┫┣┳┛┃┃ ┃
┗━┛┗━╸┗━╸╹ ╹╹ ╹╹┗╸╹┗━┛
)";
 inline constexpr std::string_view DIFFICULTY = R"(
╺┳┓╻┏━╸┏━╸╻┏━╸╻ ╻╻  ╺┳╸╻ ╻
 ┃┃┃┣╸ ┣╸ ┃┃  ┃ ┃┃   ┃ ┗┳┛
╺┻┛╹╹  ╹  ╹┗━╸┗━┛┗━╸ ╹  ╹
)";
 inline constexpr std::string_view RENDER_MODE = R"(
┏━┓┏━╸┏┓╻╺┳┓┏━╸┏━┓   ┏┳┓┏━┓╺┳┓┏━╸
┣┳┛┣╸ ┃┗┫ ┃┃┣╸ ┣┳┛   ┃┃┃┃ ┃ ┃┃┣╸
╹┗╸┗━╸╹ ╹╺┻┛┗━╸╹┗╸   ╹ ╹┗━┛╺┻┛┗━╸
)";
 inline constexpr std::string_view HELP = R"(
╻ ╻┏━╸╻  ┏━┓
┣━┫┣╸ ┃  ┣━┛
╹ ╹┗━╸┗━╸╹
)";
 inline constexpr std::string_view QUIT = R"(
┏━┓╻ ╻╻╺┳╸
┃┓┃┃ ┃┃ ┃
┗┻┛┗━┛╹ ╹
)";
}