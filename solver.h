#pragma once
#include <iostream>
#include <algorithm>
#include <cmath>
#include "board.h"
#include <numeric>
#include "weight.h"
#include <math.h>     

class state_type {
public:
	enum type : char {
		before  = 'b',
		after   = 'a',
		illegal = 'i'
	};

public:
	state_type() : t(illegal) {}
	state_type(const state_type& st) = default;
	state_type(state_type::type code) : t(code) {}

	friend std::istream& operator >>(std::istream& in, state_type& type) {
		std::string s;
		if (in >> s) type.t = static_cast<state_type::type>((s + " ").front());
		return in;
	}

	friend std::ostream& operator <<(std::ostream& out, const state_type& type) {
		return out << char(type.t);
	}

	bool is_before()  const { return t == before; }
	bool is_after()   const { return t == after; }
	bool is_illegal() const { return t == illegal; }

private:
	type t;
};

class state_hint {
public:
	state_hint(const board& state) : state(const_cast<board&>(state)) {}

	char type() const { return state.info() ? state.info() + '0' : 'x'; }
	operator board::cell() const { return state.info(); }

public:
	friend std::istream& operator >>(std::istream& in, state_hint& hint) {
		while (in.peek() != '+' && in.good()) in.ignore(1);
		char v; in.ignore(1) >> v;
		hint.state.info(v != 'x' ? v - '0' : 0);
		return in;
	}
	friend std::ostream& operator <<(std::ostream& out, const state_hint& hint) {
		return out << "+" << hint.type();
	}

private:
	board& state;
};


class solver {
public:
	typedef float value_t;

public:
  	class answer {
  	public:
  		answer(value_t min = 0.0/0.0, value_t avg = 0.0/0.0, value_t max = 0.0/0.0) : min(min), avg(avg), max(max) {}
  	    friend std::ostream& operator <<(std::ostream& out, const answer& ans) {
  	    	return !std::isnan(ans.avg) ? (out << ans.min << " " << ans.avg << " " << ans.max) : (out << "-1") << std::endl;
  		}
  	public:
  		const value_t min, avg, max;
  	};

public:

 
  //重置背包
  virtual void rebag( int bag[3])
	{
			bag[0] = 1;
			bag[1] = 2;
			bag[2] = 3;
	}
 
  //檢查背包空了沒
 	virtual void checkbag( int bag[3]  )
	{
		bool empty = true;
		for (int i = 0; i < 3; i++)
		{
			if (bag[i] != 0)
				empty = false;
		}
		if (empty)
		{
			rebag(bag);
		}
	}

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
  
  size_t GetKey( int dir,int nextAdd, int board[6] )
  {
      return (
                dir *  power(10,7) + 
                nextAdd * power(10,6) + 
                board[5]* power(10,5) + 
                board[4]* power(10,4) + 
                board[3]* power(10,3) + 
                board[2]* power(10,2) + 
                board[1]* power(10,1) + 
                board[0]* power(10,0)    );
  }
  

  virtual void init_weights() 
  {

     net.emplace_back(100000000); // create an empty weight table with size  (10^6) * 10(3-tile) * 10(4-dir) 
	   net.emplace_back(100000000); // create an empty weight table with size  (10^6) * 10(3-tile) * 10(4-dir) 
     net.emplace_back(100000000); // create an empty weight table with size  (10^6) * 10(3-tile) * 10(4-dir) 
     net.emplace_back(100000000); // create an empty weight table with size  (10^6) * 10(3-tile) * 10(4-dir) 
     net.emplace_back(100000000); // create an empty weight table with size  (10^6) * 10(3-tile) * 10(4-dir) 
     net.emplace_back(100000000); // create an empty weight table with size  (10^6) * 10(3-tile) * 10(4-dir) 

	
     //Set to 0
     for( size_t i = 0 ; i < net[0].size() ; i++)
     {
       net[0][i] = -1; //before - + - max
       net[1][i] = -1; //before - + - avg
       net[2][i] = -1; //before - + - min
       
       net[3][i] = -1; //after  - + - max
       net[4][i] = -1; //after  - + - avg
       net[5][i] = -1; //after  - + - min
     } 
     
     //std::cout<<std::endl<<"Finish Net Initially!!"<<std::endl;
	}
 

