#pragma once
#include <stdexcept>
#include <iostream>
#include <vector>
#include <array>
#include <cstdint> 
#include <cassert>
#include <bit>
#include <bitset>
#include <cmath>

#define FULL_BOARD UINT64_MAX

#define RANK_1 0xffULL
#define RANK_2 0xff00ULL
#define RANK_3 0xff0000ULL
#define RANK_4 0xff000000ULL
#define RANK_5 0xff00000000ULL
#define RANK_6 0xff0000000000ULL
#define RANK_7 0xff000000000000ULL
#define RANK_8 0xff00000000000000ULL

#define FILE_A 0x8080808080808080ULL
#define FILE_B 0x4040404040404040ULL
#define FILE_C 0x2020202020202020ULL
#define FILE_D 0x1010101010101010ULL
#define FILE_E 0x0808080808080808ULL
#define FILE_F 0x0404040404040404ULL
#define FILE_G 0x0202020202020202ULL
#define FILE_H 0x0101010101010101ULL

using bitboard = unsigned long long;


const std::vector<bitboard> RANKS = {
    RANK_1, RANK_2, RANK_3, RANK_4,
    RANK_5, RANK_6, RANK_7, RANK_8
};

const std::vector<bitboard> FILES = {
    FILE_A, FILE_B, FILE_C, FILE_D,
    FILE_E, FILE_F, FILE_G, FILE_H
};

struct Gamestate {

    bitboard white;
    bitboard black;
    bitboard pawns;
    bitboard knights;
    bitboard bishops;
    bitboard rooks;
    bitboard queens;
    bitboard kings;

    bitboard en_passant_target;

    bool whiteToMove; 
    bool blackKingHasMoved;
    uint8_t moveRuleCounter; // moves since last pawn move or capture;

    bitboard& operator[](size_t index){
        assert(0 <= index && index <= 9);
        switch (index){
        case 0: return white; break;
        case 1: return black; break;
        case 2: return pawns; break;
        case 3: return knights; break;
        case 4: return bishops; break;
        case 5: return rooks; break;
        case 6: return queens; break;
        case 7: return kings; break;
        case 8: return en_passant_target; break;
        default: break;
        }
    }

    const bitboard& operator[](size_t index) const {
        assert(0 <= index && index <= 9);
        switch (index){
        case 0: return white; break;
        case 1: return black; break;
        case 2: return pawns; break;
        case 3: return knights; break;
        case 4: return bishops; break;
        case 5: return rooks; break;
        case 6: return queens; break;
        case 7: return kings; break;
        case 8: return en_passant_target; break;
        default: break;
        }
    }

    Gamestate() : 
        white(0), black(0), pawns(0), knights(0),
        bishops(0), rooks(0), queens(0), kings(0),
        en_passant_target(0), whiteToMove(1)
    {}

    Gamestate(int mode) {

        switch (mode) {
        case 0: // blank board
            white = black = pawns = knights =
            bishops = rooks = queens = kings = 
            whiteToMove = en_passant_target = 0;
            break;
        case 1: // default board
            white = 0x00'00'00'00'00'00'ff'ff;
            black = 0xff'ff'00'00'00'00'00'00;
            pawns = 0x00'ff'00'00'00'00'ff'00;
            knights = (0b01000010ULL << 56) | 0b01000010;
            bishops = (0b00100100ULL << 56) | 0b00100100;
            rooks = (0b10000001ULL << 56) | 0b10000001;
            queens = (0b00010000ULL << 56) | 0b00010000;
            kings = (0b00001000ULL << 56) | 0b00001000;
            en_passant_target = 0;
            whiteToMove = true;
            break;
        case 2: // full board (doubt i will use this)
            white = black = pawns = knights =
            bishops = rooks = queens = kings = 0xff'ff'ff'ff'ff'ff'ff'ff;
            whiteToMove = en_passant_target = 0;
            break;
        default:
            throw std::invalid_argument("Valid Arguements Are: (0, 1, 2)");
            break;
        }
    }

