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
	weight_agent(const std::string& args = "") : random_agent(args) {
		if (meta.find("init") != meta.end()) // pass init=... to initialize the weight
			init_weights(meta["init"]);
		if (meta.find("load") != meta.end()) // pass load=... to load from a specific file
			load_weights(meta["load"]);
	}
	virtual ~weight_agent() {
		if (meta.find("save") != meta.end()) // pass save=... to save to a specific file
		{
      //init_weights("reset");
    	save_weights(meta["save"]);     
    }
      
		if (meta.find("random_train") != meta.end()) // pass save=... to save to a specific file
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

public:
	virtual void init_weights(const std::string& info) {
 
    using_net = true;
		net.emplace_back(83521); // create an empty weight table with size 65536
		net.emplace_back(83521); // create an empty weight table with size 65536
		// now net.size() == 2; net[0].size() == 65536; net[1].size() == 65536
   
   for( size_t i = 0 ; i < net[0].size() ; i++)
   {
     net[0][i] = 0;
     net[1][i] = 0;
   }
   
	}
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
	virtual void save_weights(const std::string& path) {
 
    using_net = true;
		std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		uint32_t size = net.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : net) out << w;
		out.close();
	}
 

public:
	std::vector<weight> net;
  bool random_training;
  
public:
  bool using_net = false;
};

/**
 * base agent for agents with a learning rate
 */
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
class rndenv : public random_agent {
public:
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
		space({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }), popup(0, 2) {}

	int bag[3] = { 1,2,3 };

  virtual void rebag()
	{
			bag[0] = 1;
			bag[1] = 2;
			bag[2] = 3;
	}
 
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
 
 	virtual action take_action(const board& after) {

		std::shuffle(space.begin(), space.end(), engine);

		checkbag();

    //在盤面上尋找100 (可能生成之格子)
		for (int pos : space) {
			if (after(pos) != 100) continue;		
			board::cell tile = bag[popup(engine)];		
			while (tile == 0)
			{
				tile = bag[popup(engine)];
			}
			bag[tile - 1] = 0;
			return action::place(pos, tile);
		}

		//如果整個盤面沒有100 (開局時)
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
	std::uniform_int_distribution<int> popup;
};

/**
 * dummy player
 * select a legal action randomly
 */

 size_t power (int base , int pow)
{
    size_t result = 1;
    
    for(int i = 0 ; i < pow ; i++)
    {
        result*=base;
    }
    
    return result;
}


 
 
 
 
class player : public learning_agent {
public:
	player(const std::string& args = "") : learning_agent("name=dummy role=player " + args),
		opcode({ 0, 1, 2, 3 }) , random ({ 0 }){}

  virtual double countValue(board& inBoard)
  {
    double totalValue = 0;  
    size_t  key1 = 0;
    size_t  key2 = 0;
    int oneTile1  = 0;
    int oneTile2 = 0;
    auto& tile = inBoard.tile;
    
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
        totalValue += net[0][key1];
        totalValue += net[0][key2];
        }
      else
        {
        totalValue += net[1][key1];
        totalValue += net[1][key2];
        }
    }
           
    return totalValue;
     
  }
  
  virtual float countScore(board& inBoard)
  {
    float totalScore = 0;  
     int oneTile = 0;
    auto& tile = inBoard.tile;
   
    for(int r = 0 ; r < 4 ; r++)
    {
      for (int c = 0; c < 4; c++) 
      {
        oneTile = tile[r][c];
        if( oneTile < 50 )
        {
           totalScore +=  power (3 , oneTile);
        }
        else
        {
          totalScore +=  1;
        }
      }
    }
    
    return totalScore;    
  }
  
  virtual void updateValue(board& lastBoard , double delta)
  {
     double piece =  delta/8 * alpha;
     //std::cout<<piece<<"\n";
     //if(piece < 0.0000001) return;

     size_t  key1 = 0;
     size_t  key2 = 0;
     int oneTile1  = 0;
     int oneTile2 = 0;
     auto& tile = lastBoard.tile;
   
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
          net[0][key1] += piece;
          net[0][key2] += piece;
          }
        else
         {
          net[1][key1] += piece;
          net[1][key2] += piece;
          }
      }
          
   
  }
  
  
  virtual void reset100(board& Board)
  {     auto& tile = Board.tile;
     for(int r = 0 ; r < 4 ; r++)
    {
        for (int c = 0; c < 4; c++) 
        {
           if(tile[r][c] == 100)
           tile[r][c] = 0;
        }        
    }           
  }
  
  
   virtual void showWeight()
  {
   for(int i = 0 ; i < 10000 ;i++ )
   {
     std::cout<< net[0][i]<<" "<<net[1][i]<<"\n";
   }
  }

  virtual void showBoard(board& lastBoard)
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
  
	virtual action take_action(const board& before) {
		std::shuffle(opcode.begin(), opcode.end(), engine);
   	std::shuffle(random.begin(), random.end(), engine);
    float highValue =  -1 ;
    int highOp = -1;
   
    if(  random_training == false || random[0] == 0 ||  alpha == 0 )
     {
		    for (int op : opcode) {
          board TempBoard = before;
		    	board::reward reward = TempBoard.slide(op);
          float value = countValue(TempBoard);
          
		    	if (reward != -1)
          {
            if(value >highValue)
            {  
              highValue = value;
              highOp = op;
            }     
          }
	    	}
   
        if(highOp != -1) return action::slide(highOp);
	    	return action();
     }
     else
     {
        for (int op : opcode) {
          board TempBoard = before;
		    	board::reward reward = TempBoard.slide(op);         
		    	if(reward != -1) return action::slide(op);
	    	}
          
	    	return action();
     }
	}

private:
	std::array<int, 4> opcode;
  std::array<int, 3> random;
};
