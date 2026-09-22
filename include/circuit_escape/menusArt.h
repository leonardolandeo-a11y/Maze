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
 inline constexpr std::string_view SCENARIO_1 = R"(
┏━┓┏━╸┏━╸┏┓╻┏━┓┏━┓╻┏━┓   ╺┓
┗━┓┃  ┣╸ ┃┗┫┣━┫┣┳┛┃┃ ┃    ┃
┗━┛┗━╸┗━╸╹ ╹╹ ╹╹┗╸╹┗━┛   ╺┻╸
)";
 inline constexpr std::string_view SCENARIO_2 = R"(
┏━┓┏━╸┏━╸┏┓╻┏━┓┏━┓╻┏━┓   ┏━┓
┗━┓┃  ┣╸ ┃┗┫┣━┫┣┳┛┃┃ ┃   ┏━┛
┗━┛┗━╸┗━╸╹ ╹╹ ╹╹┗╸╹┗━┛   ┗━╸
)";
 inline constexpr std::string_view RANDOM = R"(
┏━┓┏━┓┏┓╻╺┳┓┏━┓┏┳┓
┣┳┛┣━┫┃┗┫ ┃┃┃ ┃┃┃┃
╹┗╸╹ ╹╹ ╹╺┻┛┗━┛╹ ╹
)";
 inline constexpr std::string_view EASY = R"(
┏━╸┏━┓┏━┓╻ ╻
┣╸ ┣━┫┗━┓┗┳┛
┗━╸╹ ╹┗━┛ ╹
)";
 inline constexpr std::string_view STANDARD = R"(
┏━┓╺┳╸┏━┓┏┓╻╺┳┓┏━┓┏━┓╺┳╸
┗━┓ ┃ ┣━┫┃┗┫ ┃┃┣━┫┣┳┛ ┃
┗━┛ ╹ ╹ ╹╹ ╹╺┻┛╹ ╹╹┗╸ ╹
)";
 inline constexpr std::string_view HARD = R"(
╻ ╻┏━┓┏━┓╺┳┓
┣━┫┣━┫┣┳┛ ┃┃
╹ ╹╹ ╹╹┗╸╺┻┛
)";
 inline constexpr std::string_view EMOJI = R"(
┏━╸┏┳┓┏━┓ ┏┓╻
┣╸ ┃┃┃┃ ┃  ┃┃
┗━╸╹ ╹┗━┛┗━┛╹
)";
 inline constexpr std::string_view ASCII = R"(
┏━┓┏━┓┏━╸╻╻
┣━┫┗━┓┃  ┃┃
╹ ╹┗━┛┗━╸╹╹
)";
 inline constexpr std::string_view BACK = R"(
┏┓ ┏━┓┏━╸╻┏ 
┣┻┓┣━┫┃  ┣┻┓
┗━┛╹ ╹┗━╸╹ ╹
)";
}