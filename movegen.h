#pragma once
#include "header.h"

// TODO add magic bitboards, legality check, castling

std::vector<Gamestate> generateMovesFromMask(const Gamestate& game, const bitboard& from, bitboard mask){

    std::vector<Gamestate> ans;
    bitboard to;
    const bitboard occupied = game.white | game.black;

    uint8_t colorIndex, pieceIndex; 

    for(uint8_t i=0; i<2; i++) if (from & game[i]) colorIndex = i;
    for(uint8_t i=2; i<8; i++) if (from & game[i]) pieceIndex = i;

    while (mask) {

        Gamestate temp = game;

        to = mask & (~mask+1); // extracts last bit

        temp[colorIndex] &= ~from;
        temp[pieceIndex] &= ~from;

        ++temp.moveRuleCounter; 

        for(uint8_t i=0; i<8; i++) {
            if (temp[i] & to) temp.moveRuleCounter = 0;
            temp[i] &= ~to;
        }

        temp[colorIndex] |= to;
        temp[pieceIndex] |= to;

        temp.whiteToMove = !temp.whiteToMove;
        temp.en_passant_target = 0;
        ans.push_back(temp);

        mask &= ~to;

    }

    return ans;

}




std::vector<Gamestate> pawnPushes(const Gamestate& game){

    std::vector<Gamestate> ans;

    const bitboard nonPromotionSquares = 0x00'ff'ff'ff'ff'ff'ff'00ULL;
    const bitboard occupied = game.black | game.white;
    bitboard pawns;
    bitboard destinations;
    bitboard mask;

    Gamestate temp;

    if (game.whiteToMove) {

        pawns = game.white & game.pawns;
        destinations = (pawns << 8) & ~occupied & nonPromotionSquares;
        
        while(destinations){
            
            mask = 0b1'0000'0001ULL << (std::countr_zero(destinations) - 8);

            temp = game;
            temp.white ^= mask;
            temp.pawns ^= mask;

            destinations &= ~mask;

            temp.moveRuleCounter = 0;
            temp.whiteToMove = !temp.whiteToMove;
            temp.en_passant_target = 0;

            ans.push_back(temp);

        }

    } else {

        pawns = game.black & game.pawns;
        destinations = (pawns >> 8) & ~occupied & nonPromotionSquares;

        while(destinations){
            
            mask = 0b1'0000'0001ULL << (std::countr_zero(destinations));

            temp = game;
            temp.black ^= mask;
            temp.pawns ^= mask;

            destinations &= ~mask;

            temp.moveRuleCounter = 0;
            temp.whiteToMove = !temp.whiteToMove;
            temp.en_passant_target = 0;
            ans.push_back(temp);

        }

    }

    return ans;
}

std::vector<Gamestate> pawnDoubles(const Gamestate& game){

    std::vector<Gamestate> ans;

    const bitboard nonPromotionSquares = 0x00'ff'ff'ff'ff'ff'ff'00ULL;
    const bitboard occupied = game.black | game.white;
    bitboard pawns;
    bitboard destinations;
    bitboard mask;
    bitboard enPassant;

    Gamestate temp;

    if (game.whiteToMove) {

        pawns = game.white & game.pawns & RANK_2;
        destinations = (pawns << 8) & ~occupied;
        destinations = (destinations << 8) & ~occupied;
        
        while(destinations){
            
            mask = 0x1'00'01ULL << (std::countr_zero(destinations) - 16);

            temp = game;
            temp.white ^= mask;
            temp.pawns ^= mask;

            temp.en_passant_target = (destinations & ~destinations + 1) >> 8;

            destinations &= ~mask;            

            temp.moveRuleCounter = 0ULL;
            temp.whiteToMove = !temp.whiteToMove;
            ans.push_back(temp);

        }

    } else {

        pawns = game.black & game.pawns & RANK_7;
        destinations = (pawns >> 8) & ~occupied;
        destinations = (destinations >> 8) & ~occupied;

        while(destinations){
            
            mask = 0x1'00'01ULL << (std::countr_zero(destinations));

            temp = game;
            temp.black ^= mask;
            temp.pawns ^= mask;

            temp.en_passant_target = (destinations & ~destinations + 1) << 8;

            destinations &= ~mask;

            temp.moveRuleCounter = 0;
            temp.whiteToMove = !temp.whiteToMove;
            ans.push_back(temp);

        }

    }

    return ans;
}

