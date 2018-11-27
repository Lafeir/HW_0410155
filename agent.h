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

  //�}�l�ɧP�_
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
   
   //�����ɧP�_
  	virtual ~weight_agent() {
		if (meta.find("save") != meta.end()) // pass save=... to save to a specific file
		{
    	save_weights(meta["save"]);     
    }
      

	}

public:

  //���m�V�m����
  //�ثe���ƭȰѦҰϬO�U��U�C�A���ܰѦҰϮɭn�ק惡�B�C
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
 
  //Ū���V�m����(�ӽ����F�ݤ���)
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
 
  //�x�s�V�m����(�ӽ����F�ݤ���)
	virtual void save_weights(const std::string& path) {
 
		std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		uint32_t size = net.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : net) out << w;
		out.close();
	}
 
public:

  //�ŧi�����C
	std::vector<weight> net;
  //�ŧi�V�m�}��(�M�w�H���V�m�ήھڳ̦n���V�m)
  bool random_training;
  //�T�{�O�_���m�F����(�n�b���ϥΥ���ѼƮɦ۰��k�s�A�Ө���XBug)
  bool using_net = false;
};

/**
 * base agent for agents with a learning rate
 */
 
//�~�ӫe�̡A���W�[�V�m���u���]�m
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
 
//�Ĥ�AGENT(�|�b�����W�ͦ��H�����)
class rndenv : public random_agent {
public:
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
		space({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }),  //�Ŷ��}�C 0~15
    popup(0, 2)  //�H���d�� 0~2
    {}

	int bag[3] = { 1,2,3 };

  //���m�I�]
  virtual void rebag()
	{
			bag[0] = 1;
			bag[1] = 2;
			bag[2] = 3;
	}
 
  //�ˬd�I�]�ŤF�S
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
 