  int slideBoard( int board[6] , int dir ) //左下右上
  {
      
      //std::cout<<"Before :" <<dir<<" "<< board[0]<< board[1]<< board[2]<< board[3]<< board[4]<< board[5]<<std::endl;
      

      
      int x_str , x_end , x_dir , y_str , y_end , y_dir;
      bool returnMove = false;
      if(dir == 1){       x_str = 0 ; x_end =2; x_dir=1;  y_str=0;  y_end=3;  y_dir=1;    }
      else if(dir == 2){  x_str = 0 ; x_end =3; x_dir=1;  y_str=1;  y_end=-1;  y_dir=-1;   }
      else if(dir == 3){  x_str = 0 ; x_end =2; x_dir=1;  y_str=2;  y_end=-1;  y_dir=-1;    }
      else if(dir == 4){  x_str = 0 ; x_end =3; x_dir=1;  y_str=0;  y_end=2;  y_dir=1;     }
      
     	for (int x = x_str; x != x_end; x += x_dir) 
      {		  
            int last = 0;
            int boardKey = 0;
            int del = 0;
            bool move = false;
            
            for (int y = y_str; y != y_end; y += y_dir ) 
            { 
                 
                  if(dir == 1){      boardKey =  x*3 + y;   del = -1; }
                  else if(dir == 2){ boardKey =  y*3 + x;   del = 3; }
                  else if(dir == 3){ boardKey =  x*3 + y;   del = 1; }
                  else if(dir == 4){ boardKey =  y*3 + x;   del = -3; }
                  
                 //剛開始直接拿。(沒動)
                 if( y == y_str)
                 {
                    last = board[boardKey];
                    board[boardKey] = 0;
                 }
                 else if( board[boardKey] == 0) // 若此格為0，上格擺回去，拿新的。
                 {
                    board[boardKey + del ] = last; 
                    last = board[boardKey];
                    board[boardKey] = 0;
                    
                 } 
                 else if(last == 0) // 若這格不為0 ，且上格為0，移動。
                 {
                        board[boardKey  + del ] = board[boardKey];
                        last = 0;
                        board[boardKey] = 0;
                        move = true;
                 }              
                 //以下該格有數字
                 else if( last == 1) //若上格是1
                 {
                    if( board[boardKey] == 2) //融合
                    {
                        board[boardKey  + del  ] = 3;
                        last = 0;
                        board[boardKey] = 0;
                        move = true;
                    }
                    else //上格擺回去，拿新的。
                    {
                        board[boardKey  + del  ] = last;
                        last = board[boardKey];
                        board[boardKey] = 0;                     
                    } 
                 }
                 else if( last == 2)  //若上格是2
                 {
                    if( board[boardKey] == 1) //融合
                    {
                        board[boardKey  + del ] = 3;
                        last = 0;
                        board[boardKey] = 0;
                        move = true;                    
                    }
                    else //上格擺回去，拿新的。
                    {
                        board[boardKey + del  ] = last;
                        last = board[boardKey];
                        board[boardKey] = 0;                     
                    } 
                 }
                 else  //若上格是其他數字
                 {
                    if( board[boardKey] == last) //融合
                    {
                        board[boardKey  + del  ] = last + 1;
                        last = 0;
                        board[boardKey] = 0;
                        move = true;                      
                    }
                    else //上格擺回去，拿新的。
                    {
                        board[boardKey + del  ] = last;
                        last = board[boardKey];
                        board[boardKey] = 0;                     
                    } 
                 }
                  
            }
            
            //把最後一格補回去。          
            //ver1127
            if(!move)
              board[boardKey] = last;
            else
              {
                board[boardKey] = 0;
                returnMove = true;
              }
            last = 0;
                            
      }
      
      //std::cout<<"After :" <<dir<<" "<< board[0]<< board[1]<< board[2]<< board[3]<< board[4]<< board[5]<<std::endl;
      
      if( returnMove == false )
      {
          return -1;
      }
      else
      {
          return 1;
      }
    
  }
 