std::vector<Gamestate> pawnCaptures(const Gamestate& game){
    
    std::vector<Gamestate> ans;

    bitboard pawns;
    bitboard curr;
    bitboard fullMask;

    Gamestate temp;

    if (game.whiteToMove){

        pawns = game.white & game.pawns;

        while(pawns){

            curr = pawns & (~pawns + 1);
            fullMask = ((curr << 9) | (curr << 7)) & game.black;

            if (curr & FILE_A) fullMask &= ~FILE_H;
            else if (curr & FILE_H) fullMask &= ~FILE_A;
            
            std::vector<Gamestate> moves = generateMovesFromMask(game, curr, fullMask);
            ans.insert(ans.end(), moves.begin(), moves.end());

            pawns &= ~curr;

        }

    } else {

        pawns = game.black & game.pawns;

        while(pawns){

            curr = pawns & (~pawns + 1);
            fullMask = ((curr >> 7) | (curr >> 9)) & game.white;

            if (curr & FILE_A) fullMask &= ~FILE_H;
            else if (curr & FILE_H) fullMask &= ~FILE_A;
            
            std::vector<Gamestate> moves = generateMovesFromMask(game, curr, fullMask);
            ans.insert(ans.end(), moves.begin(), moves.end());

            pawns &= ~curr;

        }

    }

    return ans;

}

std::vector<Gamestate> enPassantMoves(const Gamestate& game){

    std::vector<Gamestate> ans;

    bitboard mask;
    bitboard curr;

    if (game.whiteToMove){

        mask = (game.en_passant_target >> 7) | (game.en_passant_target >> 9);
        mask &= game.white & game.pawns;

        while(mask){

            curr = 1ULL << std::countr_zero(mask);
            Gamestate temp = game;

            temp.pawns &= ~(game.en_passant_target >> 8);
            temp.black &= ~(game.en_passant_target >> 8);

            temp.white &= ~curr;
            temp.pawns &= ~curr;

            temp.white |= game.en_passant_target;
            temp.pawns |= game.en_passant_target;

            mask &= ~curr;

            temp.moveRuleCounter = 0;
            temp.en_passant_target = 0;
        
            ans.push_back(temp);

        }

    } else {

        mask = (game.en_passant_target << 9) | (game.en_passant_target << 7);
        mask &= game.black & game.pawns;

        while(mask){

            curr = 1ULL << std::countr_zero(mask);
            Gamestate temp = game;

            temp.pawns &= ~(game.en_passant_target << 8);
            temp.white &= ~(game.en_passant_target << 8);

            temp.black &= ~curr;
            temp.pawns &= ~curr;

            temp.black |= game.en_passant_target;
            temp.pawns |= game.en_passant_target;

            mask &= ~curr;

            temp.moveRuleCounter = 0;
            temp.en_passant_target = 0;
        
            ans.push_back(temp);

        }

    }

    return ans;

}




std::vector<Gamestate> pushPromoteQueen(const Gamestate& game){

    std::vector<Gamestate> ans;

    const bitboard promotionSquares = 0xff'00'00'00'00'00'00'ffULL;
    const bitboard occupied = game.black | game.white;
    bitboard pawns;
    bitboard destinations;
    bitboard mask;

    Gamestate temp;

    if (game.whiteToMove) {

        pawns = game.white & game.pawns;
        destinations = (pawns << 8) & ~occupied & promotionSquares;
        
        while(destinations){

            temp = game;
            
            mask = destinations & (~destinations + 1);

            temp.white &= ~(mask >> 8);
            temp.pawns &= ~(mask >> 8);

            temp.white |= mask;
            temp.queens |= mask;

            destinations &= ~mask;

            temp.moveRuleCounter = 0;
            temp.whiteToMove = !temp.whiteToMove;
            temp.en_passant_target = 0;

            ans.push_back(temp);

        }

    } else {

        pawns = game.black & game.pawns;
        destinations = (pawns >> 8) & ~occupied & promotionSquares;
        
        while(destinations){

            temp = game;
            
            mask = destinations & (~destinations + 1);

            temp.black &= ~(mask << 8);
            temp.pawns &= ~(mask << 8);

            temp.black |= mask;
            temp.queens |= mask;

            destinations &= ~mask;

            temp.moveRuleCounter = 0;
            temp.whiteToMove = !temp.whiteToMove;
            temp.en_passant_target = 0;

            ans.push_back(temp);

        }

    }

    return ans;

}

