#include "header.h"
#include "movegen.h"

int main() {

    Gamestate game(1);
    std::vector<Gamestate> games;
    games = generateLegalMoves(game);

    game = games[11];
    games = generateLegalMoves(game);

    game = games[0];
    games = generateLegalMoves(game);
    
    game = games[7];
    games = generateLegalMoves(game);

    game = games[10];
    games = generateLegalMoves(game);

    for(auto& g : games) std::cout << g;

    

}
