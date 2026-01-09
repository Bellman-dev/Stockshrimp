#pragma once
#include "header.h"
#include "movegen.h"

int evaluate(const Gamestate& game){

    bitboard pieces = game.knights | game.bishops | game.rooks | game.queens;
    int pieceCount = std::popcount(pieces);

    int eval = (pieceCount > 7) ? middleGameEval(game) : endGameEval(game);

    return eval + pawnEval(game);

}

int naiveEval(const Gamestate& game){

    int eval = 0;

    eval += 100 * std::popcount(game.white | game.pawns);
    eval -= 100 * std::popcount(game.black | game.pawns);

    eval += 300 * std::popcount(game.white | game.knights);
    eval -= 300 * std::popcount(game.black | game.knights);

    eval += 330 * std::popcount(game.white | game.bishops);
    eval -= 330 * std::popcount(game.black | game.bishops);

    eval += 500 * std::popcount(game.white | game.rooks);
    eval -= 500 * std::popcount(game.black | game.rooks);

    eval += 900 * std::popcount(game.white | game.queens);
    eval -= 900 * std::popcount(game.black | game.queens);

    return eval;

}

int pawnEval(const Gamestate& game){

    int eval = 0;

    eval += 5 * std::popcount(game.white & game.pawns & RANK_3);
    eval += 10 * std::popcount(game.white & game.pawns & RANK_4);
    eval += 20 * std::popcount(game.white & game.pawns & RANK_5);
    eval += 40 * std::popcount(game.white & game.pawns & RANK_6);
    eval += 180 * std::popcount(game.white & game.pawns & RANK_7);

    eval -= 5 * std::popcount(game.black & game.pawns & RANK_6);
    eval -= 10 * std::popcount(game.black & game.pawns & RANK_5);
    eval -= 20 * std::popcount(game.black & game.pawns & RANK_4);
    eval -= 40 * std::popcount(game.black & game.pawns & RANK_3);
    eval -= 180 * std::popcount(game.black & game.pawns & RANK_2);

    return eval;

}

int middleGameEval(const Gamestate& game){

    int eval = naiveEval(game);

    const bitboard mask1 = RANK_1 | RANK_8 | FILE_A | FILE_H;
    const bitboard mask2 = (RANK_2 | RANK_7 | FILE_B | FILE_G) & ~mask1;
    const bitboard mask3 = (RANK_3 | RANK_6 | FILE_C | FILE_F) & ~mask1 & ~mask2;
    const bitboard mask4 = (RANK_4 | RANK_5 | FILE_D | FILE_E) & ~mask1 & ~mask2 & ~mask3;



    eval += -10 * std::popcount(game.white & mask1 & ~game.pawns & ~game.kings);
    eval += 5 * std::popcount(game.white & mask2 & ~game.pawns & ~game.kings);
    eval += 20 * std::popcount(game.white & mask3 & ~game.pawns & ~game.kings);
    eval += 35 * std::popcount(game.white & mask4 & ~game.pawns & ~game.kings);

    eval -= -10 * std::popcount(game.black & mask1 & ~game.pawns & ~game.kings);
    eval -= 5 * std::popcount(game.black & mask2 & ~game.pawns & ~game.kings);
    eval -= 20 * std::popcount(game.black & mask3 & ~game.pawns & ~game.kings);
    eval -= 35 * std::popcount(game.black & mask4 & ~game.pawns & ~game.kings);



    eval -= 20 * std::popcount(game.white & game.kings & RANK_3);
    eval -= 40 * std::popcount(game.white & game.kings & RANK_4);
    eval -= 60 * std::popcount(game.white & game.kings & RANK_5);
    eval -= 100 * std::popcount(game.white & game.kings & RANK_6);
    eval -= 200 * std::popcount(game.white & game.kings & RANK_7);

    eval += 20 * std::popcount(game.black & game.kings & RANK_6);
    eval += 40 * std::popcount(game.black & game.kings & RANK_5);
    eval += 60 * std::popcount(game.black & game.kings & RANK_4);
    eval += 100 * std::popcount(game.black & game.kings & RANK_3);
    eval += 200 * std::popcount(game.black & game.kings & RANK_2);

    
    return eval;

}

int endGameEval(const Gamestate& game){

    int eval = naiveEval(game);

    eval += 40 * std::popcount(game.white & (game.rooks | game.queens) & RANK_7);
    eval -= 40 * std::popcount(game.black & (game.rooks | game.queens) & RANK_2);

    return eval;

}


// TODO
int checkmate(const Gamestate& game){

    // if checkmate for white return 1'000'000'000
    // if checkmate for black return -1'000'000'000

    return 0;
}

// TODO update for more pruning
int minimax(const Gamestate& game, bool maxxer, int alpha, int beta, int depth){

    if (depth == 0) return evaluate(game);
    if (checkmate(game)) return checkmate(game);
    if (game.moveRuleCounter == 100) return 0;

    std::vector<Gamestate> children = generateLegalMoves(game);

    // if no legal moves and not checkmate then stalemate result is draw
    if (children.size() == 0) return 0;

    int best, curr;

    if (maxxer) {
        best = INT_MIN;
        for(const Gamestate& child : children) {
            curr = minimax(child, false, alpha, beta, depth-1);
            best = std::max(best, curr);
            alpha = std::max(best, alpha);
            if (beta <= alpha) break;
        }
    } else {
        best = INT_MAX;
        for(const Gamestate& child : children) {
            curr = minimax(child, true, alpha, beta, depth-1);
            best = std::min(best, curr);
            beta = std::min(best, beta);
            if (beta <= alpha) break;
        }
    }

    return best;

}