std::vector<Gamestate> pushPromoteOther(const Gamestate& game){

    std::vector<Gamestate> ans;

    const bitboard promotionSquares = 0xff'00'00'00'00'00'00'ffULL;
    const bitboard occupied = game.black | game.white;
    bitboard pawns;
    bitboard destinations;
    bitboard mask;

    Gamestate temp;

    if (game.whiteToMove) {

        pawns = game.white & game.pawns;
        destinations = (pawns << 8) & ~occupied & promotionSquares;
        
        while(destinations){
            
            mask = destinations & (~destinations + 1);

            for(unsigned char i=3; i<6; i++){

                temp = game;

                temp.white &= ~(mask >> 8);
                temp.pawns &= ~(mask >> 8);

                temp.white |= mask;
                temp[i] |= mask;

                temp.moveRuleCounter = 0;
                temp.whiteToMove = !temp.whiteToMove;
                temp.en_passant_target = 0;

                ans.push_back(temp);
            }

            destinations &= ~mask;

        }

    } else {

        pawns = game.black & game.pawns;
        destinations = (pawns >> 8) & ~occupied & promotionSquares;
        
        while(destinations){
            
            mask = destinations & (~destinations + 1);

            for(unsigned char i=3; i<6; i++){

                temp = game;

                temp.black &= ~(mask << 8);
                temp.pawns &= ~(mask << 8);

                temp.black |= mask;
                temp[i] |= mask;

                temp.moveRuleCounter = 0;
                temp.whiteToMove = !temp.whiteToMove;
                temp.en_passant_target = 0;

                ans.push_back(temp);
            }

            destinations &= ~mask;

        }

    }

    return ans;


}

std::vector<Gamestate> capturePromoteQueen(const Gamestate& game){

    std::vector<Gamestate> ans;

    const bitboard promotionSquares = 0xff'00'00'00'00'00'00'ffULL;

    bitboard pawns;
    bitboard curr;
    bitboard mask;
    bitboard to;

    Gamestate temp;

    if (game.whiteToMove){

        pawns = game.white & game.pawns;

        while(pawns){

            curr = pawns & (~pawns + 1);
            mask = ((curr << 9) | (curr << 7)) & game.black & promotionSquares;

            if (curr & FILE_A) mask &= ~FILE_H;
            else if (curr & FILE_H) mask &= ~FILE_A;

            while (mask) {

                Gamestate temp = game;

                to = mask & (~mask+1); // extracts last bit

                temp.white &= ~curr;
                temp.pawns &= ~curr;

                for(uint8_t i=0; i<8; i++) {
                    temp[i] &= ~to;
                }

                temp.white |= to;
                temp.queens |= to;

                temp.whiteToMove = !temp.whiteToMove;
                temp.en_passant_target = 0;
                ans.push_back(temp);

                mask &= ~to;

            }
            
            pawns &= ~curr;

        }

    } else {

        pawns = game.black & game.pawns;

        while(pawns){

            curr = pawns & (~pawns + 1);
            mask = ((curr >> 7) | (curr >> 9)) & game.white & promotionSquares;

            if (curr & FILE_A) mask &= ~FILE_H;
            else if (curr & FILE_H) mask &= ~FILE_A;

            while (mask) {

                Gamestate temp = game;

                to = mask & (~mask+1); // extracts last bit

                temp.black &= ~curr;
                temp.pawns &= ~curr;

                for(uint8_t i=0; i<8; i++) {
                    temp[i] &= ~to;
                }

                temp.white |= to;
                temp.queens |= to;

                temp.whiteToMove = !temp.whiteToMove;
                temp.en_passant_target = 0;
                ans.push_back(temp);

                mask &= ~to;

            }
            
            pawns &= ~curr;

        }
    
    }

    return ans;

}

