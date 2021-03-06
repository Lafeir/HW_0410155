#pragma once
#include <array>
#include <iostream>
#include <iomanip>

//ver.5

/**
 * array-based board for 2048
 *
 * index (1-d form):
 *  (0)  (1)  (2)  (3)
 *  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11)
 * (12) (13) (14) (15)
 *
 */
class board {
public:
	typedef uint32_t cell;
	typedef std::array<cell, 4> row;
	typedef std::array<row, 4> grid;
	typedef uint64_t data;
	typedef int reward;

public:
	board() : tile(), attr(0) {}
	board(const grid& b, data v = 0) : tile(b), attr(v) {}
	board(const board& b) = default;
	board& operator =(const board& b) = default;

	operator grid&() { return tile; }
	operator const grid&() const { return tile; }
	row& operator [](unsigned i) { return tile[i]; }
	const row& operator [](unsigned i) const { return tile[i]; }
	cell& operator ()(unsigned i) { return tile[i / 4][i % 4]; }
	const cell& operator ()(unsigned i) const { return tile[i / 4][i % 4]; }

	data info() const { return attr; }
	data info(data dat) { data old = attr; attr = dat; return old; }

  int pow(int base,int power)
  {
    int result = 1;
    for(int i = 0 ; i < power; i++)
    {
    result = result * base;
    }
    return result;    
  }


public:
	bool operator ==(const board& b) const { return tile == b.tile; }
	bool operator < (const board& b) const { return tile <  b.tile; }
	bool operator !=(const board& b) const { return !(*this == b); }
	bool operator > (const board& b) const { return b < *this; }
	bool operator <=(const board& b) const { return !(b < *this); }
	bool operator >=(const board& b) const { return !(*this < b); }

public:

	/**
	 * place a tile (index value) to the specific position (1-d form index)
	 * return 0 if the action is valid, or -1 if not
	 */
	reward place(unsigned pos, cell tile) {
		if (pos >= 16) return -1;
   
    // 這裡有經過修改
    // 限定生成1、2、3這三種類型的數字。
    // 之後增加獎勵格時。這邊應該要修改。
		//if (tile != 1 && tile != 2 && tile != 3) return -1;
		operator()(pos) = tile;
		return 0;
	}

	/**
	 * apply an action to the board
	 * return the reward of the action, or -1 if the action is illegal
	 */
	reward slide(unsigned opcode) {
		switch (opcode & 0b11) {
		case 0: return slide_up();
		case 1: return slide_right();
		case 2: return slide_down();
		case 3: return slide_left();
		default: return -1;
		}
	}

  void clear100()
  {
     //清除盤面上的生成記號(100)
  	for (int r = 0; r < 4; r++) 
    {
       auto& row = tile[r];
       for(int c = 0 ; c<4;c++)
       {
           int tile = row[c];
           if(tile > 20)
           {
             row[c] = 0;
           }
       }
     }
  }

	reward slide_left() {
 
    //清除盤面上的生成記號(100)(或其他特殊數字)
    clear100();
 
		board prev = *this;
		reward score = 0;
   
	  //每行
		for (int r = 0; r < 4; r++) {

			bool move = false;
      
			auto& row = tile[r];
			int top = 0, hold = 0;
      
      //每列
			for (int c = 0; c < 4; c++) {
				        
				int tile = row[c];
				row[c] = 0;
				
        //如果是第一格，直接拿取
				if (c == 0)
				{
					hold = tile;
				}
        //否則，
				else
				{		
          //如果上一格沒東西，且此格有東西，直接移動 (然後本格變空)。
					if (hold == 0 && tile != 0)
					{
						move = true;
						row[top++] = tile;
						hold = 0;
					}
          //如果此格沒東西。
					else if (tile == 0)
					{
						row[top++] = hold;
						hold = 0;
					}
          //如果此格是 1 ，
					else if (tile == 1)
					{
						if (hold == 2) //如果可以融合
						{
							move = true;
							row[top++] = 3;
							score += (3);
							hold = 0;
						}
						else //否則放下hold，拿取此格繼續往下看
						{
							row[top++] = hold;
							hold = tile;
						}
					}
          //如果此格是 2 ，
					else if (tile == 2)
					{
						if (hold == 1)//如果可以融合
						{
							move = true;
							row[top++] = 3;
							score += (3);
							hold = 0;
						}
						else //否則放下hold，拿取此格繼續往下看
						{
							row[top++] = hold;
							hold = tile;
						}
					}
          //如果下一格是 其他數字 ，
					else 
					{
						if (tile == hold) //如果相同可以融合
						{
							move = true;
							row[top++] = ++hold;
							score += pow(3, hold - 2 );
							hold = 0;
						}
						else //否則放下hold，拿取此格繼續往下看
						{
							row[top++] = hold;
							hold = tile;
						}
					}
				}	
			}
     
      //若這一行進行過移動，則在列尾打上生成記號，否則放回本處的數字 。
			if (move) 
          tile[r][top] = 100;
			else if (hold) 
          tile[r][top] = hold;
          
		}   
   
		return (*this != prev) ? score : -1;
   
	}
 
	reward slide_right() {
		reflect_horizontal();
		reward score = slide_left();
		reflect_horizontal();
		return score;
	}
	reward slide_up() {
		rotate_right();
		reward score = slide_right();
		rotate_left();
		return score;
	}
	reward slide_down() {
		rotate_right();
		reward score = slide_left();
		rotate_left();
		return score;
	}

	void transpose() {
		for (int r = 0; r < 4; r++) {
			for (int c = r + 1; c < 4; c++) {
				std::swap(tile[r][c], tile[c][r]);
			}
		}
	}

	void reflect_horizontal() {
		for (int r = 0; r < 4; r++) {
			std::swap(tile[r][0], tile[r][3]);
			std::swap(tile[r][1], tile[r][2]);
		}
	}

	void reflect_vertical() {
		for (int c = 0; c < 4; c++) {
			std::swap(tile[0][c], tile[3][c]);
			std::swap(tile[1][c], tile[2][c]);
		}
	}

	/**
	 * rotate the board clockwise by given times
	 */
	void rotate(int r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotate_right(); break;
		case 2: reverse(); break;
		case 3: rotate_left(); break;
		}
	}

	void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
	void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise
	void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
	friend std::ostream& operator <<(std::ostream& out, const board& b) {
		out << "+------------------------+" << std::endl;
		for (auto& row : b.tile) {
			out << "|" << std::dec;
			for (auto t : row) out << std::setw(6) << ((1 << t) & -2u);
			out << "|" << std::endl;
		}
		out << "+------------------------+" << std::endl;
		return out;
	}

public:
	grid tile;
	data attr;
};