    friend std::ostream& operator<<(std::ostream& ostr, const Gamestate& game){

        std::vector<std::string> icons = {
            "k","q","r","b","n","p",
            "K","Q","R","B","N","P"
        };

        std::vector<std::string> board(64, " ");

        std::array<bitboard, 12> pieces;

        pieces[0] = game.white & game.pawns;
        pieces[1] = game.white & game.knights;
        pieces[2] = game.white & game.bishops;
        pieces[3] = game.white & game.rooks;
        pieces[4] = game.white & game.queens;
        pieces[5] = game.white & game.kings;

        pieces[6] = game.black & game.pawns;
        pieces[7] = game.black & game.knights;
        pieces[8] = game.black & game.bishops;
        pieces[9] = game.black & game.rooks;
        pieces[10] = game.black & game.queens;
        pieces[11] = game.black & game.kings;


        for(int i=0; i<12; i++){
            int n = 8;
            while (pieces[i] && n--){
                const int trailing = std::countr_zero(pieces[i]);
                int index = 63 - trailing;
                board[index] = icons[11-i];
                
                pieces[i] &= ~bitboard(1ULL << trailing);
            }
        }

        for(int i=0; i<64; i++){
            if (i % 8 == 0) {
                ostr << '\n';
                ostr << ' ' << 8 - (i/8) << "  |  ";
            }
            ostr << board[i] << "  ";
        }

        ostr << '\n' << "    +-------------------------\n";
        ostr << "       a  b  c  d  e  f  g  h" << std::endl;

        return ostr;
        
    }

    Gamestate& makeMove(const Move move) {
        
        

    }

    Gamestate& unmakeMove(const Move move){
        
    }


};

struct Move {
    uint16_t data; // xx from xx | xxto flag

    /*
    flag bits 
        FLAG   | promotion |  capture  | special 1 | special 0 | Move Type

        0x0          0           0           0           0       Quiet Move

        0x1          0           0           0           1       Double Pawn Push

        0x2          0           0           1           0       King Side Castle
        0x3          0           0           1           1       Queen Side Castle

        0x4          0           1           0           0       Standard Capture
        0x4          0           1           0           1       EnPass Capture

        0x8          1           0           0           0       Knight Promotion
        0x9          1           0           0           1       Bishop Promotion
        0xa          1           0           1           0       Rook Promotion
        0xb          1           0           1           1       Queen Promotion

        0x8          1           1           0           0       Knight Promo-Capture
        0x9          1           1           0           1       Bishop Promo-Capture
        0xa          1           1           1           0       Rook Promo-Capture
        0xb          1           1           1           1       Queen Promo-Capture
    */

    Move (unsigned from, unsigned to, unsigned flag) : 
        data( (flag & 0xf) | ((from & 0x3f) << 12) | ((to & 0x3f) << 6) )
    {}

    void setFrom(unsigned from)  { data &= ~0xfb'00;   data |= from << 10;   }
    void setTo(unsigned to)      { data &= ~0x03'f0;   data |= to << 8;      }
    void setFlag(unsigned flag)  { data &= ~0x00'0f;   data |= flag;         }

    uint16_t from() const { return (data & 0xfb'00) >> 10; }
    uint16_t to()   const { return (data & 0x03'f0) >> 4; }
    uint16_t flag() const { return (data & 0x00'0f); }

    bool isQuiet() const { return (data & 0b1100) == 0; } 
    bool isStrictlyQuiet() { return (data & 0b1111) == 0; } 
    bool isCapture() const { return data & 0b0100; } 
    bool isPromotion() const { return data & 0b1000; } 

};

void printBitboard(const bitboard& board){

    std::bitset<64> b(board);
    std::string s = b.to_string();
    int pad = 64 - s.length();
    std::string temp(pad, '0');
    s = temp + s;
    for(int i=0; i<64; i++){
        if (i % 8 == 0) std::cout << ' ' << 8 - (i/8) << "  |  ";
        std::cout << s[i] << "  ";
        if (i % 8 == 7) std::cout << '\n'; 
    }

    std::cout << "    +-------------------------\n";
    std::cout << "       a  b  c  d  e  f  g  h" << std::endl;

    std::cout << std::endl;
}