std::vector<Gamestate> capturePromoteOther(const Gamestate& game){

    std::vector<Gamestate> ans;

    const bitboard promotionSquares = 0xff'00'00'00'00'00'00'ffULL;

    bitboard pawns;
    bitboard curr;
    bitboard mask;
    bitboard to;

    Gamestate temp;

    if (game.whiteToMove){

        pawns = game.white & game.pawns;

        while(pawns){

            curr = pawns & (~pawns + 1);
            mask = ((curr << 9) | (curr << 7)) & game.black & promotionSquares;

            if (curr & FILE_A) mask &= ~FILE_H;
            else if (curr & FILE_H) mask &= ~FILE_A;

            while (mask) {

                to = mask & (~mask+1); // extracts last bit

                for(uint8_t i=3; i<6; i++){

                    Gamestate temp = game;

                    temp.white &= ~curr;
                    temp.pawns &= ~curr;

                    for(uint8_t i=0; i<8; i++) {
                        temp[i] &= ~to;
                    }

                    temp.white |= to;
                    temp[i] |= to;

                    temp.whiteToMove = !temp.whiteToMove;
                    temp.en_passant_target = 0;
                    ans.push_back(temp);

                }

                mask &= ~to;

            }
            
            pawns &= ~curr;

        }

    } else {

        pawns = game.black & game.pawns;

        while(pawns){

            curr = pawns & (~pawns + 1);
            mask = ((curr >> 7) | (curr >> 9)) & game.white & promotionSquares;

            if (curr & FILE_A) mask &= ~FILE_H;
            else if (curr & FILE_H) mask &= ~FILE_A;

            while (mask) {

                to = mask & (~mask+1); // extracts last bit

                for(uint8_t i=3; i<6; i++){

                    Gamestate temp = game;

                    temp.black &= ~curr;
                    temp.pawns &= ~curr;

                    for(uint8_t i=0; i<8; i++) {
                        temp[i] &= ~to;
                    }

                    temp.black |= to;
                    temp[i] |= to;

                    temp.whiteToMove = !temp.whiteToMove;
                    temp.en_passant_target = 0;
                    ans.push_back(temp);

                }

                mask &= ~to;

            }
            
            pawns &= ~curr;

        }
    
    }

    return ans;

}




bitboard helperKnightMask(const bitboard& knight){

    assert(std::popcount(knight) == 1);

    bitboard mask = 0;
    mask |= knight << 17;
    mask |= knight << 15;
    mask |= knight << 10;
    mask |= knight << 6;
    mask |= knight >> 6;
    mask |= knight >> 10;
    mask |= knight >> 15;
    mask |= knight >> 17;
    return mask;
}

std::vector<Gamestate> knightCaptures(const Gamestate& game){

    std::vector<Gamestate> ans;

    bitboard knights;
    bitboard curr;
    bitboard fullMask;

    Gamestate temp;

    if (game.whiteToMove){

        knights = game.white & game.knights;

        while(knights){

            curr = 1ULL << std::countr_zero(knights);
            fullMask = helperKnightMask(curr) & ~game.white & game.black;

            if (curr & FILE_A) fullMask &= ~(FILE_H | FILE_G);
            else if (curr & FILE_B) fullMask &= ~FILE_H;
            else if (curr & FILE_G) fullMask &= ~(FILE_A | FILE_B);
            else if (curr & FILE_H) fullMask &= ~(FILE_A);
            
            std::vector<Gamestate> moves = generateMovesFromMask(game, curr, fullMask);
            ans.insert(ans.end(), moves.begin(), moves.end());

            knights &= ~curr;

        }

    } else {

        knights = game.black & game.knights;

        while(knights){

            curr = 1ULL << std::countr_zero(knights);
            fullMask = helperKnightMask(curr) & ~game.black & game.white;

            if (curr & FILE_A) fullMask &= ~(FILE_H | FILE_G);
            else if (curr & FILE_B) fullMask &= ~FILE_H;
            else if (curr & FILE_G) fullMask &= ~(FILE_A | FILE_B);
            else if (curr & FILE_H) fullMask &= ~(FILE_A);
            
            // check that correct piece is selected below
            std::vector<Gamestate> moves = generateMovesFromMask(game, curr, fullMask); 
            ans.insert(ans.end(), moves.begin(), moves.end());

            knights &= ~curr;

        }

    }

    return ans;

}

