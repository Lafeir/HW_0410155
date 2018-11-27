#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include "weight.h"
#include <fstream>

//ver.AI.0

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};


/**
 * base agent for agents with weight tables
 */
class weight_agent : public random_agent {
public:

  //開始時判斷
  	weight_agent(const std::string& args = "") : random_agent(args) {
		if (meta.find("init") != meta.end()) // pass init=... to initialize the weight
			init_weights(meta["init"]);
		if (meta.find("load") != meta.end()) // pass load=... to load from a specific file
			load_weights(meta["load"]);
      
   	if (meta.find("random_train") != meta.end()) // pass random_train=... 
			{
        if( ((std::string)meta["random_train"]) == "true")
      	  {
               random_training = true;
               std::cout<<"open random training"<<"\n";
          }
        else
           random_training = false;
      }
	}
   
   //結束時判斷
  	virtual ~weight_agent() {
		if (meta.find("save") != meta.end()) // pass save=... to save to a specific file
		{
    	save_weights(meta["save"]);     
    }
      

	}

public:

  //重置訓練網路
  //目前的數值參考區是各行各列，改變參考區時要修改此處。
	virtual void init_weights(const std::string& info) {
 
 	  // Set net.size() == 2; net[0].size() == 83521; net[1].size() == 83521
    using_net = true;
		net.emplace_back(83521); // create an empty weight table with size 83521 (17^4)
		net.emplace_back(83521); // create an empty weight table with size 83521
	
     //Set to 0
     for( size_t i = 0 ; i < net[0].size() ; i++)
     {
       net[0][i] = 0;
       net[1][i] = 0;
     }
   
	}
 
  //讀取訓練網路(太複雜了看不懂)
	virtual void load_weights(const std::string& path) {
 
    using_net = true;
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (!in.is_open()) std::exit(-1);
		uint32_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		net.resize(size);
		for (weight& w : net) in >> w;
		in.close();
	}
 
  //儲存訓練網路(太複雜了看不懂)
	virtual void save_weights(const std::string& path) {
 
		std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		uint32_t size = net.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : net) out << w;
		out.close();
	}
 
public:

  //宣告網路。
	std::vector<weight> net;
  //宣告訓練開關(決定隨機訓練或根據最好的訓練)
  bool random_training;
  //確認是否重置了網路(好在未使用任何參數時自動歸零，來防止出Bug)
  bool using_net = false;
};

/**
 * base agent for agents with a learning rate
 */
 
//繼承前者，但增加訓練曲線的設置
class learning_agent : public weight_agent {
public:
	learning_agent(const std::string& args = "") : weight_agent(args), alpha(0.1f) {
		if (meta.find("alpha") != meta.end())
			alpha = float(meta["alpha"]);
	}
	virtual ~learning_agent() {}

protected:
	float alpha = 0.0025;
};


/**
 * random environment
 * add a new random tile to an empty cell
 * 2-tile: 90%
 * 4-tile: 10%
 */
 
//敵方AGENT(會在版面上生成隨機方塊)
class rndenv : public random_agent {
public:
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
		space({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }),  //空間陣列 0~15
    popup(0, 2)  //隨機範圍 0~2
    {}

	int bag[3] = { 1,2,3 };

  //重置背包
  virtual void rebag()
	{
			bag[0] = 1;
			bag[1] = 2;
			bag[2] = 3;
	}
 
  //檢查背包空了沒
 	virtual void checkbag()
	{
		bool empty = true;
		for (int i = 0; i < 3; i++)
		{
			if (bag[i] != 0)
				empty = false;
		}
		if (empty)
		{
			rebag();
		}
	}
 
  //下棋
 	virtual action take_action(const board& after) {

		std::shuffle(space.begin(), space.end(), engine);
  
    //檢查背包空了沒。
		checkbag();

    //在盤面上尋找100 (可能生成之空格)
		for (int pos : space) {
      
      //不是100就跳過
			if (after(pos) != 100) continue;		    
      
      //隨機取一種生成結果
			board::cell tile = bag[popup(engine)];	
       //如果背包中目前沒有該數字，重新隨機
			while (tile == 0) 
			{
				tile = bag[popup(engine)];
			}      
      //決定後從背包中拿走該數字 (變成0)
			bag[tile - 1] = 0; 
      
      //回傳決定(位置,數字)
			return action::place(pos, tile); 
		}

		//如果搜尋完發現整個盤面沒有100，改搜尋0 (開局時)
		for (int pos : space) {
			if (after(pos) != 0) continue;
			board::cell tile = bag[popup(engine)];
			while (tile == 0)
			{
				tile = bag[popup(engine)];
			}
			bag[tile - 1] = 0;
			return action::place(pos, tile);
		}

		return action();
	}



private:
	std::array<int, 16> space;
	std::uniform_int_distribution<int> popup; //隨機參數，建構時會設置範圍
};




//power公式，寫在這裡純粹方便使用(X
 size_t power (int base , int pow)
{
    size_t result = 1;
    
    for(int i = 0 ; i < pow ; i++)
    {
        result*=base;
    }
    
    return result;
}