  void recursiveSolve(  int type ,  int& add ,  int board[6],  int bag[3] ,  int dir = 0)
  {
    int keyboard[6] = {0};    
    for(int k = 0 ; k < 6 ; k++)  { keyboard[k] =board[k]; if(keyboard[k] == 100) keyboard[k] = 0;}
  
    if(type == 0) //輪到玩家(before)
    {
        //若不等於-1，代表已經做過此格的計算了。
        size_t thisKey =  GetKey( 0 ,add ,keyboard );
        if(net[0][thisKey] != -1 ) return;
        
  
        
        double max_max = 0 ,  max_avg = 0 ,  max_min = 0;
        bool success = false;
        
        for(int nextDir = 1 ; nextDir < 5 ; nextDir++) //決定下個滑動方向
        {
            int nextboard[6] = {0};      
            for(int p = 0 ; p < 6 ; p++)  nextboard[p] =board[p];
            
            int result =  slideBoard(nextboard , nextDir ); //滑動nextbaord，同時判斷是否成功。                
            
            if(  result != -1  ) //若此方向可滑
            {                   
                  success = true; 
                  recursiveSolve( 1 , add , nextboard , bag , nextDir );   
                  
                  //ver1127
                  for(int p = 0 ; p < 6 ; p++)  if(nextboard[p] == 100) nextboard[p] = 0;
                  
                  size_t key =  GetKey( nextDir, add ,nextboard );    
                  if( max_max == 0 || net[3][key] > max_max ) max_max = net[3][key]; //after  - + - max                 
                  if( max_avg == 0 || net[4][key] > max_avg ) max_avg = net[4][key]; //after  - + - avg                
                  if( max_min == 0 || net[5][key] > max_min ) max_min = net[5][key]; //after  - + - min                         
            }    
                
        }
        
        
        if(success)
        {
          //跑過了所有可能性，紀錄分數。
            net[0][thisKey] = max_max            ; //after  - + - max
            net[1][thisKey] = max_avg            ; //after  - + - avg
            net[2][thisKey] = max_min            ; //after  - + - min   
        }
        else //若無處可滑，直接計算此格分數。
        { 
            double score = 0;
            for(int i = 0 ; i < 6 ; i ++)
            {
               int tile = keyboard[i];
               if(tile>=3)
               {
                   tile = tile - 2;
                   score += power(3,tile);
               }
            }
          
            net[0][thisKey] = score            ; //after  - + - max
            net[1][thisKey] = score            ; //after  - + - avg
            net[2][thisKey] = score            ; //after  - + - min   
            
            //std::cout<<board[0]<<" "<<board[1]<<" "<<board[2]<<" "<<board[3]<<" "<<board[4]<<" "<<board[5]<<" +"<<add << " = " <<  score <<std::endl;
            
        }
      
        //std::cout<<board[0]<<" "<<board[1]<<" "<<board[2]<<" "<<board[3]<<" "<<board[4]<<" "<<board[5]<<" +"<<add << " = " <<  net[0][thisKey] <<std::endl;
    }
    else if(type == 1) //輪到敵方(after)
    {
    
        //若不等於-1，代表已經做過此格的計算了。  
        size_t thisKey =  GetKey( dir ,add ,keyboard );
        //if(net[3][thisKey] != -1 ) return;
              
        int nextBag[3] = {0};    
        for(int b = 0 ; b < 3 ; b++)  nextBag[b] =bag[b];
        nextBag[add] = 0; //拿走該方塊
        checkbag(nextBag); //確認背包空了沒(空了就填滿)
     
        double caseNum = 0;
        double max = 0 , avg = 0 , min = 100000;
                             
        if(dir == 0) //開局
        {
            for(int i = 0 ; i < 6 ; i++) //隨機空格
            {
                if( board[i] == 0) //如果該處有空間
                {
                    int nextboard[6] = {0};    
                    for(int p = 0 ; p < 6 ; p++) { nextboard[p] =board[p]; if(nextboard[p] == 100) nextboard[p] = 0; }
                    nextboard[i] = add+1;
                
                    for(int nextAdd = 0 ; nextAdd < 3 ; nextAdd++) //隨機下個方塊
                    {
                       if( nextBag[nextAdd] != 0) // 如果該方塊可使用
                       {                   
                           recursiveSolve( 0 , nextAdd , nextboard, nextBag );
                           
                           size_t key =  GetKey( 0 ,nextAdd ,nextboard );
                           
                           caseNum += 1;                                        
                           if(net[0][key] > max ) max = net[0][key]; //before  - + - max      
                           avg +=  net[1][key] ;                                  //before  - + - avg             
                           if(net[2][key] < min ) min = net[2][key]; //before  - + - min                
                       }
                    }  
                }           
            }        
        }
        else if(dir == 1) //往左
        {
            for(int i = 0 ; i < 2 ; i++) //隨機空格
            {
                int boardKey =  i*3 + 2;
                if( board[boardKey] == 0) //如果該處有空間
                {
                    int nextboard[6] = {0};    
                    for(int p = 0 ; p < 6 ; p++) { nextboard[p] =board[p]; if(nextboard[p] == 100) nextboard[p] = 0; }
                    nextboard[boardKey] =  add+1;
                
                    for(int nextAdd = 0 ; nextAdd < 3 ; nextAdd++) //隨機下個方塊
                    {
                       if( nextBag[nextAdd] != 0) // 如果該方塊可使用
                       {
                           recursiveSolve( 0 , nextAdd , nextboard , nextBag );
                           
                           size_t key =  GetKey( 0 ,nextAdd ,nextboard );
                           
                           caseNum += 1;   
                           if( net[0][key] > max ) max = net[0][key]; //before  - + - max      
                           avg +=  net[1][key] ;                                  //before  - + - avg             
                           if( net[2][key] < min ) min = net[2][key]; //before  - + - min          
                       }
                    }  
                }           
            }        
        }
        else if(dir == 2) //往下
        {
            for(int i = 0 ; i < 3 ; i++) //隨機空格
            {
                int boardKey =  i;
                if( board[boardKey] == 0) //如果該處有空間
                {
                    int nextboard[6] = {0};    
                    for(int p = 0 ; p < 6 ; p++) { nextboard[p] =board[p]; if(nextboard[p] == 100) nextboard[p] = 0; }
                    nextboard[boardKey] =  add+1;
                
                    for(int nextAdd = 0 ; nextAdd < 3 ; nextAdd++) //隨機下個方塊
                    {
                       if( nextBag[nextAdd] != 0) // 如果該方塊可使用
                       {
                           recursiveSolve( 0 , nextAdd , nextboard , nextBag );
                           
                           size_t key =  GetKey( 0 ,nextAdd ,nextboard );
                           
                           caseNum += 1;   
                           if( net[0][key] > max ) max = net[0][key]; //before  - + - max      
                           avg +=  net[1][key] ;                                  //before  - + - avg             
                           if( net[2][key] < min ) min = net[2][key]; //before  - + - min             
                       }
                    }  
                }           
            }      
        }
        else if(dir == 3) //往右
        {        
           for(int i = 0 ; i < 2 ; i++) //隨機空格
            {
                int boardKey =  i*3;
                if( board[boardKey] == 0) //如果該處有空間
                {
                    int nextboard[6] = {0};    
                    for(int p = 0 ; p < 6 ; p++) { nextboard[p] =board[p]; if(nextboard[p] == 100) nextboard[p] = 0; }
                    nextboard[boardKey] = add+1;
                
                    for(int nextAdd = 0 ; nextAdd < 3 ; nextAdd++) //隨機下個方塊
                    {
                       if( nextBag[nextAdd] != 0) // 如果該方塊可使用
                       {
                           recursiveSolve( 0 , nextAdd , nextboard, nextBag );
                           
                           size_t key =  GetKey( 0 ,nextAdd ,nextboard );
                           
                           caseNum += 1;   
                           if( net[0][key] > max ) max = net[0][key]; //before  - + - max      
                           avg +=  net[1][key] ;                                  //before  - + - avg             
                           if( net[2][key] < min ) min = net[2][key]; //before  - + - min   
                       }
                    }  
                }           
            }        
        }
        else if(dir == 4) //往上
        {
            for(int i = 0 ; i < 3 ; i++) //隨機空格
            {
                int boardKey = 3 + i;
                if( board[boardKey] == 0) //如果該處有空間
                {
                    int nextboard[6] = {0};    
                    for(int p = 0 ; p < 6 ; p++) { nextboard[p] =board[p]; if(nextboard[p] == 100) nextboard[p] = 0; }
                    nextboard[boardKey] =  add+1;
                
                    for(int nextAdd = 0 ; nextAdd < 3 ; nextAdd++) //隨機下個方塊
                    {
                       if( nextBag[nextAdd] != 0) // 如果該方塊可使用
                       {
                           recursiveSolve( 0 , nextAdd , nextboard , nextBag );
                           
                           size_t key =  GetKey( 0 ,nextAdd ,nextboard );
                           
                           caseNum += 1;   
                           if( net[0][key] > max ) max = net[0][key]; //before  - + - max      
                           avg +=  net[1][key] ;                                  //before  - + - avg             
                           if( net[2][key] < min ) min = net[2][key]; //before  - + - min   
                       }
                    }  
                }           
            }      
        } 
              
        //跑過了所有可能性，紀錄分數。
        net[3][thisKey] = max            ; //after  - + - max
        net[4][thisKey] = avg / caseNum  ; //after  - + - avg
        net[5][thisKey] = min            ; //after  - + - min   
    }
  
  }
  