std::vector<Gamestate> knightMoves(const Gamestate& game){

    std::vector<Gamestate> ans;

    bitboard knights;
    bitboard curr;
    bitboard fullMask;

    Gamestate temp;

    if (game.whiteToMove){

        knights = game.white & game.knights;

        while(knights){

            curr = 1ULL << std::countr_zero(knights);
            fullMask = helperKnightMask(curr) & ~(game.white | game.black);

            if (curr & FILE_A) fullMask &= ~(FILE_H | FILE_G);
            else if (curr & FILE_B) fullMask &= ~FILE_H;
            else if (curr & FILE_G) fullMask &= ~(FILE_A | FILE_B);
            else if (curr & FILE_H) fullMask &= ~(FILE_A);
            
            std::vector<Gamestate> moves = generateMovesFromMask(game, curr, fullMask);
            ans.insert(ans.end(), moves.begin(), moves.end());

            knights &= ~curr;

        }

    } else {

        knights = game.black & game.knights;

        while(knights){

            curr = 1ULL << std::countr_zero(knights);
            fullMask = helperKnightMask(curr) & ~(game.white | game.black);

            if (curr & FILE_A) fullMask &= ~(FILE_H | FILE_G);
            else if (curr & FILE_B) fullMask &= ~FILE_H;
            else if (curr & FILE_G) fullMask &= ~(FILE_A | FILE_B);
            else if (curr & FILE_H) fullMask &= ~(FILE_A);
            
            // check that correct piece is selected below
            std::vector<Gamestate> moves = generateMovesFromMask(game, curr, fullMask); 
            ans.insert(ans.end(), moves.begin(), moves.end());

            knights &= ~curr;

        }

    }

    return ans;

}




bitboard helperKingMask(const bitboard& king){

    assert(std::popcount(king) == 1);

    bitboard mask = 0;
    mask |= king << 9;
    mask |= king << 8;
    mask |= king << 7;
    mask |= king << 1;
    mask |= king >> 1;
    mask |= king >> 7;
    mask |= king >> 8;
    mask |= king >> 9;
    return mask;
}

std::vector<Gamestate> kingCaptures(const Gamestate& game){
    
    std::vector<Gamestate> ans;

    bitboard king;
    bitboard fullMask;

    Gamestate temp;

    if (game.whiteToMove){

        king = game.white & game.kings;
        fullMask = helperKingMask(king) & game.black;

        if (king & FILE_A) fullMask &= ~FILE_H;
        else if (king & FILE_H) fullMask &= ~FILE_A;
        
        std::vector<Gamestate> moves = generateMovesFromMask(game, king, fullMask);
        ans.insert(ans.end(), moves.begin(), moves.end());

    } else {

        king = game.black & game.kings;
        fullMask = helperKingMask(king) & game.white;

        if (king & FILE_A) fullMask &= ~FILE_H;
        else if (king & FILE_H) fullMask &= ~FILE_A;
        
        std::vector<Gamestate> moves = generateMovesFromMask(game, king, fullMask);
        ans.insert(ans.end(), moves.begin(), moves.end());

    }

    return ans;

}

std::vector<Gamestate> kingMoves(const Gamestate& game){
    
    std::vector<Gamestate> ans;

    bitboard king;
    bitboard fullMask;

    Gamestate temp;

    if (game.whiteToMove){

        king = game.white & game.kings;
        fullMask = helperKingMask(king) & ~(game.white | game.black);

        if (king & FILE_A) fullMask &= ~FILE_H;
        else if (king & FILE_H) fullMask &= ~FILE_A;
        
        std::vector<Gamestate> moves = generateMovesFromMask(game, king, fullMask);
        ans.insert(ans.end(), moves.begin(), moves.end());

    } else {

        king = game.black & game.kings;
        fullMask = helperKingMask(king) & ~(game.white | game.black);

        if (king & FILE_A) fullMask &= ~FILE_H;
        else if (king & FILE_H) fullMask &= ~FILE_A;
        
        std::vector<Gamestate> moves = generateMovesFromMask(game, king, fullMask);
        ans.insert(ans.end(), moves.begin(), moves.end());

    }

    return ans;

}