  //�U��
 	virtual action take_action(const board& after) {

		std::shuffle(space.begin(), space.end(), engine);
  
    //�ˬd�I�]�ŤF�S�C
		checkbag();

    //�b�L���W�M��100 (�i��ͦ����Ů�)
		for (int pos : space) {
      
      //���O100�N���L
			if (after(pos) != 100) continue;		    
      
      //�H�����@�إͦ����G
			board::cell tile = bag[popup(engine)];	
       //�p�G�I�]���ثe�S���ӼƦr�A���s�H��
			while (tile == 0) 
			{
				tile = bag[popup(engine)];
			}      
      //�M�w��q�I�]�������ӼƦr (�ܦ�0)
			bag[tile - 1] = 0; 
      
      //�^�ǨM�w(��m,�Ʀr)
			return action::place(pos, tile); 
		}

		//�p�G�j�M���o�{��ӽL���S��100�A��j�M0 (�}����)
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
	std::uniform_int_distribution<int> popup; //�H���ѼơA�غc�ɷ|�]�m�d��
};




//power�����A�g�b�o�̯º��K�ϥ�(X
 size_t power (int base , int pow)
{
    size_t result = 1;
    
    for(int i = 0 ; i < pow ; i++)
    {
        result*=base;
    }
    
    return result;
}


//�ڤ�AGENT(�|�b�����W�M�w���ʤ�V)
class player : public learning_agent {
public:
	player(const std::string& args = "") : learning_agent("name=dummy role=player " + args),
		opcode({ 0, 1, 2, 3 }) , random (0, 2){}
  
  
  //�p��L��"����"(�ݿ�J��e�L��)
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
        if( oneTile < 50) //�p�G���O100(�ݥͦ���)�A�W�[���ơC
        {
           totalScore +=  power (3 , oneTile);
        }
        else //�Ů�h�B��1���C
        {
           totalScore +=  1;
        }
      }
    }    
    return totalScore;    
  }
  
  
  //�p��L��"�ƭ�"(�ݿ�J��e�L��)
  //�ثe���ƭȰѦҰϬO�U��U�C�A���ܰѦҰϮɭn�ק惡�B�C
  virtual double countValue(const board& inBoard)
  {
    double totalValue = 0; //�`��
    size_t  key1 = 0 , key2 = 0; //Key�� (�ΨӬd��Table)
    int oneTile1  = 0 , oneTile2 = 0;
    auto& tile = inBoard.tile;
    
    //�q�U��(key1)�C(key2)���YŪ����A�p���`��
    for(int r = 0 ; r < 4 ; r++)
    {
      key1 = 0;   
      key2 = 0;
      for (int c = 0; c < 4; c++) 
      {
	      oneTile1 = tile[r][c]; //Ū���椤�C�@�檺�Ʀr
        oneTile2 = tile[c][r]; //Ū���C���C�@�檺�Ʀr
        if(oneTile1 == 100 )oneTile1 = 16; //�p�G�O100�A�]�m�N����16(�ݥͦ���)
        if(oneTile2 == 100 )oneTile2 = 16;
        key1 += oneTile1 * power(17,c);    //Key�W�[�Ʀr*17^���
        key2 += oneTile2 * power(17,c);  
      }
      
      if(r == 0 || r == 3) //��ɪ�Table��net[0]
        {
        totalValue += net[0][key1];
        totalValue += net[0][key2];
        }
      else if(r == 1 || r == 2) //������Table��net[1]
        {
        totalValue += net[1][key1];
        totalValue += net[1][key2];
        }
    }
           
    return totalValue; //�^���`��   
  }
  

  //��s"�ƭ�"��(�ݿ�J��e�L��,�t��)
  //�ثe���ƭȰѦҰϬO�U��U�C�A���ܰѦҰϮɭn�ק惡�B�C
  virtual void updateValue(const board& lastBoard , double delta)
  {
     //�Ҧ��ѦҦC�W�[�G �t�� * ��s�T�� / �`�ѦҦC�� 
     float CheckNum = 8;
     double piece =  delta/CheckNum * alpha;

     size_t  key1 = 0 , key2 = 0;
     int oneTile1  =0,oneTile2= 0;
     auto& tile = lastBoard.tile;

    //�аѦҤW��Ū���B�A�S�t�ܦh�@
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
              net[0][key1] += piece; //��s�����t��(�L�׬O���O�t)
              net[0][key2] += piece;
          }
        else
         {
              net[1][key1] += piece;
              net[1][key2] += piece;
          }
      }
           
  }
  
  //�M���L���W��100 (�Ψ�L�S��Ʀr)
  virtual void reset100(board& Board) //�o�̪����n�ܰʡA����const�O�@
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

  virtual void showBoard(const board& lastBoard) //��X��ܽL��
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
  
  //�i����
	virtual action take_action(const board& before) {
		
    std::shuffle(opcode.begin(), opcode.end(), engine);
    
    float highValue =  -1 ;
    int highOp = -1;
   
    if( random_training == false ||  alpha == 0 ) //�p�G���ϥ��H���V�m�A�άO���ݭn�V�m(�V�m�T��=0)
     {
		    
        for (int op : opcode) {
          board TempBoard = before;                   //�ϥΪ�l����(�ǤJ������)
		    	board::reward reward = TempBoard.slide(op); //������op��V�Ƨ��᪺����
          float value = countValue(TempBoard);        //�����Ƨ��᪺�����ƭ�
          
		    	if (reward != -1) //�p�G�Ӥ�V�i�ư�
          {
            if( value > highValue) //�M��ƭȳ̰�����V�A�O���U��
            {  
              highValue = value;
              highOp = op;
            }     
          }
	    	}
   
        if(highOp != -1) //�p�G�ܤ֦��@��V�i�ʡA�ϥθӼƭȡC
            return action::slide(highOp);
        else             //�Ҧ���V�����i�ʡAGG�C     
	    	    return action();
     }
     else //�p�G�ϥ��H���V�m
     { 
        for (int op : opcode) //�H���D�@�ӥi�ƪ���V�ϥ�
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
	std::uniform_int_distribution<int> random; //�H���ѼơA�غc�ɷ|�]�m�d��
};
