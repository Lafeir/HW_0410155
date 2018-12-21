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

//ver.5

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

  //設置設置權重表對照用的bitBoard公式(Ver.4)
   virtual void transpose(int inBoard[16]) {
		for (int r = 0; r < 4; r++) {
			for (int c = r + 1; c < 4; c++) {
				std::swap(inBoard[4 * r + c], inBoard[ 4 * c + r]);
			}
		}
	}

	virtual void reflect_horizontal(int inBoard[16]) {
		for (int r = 0; r < 4; r++) {
			std::swap(inBoard[ 4 * r + 0], inBoard[ 4 * r + 3]);
			std::swap(inBoard[ 4 * r + 1], inBoard[ 4 * r + 2]);
		}
	}

	virtual void reflect_vertical(int inBoard[16]) {
		for (int c = 0; c < 4; c++) {
			std::swap(inBoard[ 4 * 0 + c], inBoard[ 4 * 3 + c]);
			std::swap(inBoard[ 4 * 1 + c], inBoard[ 4 * 2 + c]);
		}
	}
 
 	virtual void rotate_right(int inBoard[16]) { transpose(inBoard); reflect_horizontal(inBoard); } // clockwise
	virtual void rotate_left(int inBoard[16]) { transpose(inBoard); reflect_vertical(inBoard); } // counterclockwise
	virtual void reverse(int inBoard[16]) { reflect_horizontal(inBoard); reflect_vertical(inBoard); }
 
 
   virtual void init_bitboard() //設置權重表對照用的bitBoard(Ver.4)
   {
      bitBoard.clear();
      
      int oneBoardA[16] = 
       { 4, 5, 0, 0,
         3, 6, 0, 0,     
         2, 0, 0, 0, 
         1, 0, 0, 0 };
         
      int oneBoardB[16] = 
       { 0, 4, 5, 0,
         0, 3, 6, 0,     
         0, 2, 0, 0, 
         0, 1, 0, 0 };
         
       int oneBoardC[16] = 
       { 0, 0, 0, 0,
         3, 4, 0, 0,     
         2, 5, 0, 0, 
         1, 6, 0, 0 };
         
       int oneBoardD[16] = 
       { 0, 0, 0, 0,
         0, 3, 4, 0,     
         0, 2, 5, 0, 
         0, 1, 6, 0 };
       
      //bitBoardA
      for(int mirror = 0 ; mirror < 2 ; mirror++)  
      {
        for(int dir = 0 ; dir< 4 ; dir ++)
        {
          std::vector<int> temp;
          for(int i = 0 ; i < 16 ; i++)
          {
            temp.emplace_back(oneBoardA[i]);
          }
          bitBoard.emplace_back(temp);  
                
          rotate_right(oneBoardA);
        }
        reflect_vertical(oneBoardA);
      }
    
      //bitBoardB
      for(int mirror = 0 ; mirror < 2 ; mirror++)  
      {
        for(int dir = 0 ; dir< 4 ; dir ++)
        {
          std::vector<int> temp;
          for(int i = 0 ; i < 16 ; i++)
          {
            temp.emplace_back(oneBoardB[i]);
          }
          bitBoard.emplace_back(temp);  
                
          rotate_right(oneBoardB);
        }
        reflect_vertical(oneBoardB);
      }
      
      //bitBoardC     
      for(int mirror = 0 ; mirror < 2 ; mirror++)  
      {
        for(int dir = 0 ; dir< 4 ; dir ++)
        {
          std::vector<int> temp;
          for(int i = 0 ; i < 16 ; i++)
          {
            temp.emplace_back(oneBoardC[i]);
          }
          bitBoard.emplace_back(temp);  
                
          rotate_right(oneBoardC);
        }
        reflect_vertical(oneBoardC);
      }

      //bitBoardD    
      for(int mirror = 0 ; mirror < 2 ; mirror++)  
      {
        for(int dir = 0 ; dir< 4 ; dir ++)
        {
          std::vector<int> temp;
          for(int i = 0 ; i < 16 ; i++)
          {
            temp.emplace_back(oneBoardD[i]);
          }
          bitBoard.emplace_back(temp);  
                
          rotate_right(oneBoardD);
        }
        reflect_vertical(oneBoardD);
      }
      
       /*
      std::cout<<std::endl<<"bitBoard Setting Finish"<<std::endl;
            
      for(int i = 0 ; i<32 ;i++)
      {
        for(int j = 0 ; j < 16 ; j++)
        {
          std::cout << bitBoard[i][j];
        }
          std::cout << std::endl;
      }
      */
   }


  //重置訓練網路
  //目前的數值參考區是各行各列，改變參考區時要修改此處。
	virtual void init_weights(const std::string& info) {
 
 	  // Set net.size() == 4
    using_net = true;
		net.emplace_back(72412707); // create an empty weight table with size 72412707 (17^6 * 3) //(Ver.4)
		net.emplace_back(72412707); 
		net.emplace_back(72412707); 
		net.emplace_back(72412707); 
	
     //Set to 0
     for( size_t i = 0 ; i < net[0].size() ; i++)
     {
       net[0][i] = 0;
       net[1][i] = 0;
       net[2][i] = 0;
       net[3][i] = 0;
       
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
  
     //宣告bitBoard。
	std::vector< std::vector<int> > bitBoard;
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
    popup(0, 11),  //隨機範圍 0~2
    random_21 (0, 20)
    {}

	int bag[12] = { 1,1,1,1, 2,2,2,2, 3,3,3,3 };

  //重置背包
  virtual void rebag()
	{
		 for(int i = 0 ; i < 3 ; i++)
     {
       for(int j = 0 ; j < 4 ; j++)
       {
          bag[ (i * 4) + j] = i + 1;
       } 
     }
	}
 
  //檢查背包空了沒
 	virtual void checkbag()
	{
		bool empty = true;
		for (int i = 0; i < 12; i++)
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

    //找到盤面最大值。
    unsigned int max = 0;  
    for(int pos : space)
    {
        if( after(pos) < 50 && after(pos) > max )
        {
            max =  after(pos);       
        }    
    }
    if(max > 6 && RandomCoolDown==100)
    {
        RandomCoolDown = 0;
    }

    //在盤面上尋找100 (可能生成之空格)
		for (int pos : space) {
     
      //不是100就跳過
			if (after(pos) != 100) continue;		 
       
      board::cell tile;
      
        if(  random_21(engine) == 15 && RandomCoolDown == 0 && max >  6 )
        {
            RandomCoolDown = 20;
          
            max = max - 7;
          
            unsigned int temp_tile = random_21(engine);
            while( temp_tile > max)
            {
               temp_tile = random_21(engine);
            }
          
            tile = ( 4 + temp_tile );                
        }
        else
        {        
            //如果之前有生成過bouns格，還在冷卻中，減少冷卻時間。(Ver.4)
             if(RandomCoolDown > 0 )
            {
               RandomCoolDown -= 1 ;
            }
          
            //隨機取一種生成結果
            int randomNum = popup(engine);
		  	    tile = bag[randomNum];	
             //如果背包中目前沒有該數字，重新隨機
		      	while (tile == 0) 
		      	{
              randomNum = popup(engine);
		  	    	tile = bag[randomNum];
		      	}      
             //決定後從背包中拿走該數字 (變成0)
		      	bag[randomNum] = 0; 
      
            //回傳決定(位置,數字)
	      		return action::place(pos, tile); 
  	  	}
    }
    
		//如果搜尋完發現整個盤面沒有100，改搜尋0 (開局時)
		for (int pos : space) {
			if (after(pos) != 0) continue;
      
	    int randomNum = popup(engine);
			board::cell tile = bag[randomNum];	
			while (tile == 0)
			{
        randomNum = popup(engine);
				tile = bag[randomNum];
			}
			bag[randomNum] = 0;
      
			return action::place(pos, tile);
		}

		return action();
	}



private:
	std::array<int, 16> space;
	std::uniform_int_distribution<int> popup; //隨機參數，建構時會設置範圍
 	std::uniform_int_distribution<int> random_21;

public :
  int RandomCoolDown;

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
  virtual double countValue(const board& inBoard ,const int& RandomCoolDown = 100)
  {
    double totalValue = 0; //總值
    size_t  key = 0; //Key值 (用來查詢Table)
    int oneTile  = 0;
    auto& tile = inBoard.tile;
    
    int BounsSituation = 0;
    
    //if(RandomCoolDown > 5) {BounsSituation = 2;}
    //else if(RandomCoolDown > 0) {BounsSituation = 1;}
    //else { BounsSituation = 0;}    
    
    //讀取4種共8個方向的bitBoard，計算總值
    int bitNum = 0;
    for(int type = 0 ; type < 4 ; type++)
    {
      for(int no = 0 ; no < 8 ; no++)
      { 
          //從版面根據bitBoard獲得key
          key = 0;
          
          for(int i = 0 ; i < 16 ; i++)
          {
                      
            bitNum = bitBoard[type*8 + no][i];
            
            if( bitNum != 0)
            {
                oneTile = tile[i/4][i%4];
                if(oneTile > 50 )oneTile = 16; //如果是100，設置代號為16(待生成格)
                key += oneTile *  power(17, bitNum - 1);  //Key增加bit數字*17^位數
            }
          }         
          
          key += BounsSituation *  power(17, 6);
          
          //根據key從權重表中獲取數值。              
          totalValue += net[type][key];
      }
    } 
        
    return totalValue; //回傳總值    
  }
  

  //更新"數值"表(需輸入當前盤面,差值)
  //目前的數值參考區是各行各列，改變參考區時要修改此處。
  virtual void updateValue(const board& lastBoard ,const double& delta , const int& RandomCoolDown = 100)
  {
      
      //所有參考列增加： 差值 * 更新幅度 / 總參考列數 
     float CheckNum = 32;
     double piece =  delta/CheckNum * alpha;

     size_t  key = 0 ;
     size_t  keyA = 0 ;
     size_t  keyB = 0 ;
     size_t  keyC = 0 ;
     
     int oneTile  =0;
     auto& tile = lastBoard.tile;
    
    int BounsSituation = 0;
    //if(RandomCoolDown > 3) {BounsSituation = 2;}
    //else if(RandomCoolDown > 0) {BounsSituation = 1;}
    //else { BounsSituation = 0;}

    //讀取4種共8個方向的bitBoard，計算總值
    int bitNum = 0;
    for(int type = 0 ; type < 4 ; type++)
    {
      for(int no = 0 ; no < 8 ; no++)
      {
          //從版面根據bitBoard獲得key
          key = 0;
          
          for(int i = 0 ; i < 16 ; i++)
          {
            bitNum = bitBoard[type*8 + no][i];
            if( bitNum != 0)
            {
                oneTile = tile[i/4][i%4];
                if(oneTile > 50 )oneTile = 16; //如果是100，設置代號為16(待生成格)
                key += oneTile *  power(17, bitNum - 1);  //Key增加bit數字*17^位數
            }
          }
         
          //更新平均差值(無論是正是負)
          //key += BounsSituation *  power(17, 6);
          //net[type][keyC] += piece;
          
          keyA = key;
          keyB = key;
          keyC = key;
          
          keyA += 2 *  power(17, 6);
          keyB += 1 *  power(17, 6);
          keyC += 0 *  power(17, 6);
          

          if(BounsSituation >= 2)
          {
            net[type][keyA] += piece;
          }
          if(BounsSituation >= 1)
          {
            net[type][keyB] += piece;
          }
          if(BounsSituation >= 0)
          {
            net[type][keyC] += piece;
          }
          
          
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