bitboard helperBishopMask(const bitboard& bishop, bool collision = false){

    bitboard rank1, rank2, file1, file2;
    bitboard mask = 0;

    for(const bitboard& r : RANKS){
        if (bishop & r) {
            rank1 = rank2 = r;
            break;
        }
    }
    rank1 <<= 8; rank2 >>= 8;

    for(const bitboard& f : FILES){
        if (bishop & f) {
            file1 = file2 = f;
            break;
        }
    }
    file1 = (file1 == FILE_A) ? 0ULL : file1 << 1; 
    file2 = (file2 == FILE_H) ? 0ULL : file2 >> 1; 

    while (rank1 | rank2 | file1 | file2) {

        mask |= 
            (rank1 & file1) |
            (rank1 & file2) |
            (rank2 & file1) |
            (rank2 & file2);

        // apply modification
        rank1 <<= 8; rank2 >>= 8;
        file1 = (file1 == FILE_A) ? 0ULL : file1 << 1; 
        file2 = (file2 == FILE_H) ? 0ULL : file2 >> 1;  
    }

    if (collision) {
        mask &= ~(RANK_1 | RANK_8 | FILE_A | FILE_H);
    }

    return mask;

}

bitboard rayCastBishopMask(const bitboard& bishop, const bitboard& occupied){

    const bitboard blockers = helperBishopMask(bishop, true);
    bitboard ans = 0;
    bitboard flag;
    
    // northwest
    flag = bishop;
    while (flag) {
        ans |= flag;
        if (flag & blockers) break;
        flag = (flag & FILE_A) ? 0 :  flag << 9;
    }

    // northeast
    flag = bishop;
    while (flag) {
        ans |= flag;
        if (flag & blockers) break;
        flag = (flag & FILE_H) ? 0 : flag << 7;
    }

    // southwest
    flag = bishop;
    while (flag) {
        ans |= flag;
        if (flag & blockers) break;
        flag = (flag & FILE_A) ? 0 : flag >> 7;
    }

    // southeast
    flag = bishop;
    while (flag) {
        ans |= flag;
        if (flag & blockers) break;
        flag = (flag & FILE_H) ? 0 : flag >> 9;
    }

    return ans & ~bishop;

}




bitboard helperRookMask(const bitboard& rook, bool collision = false){

    bitboard rank, file;

    for(const bitboard& r : RANKS){
        if (rook & r) {
            rank = r;
            if (collision) rank &= ~(FILE_A | FILE_H);
            break;
        }
    }

    for(const bitboard& f : FILES){
        if (rook & f) {
            file = f;
            if (collision) file &= ~(RANK_1 | RANK_8);
            break;
        }
    }



    return (rank | file) & ~rook;

}

bitboard rayCastRookMask(const bitboard& rook, const bitboard& occupied){
 
    const bitboard blockers = helperRookMask(rook, true);
    bitboard ans = 0;
    bitboard flag;

    // north
    flag = rook;
    while (flag) {
        ans |= flag;
        if (flag & blockers) break;
        flag <<= 8;
    }

    // east
    flag = rook;
    while (flag) {
        ans |= flag;
        if (flag & blockers) break;
        flag = (flag & FILE_H) ? 0 : flag >> 1;
    }

    // south
    flag = rook;
    while (flag) {
        ans |= flag;
        if (flag & blockers) break;
        flag >>= 8;
    }

    // west
    flag = rook;
    while (flag) {
        ans |= flag;
        if (flag & blockers) break;
        flag = (flag & FILE_A) ? 0 : flag << 1;
    }

    return ans & ~rook;

}





std::vector<Gamestate> generateLegalMoves(const Gamestate& game){

    std::vector<Gamestate> ans;

    const std::vector<Gamestate> a1 = pawnPushes(game);
    const std::vector<Gamestate> a2 = pawnDoubles(game);
    const std::vector<Gamestate> a3 = knightMoves(game);
    const std::vector<Gamestate> a4 = kingMoves(game);
    const std::vector<Gamestate> a5 = pawnCaptures(game);
    const std::vector<Gamestate> a6 = enPassantMoves(game);


    ans.insert(ans.end(), a1.begin(), a1.end());
    ans.insert(ans.end(), a2.begin(), a2.end());
    ans.insert(ans.end(), a3.begin(), a3.end());
    ans.insert(ans.end(), a4.begin(), a4.end());
    ans.insert(ans.end(), a5.begin(), a5.end());
    ans.insert(ans.end(), a6.begin(), a6.end());
    // ...
    // ...

    return ans;

}