  void startSolve()
  {          
      for(int add = 0 ; add < 3; add++) //三種開局
      {
                    
           int bag[3] = {1,2,3};
           int board[6] = { 0,0,0,0,0,0 };    
                                 
           recursiveSolve( 1 , add , board , bag);     
      }                             
        
  }

  void test(int t[3])
  {
    t[0] = 100;
  }

  //待編寫，要在這裡讓呼叫的時候算完並儲存所有可能的盤面變化
	solver(const std::string& args) {
		
      init_weights();
      //std::cout<<"a"<<std::endl;
      startSolve();
      //std::cout<<"b"<<std::endl;
   
	}
  
  //待編寫，要在這裡查詢並回傳答案
	void solve( double result[3] ,   char  intype, int ta[6] , char inadd) 
  {
      
    int ty  = 0 , ad = 0;
    
    if(intype == 'b') ty=0;
    if(intype == 'a') ty=1;
    
    if(inadd == '1') ad = 0;
    if(inadd == '2') ad = 1;
    if(inadd == '3') ad = 2;

    

    size_t thisKey;
    
    for(int i = 0 ; i < 5 ; i++)
    {
        thisKey =  GetKey( i, ad , ta );
        
        if(net[ty*3 + 0][thisKey] != -1) break;
    }
    
             
    result[0] = net[ty*3 + 0][thisKey]; //max
    result[1] = net[ty*3 + 1][thisKey]; //avg
    result[2] = net[ty*3 + 2][thisKey]; //min
    
    
  	// TODO: find the answer in the lookup table and return it
		//       do NOT recalculate the tree at here

		// to fetch the hint (if type == state_type::after, hint will be 0)
    //		board::cell hint = state_hint(state);

		// for a legal state, return its three values.
    //		return { min, avg, max };
		// for an illegal state, simply return {}
   

	}

private:
 
 	// TODO: place your transposition table here
  //宣告網路。
	std::vector<weight> net;

 
};
