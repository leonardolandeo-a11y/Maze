#include "include/circuit_escape/types.h"



std::optional<Position> neighbor(Position origin, Action action){
    if (action == Action::up){
        if (origin.row==0){
            return std::nullopt;
        }
        return Position{origin.row-1,origin.column};
        
    }else if (action == Action::down){
        return Position{origin.row+1, origin.column};

    }else if (action == Action::left){
        if (origin.column == 0){
            return std::nullopt;
        }
        return Position{origin.row, origin.column-1};

    }else if (action == Action::right){
        return Position{origin.row, origin.column+1};

    }else if (action == Action::wait){
        return origin;
    }
    return std::nullopt;
}

std::string toString(Position position){
    return "( "+ std::to_string(position.column) + " , " + std::to_string(position.row) +" )";
}
bool operator==(const Position& lhs, const Position& rhs) {
    return lhs.row == rhs.row && lhs.column == rhs.column;
}