//我方AGENT(會在版面上決定移動方向)
class player : public learning_agent {
public:
	player(const std::string& args = "") : learning_agent("name=dummy role=player " + args),
		opcode({ 0, 1, 2, 3 }) , random (0, 2){}
  
  
  //計算盤面"分數"(需輸入當前盤面)
  virtual float countScore(const board& inBoard)
  {
    float totalScore = 0;  
    int oneTile = 0;
    auto& tile = inBoard.tile;
   
    for(int r = 0 ; r < 4 ; r++)
    {
      for (int c = 0; c < 4; c++) 
      {
        oneTile = tile[r][c];
        if( oneTile < 50) //如果不是100(待生成格)，增加分數。
        {
           totalScore +=  power (3 , oneTile);
        }
        else //空格姑且算1分。
        {
           totalScore +=  1;
        }
      }
    }    
    return totalScore;    
  }
  
  
  //計算盤面"數值"(需輸入當前盤面)
  //目前的數值參考區是各行各列，改變參考區時要修改此處。
  virtual double countValue(const board& inBoard)
  {
    double totalValue = 0; //總值
    size_t  key1 = 0 , key2 = 0; //Key值 (用來查詢Table)
    int oneTile1  = 0 , oneTile2 = 0;
    auto& tile = inBoard.tile;
    
    //從各行(key1)列(key2)的頭讀到尾，計算總值
    for(int r = 0 ; r < 4 ; r++)
    {
      key1 = 0;   
      key2 = 0;
      for (int c = 0; c < 4; c++) 
      {
	      oneTile1 = tile[r][c]; //讀取行中每一格的數字
        oneTile2 = tile[c][r]; //讀取列中每一格的數字
        if(oneTile1 == 100 )oneTile1 = 16; //如果是100，設置代號為16(待生成格)
        if(oneTile2 == 100 )oneTile2 = 16;
        key1 += oneTile1 * power(17,c);    //Key增加數字*17^位數
        key2 += oneTile2 * power(17,c);  
      }
      
      if(r == 0 || r == 3) //邊界的Table為net[0]
        {
        totalValue += net[0][key1];
        totalValue += net[0][key2];
        }
      else if(r == 1 || r == 2) //中間的Table為net[1]
        {
        totalValue += net[1][key1];
        totalValue += net[1][key2];
        }
    }
           
    return totalValue; //回傳總值   
  }
  

  //更新"數值"表(需輸入當前盤面,差值)
  //目前的數值參考區是各行各列，改變參考區時要修改此處。
  virtual void updateValue(const board& lastBoard , double delta)
  {
     //所有參考列增加： 差值 * 更新幅度 / 總參考列數 
     float CheckNum = 8;
     double piece =  delta/CheckNum * alpha;

     size_t  key1 = 0 , key2 = 0;
     int oneTile1  =0,oneTile2= 0;
     auto& tile = lastBoard.tile;

    //請參考上方讀取處，沒差很多　
    for(int r = 0 ; r < 4 ; r++)
    {
        key1 = 0;   
        key2 = 0;  
        for (int c = 0; c < 4; c++) 
        {    
         oneTile1 = tile[r][c];
         oneTile2 = tile[c][r];
         if(oneTile1 == 100 )oneTile1 = 16;
         if(oneTile2 == 100 )oneTile2 = 16;
         key1 += oneTile1 * power(17,c);  
         key2 += oneTile2 * power(17,c);  
        }
      
        if(r == 0 || r == 3)
          {
              net[0][key1] += piece; //更新平均差值(無論是正是負)
              net[0][key2] += piece;
          }
        else
         {
              net[1][key1] += piece;
              net[1][key2] += piece;
          }
      }
           
  }
  
  //清除盤面上的100 (或其他特殊數字)
  virtual void reset100(board& Board) //這裡版面要變動，不用const保護
  {     
    auto& tile = Board.tile;
    
    for(int r = 0 ; r < 4 ; r++)
    {
        for (int c = 0; c < 4; c++) 
        {
           if(tile[r][c] > 20)
               tile[r][c] = 0;
        }        
    }           
  }  

  virtual void showBoard(const board& lastBoard) //輸出顯示盤面
  {  
   auto& tile = lastBoard.tile;
   
   for(int r= 0 ; r < 4 ;r++)
   {
     for(int c = 0 ; c < 4 ; c++)
     {
       std::cout<<tile[r][c] << "  ";
     }
      std::cout<<"\n";
   }
    std::cout<<"\n"<<"-------------------"<<"\n";  
  }
  
  //進行行動
	virtual action take_action(const board& before) {
		
    std::shuffle(opcode.begin(), opcode.end(), engine);
    
    float highValue =  -1 ;
    int highOp = -1;
   
    if( random_training == false ||  alpha == 0 ) //如果不使用隨機訓練，或是不需要訓練(訓練幅度=0)
     {
		    
        for (int op : opcode) {
          board TempBoard = before;                   //使用初始版面(傳入的版面)
		    	board::reward reward = TempBoard.slide(op); //紀錄往op方向滑完後的版面
          float value = countValue(TempBoard);        //紀錄滑完後的網路數值
          
		    	if (reward != -1) //如果該方向可滑動
          {
            if( value > highValue) //尋找數值最高的方向，記錄下來
            {  
              highValue = value;
              highOp = op;
            }     
          }
	    	}
   
        if(highOp != -1) //如果至少有一方向可動，使用該數值。
            return action::slide(highOp);
        else             //所有方向都不可動，GG。     
	    	    return action();
     }
     else //如果使用隨機訓練
     { 
        for (int op : opcode) //隨機挑一個可滑的方向使用
        {
          board TempBoard = before; 
		    	board::reward reward = TempBoard.slide(op);         
		    	if(reward != -1) return action::slide(op);
	    	}
          
	    	return action();
     }
	}

private:
	std::array<int, 4> opcode;
	std::uniform_int_distribution<int> random; //隨機參數，建構時會設置範圍
};
