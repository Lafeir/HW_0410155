/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++11 -O3 -g -o 2048 2048.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */
 
// Version : 05

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"
#include "weight.h"
#include <fstream>
#include "solver.h"
#include <string>

int main(int argc, const char* argv[]) {

  int projectNowIs = 4;
  
  if(projectNowIs == 3)
  {
     	std::string solve_args;
     	int precision = 10;
      
     	for (int i = 1; i < argc; i++) {
     		std::string para(argv[i]);
     		if (para.find("--solve=") == 0) {
     			solve_args = para.substr(para.find("=") + 1);
     		} else if (para.find("--precision=") == 0) {
     			precision = std::stol(para.substr(para.find("=") + 1));
     		}
     	}
     
     	solver solve(solve_args);
     	
      std::cout << std::setprecision(precision);
      
      char intype,inadd[2];
      int t[6]={0};
      int tt[6]={0};
      
      if(true)
      { 
          while (std::cin >> intype >> t[0]>>t[1]>>t[2]>>t[3]>>t[4]>>t[5] >> inadd) 
          {
            for(int tran = 0 ; tran < 6;tran++)
            {
              tt[tran] = t[tran];
            
              if(tt[tran] == 6) tt[tran] = 4;
              else if(tt[tran] == 12) tt[tran] = 5;
              else if(tt[tran] == 24) tt[tran] = 6;
              else if(tt[tran] == 48) tt[tran] = 7;
              else if(tt[tran] == 96) tt[tran] = 8;
            }
            
            
         		double value[3] = { -1 , -1 , -1 };
            
            solve.solve( value, intype,  tt ,inadd[1]);
         		std::cout << intype << " " << t[0] << " " << t[1] << " " << t[2] << " " << t[3] << " " << t[4] << " " << t[5] <<  " " << inadd;
            
            if(value[0] == -1) 	std::cout << " = -1" << std::endl;
            else                std::cout << " = " << value[2] << " " << value[1] << " " <<  value[0]<< std::endl; 
           	//std::cout << " = " << value[2] << " " << value[1] << " " <<  value[0]<< std::endl; 
         	}
      
      }
      
  }  
  else if(projectNowIs == 4)
  {	
  
      std::cout << "Threes!(Project : "<<projectNowIs<<" )-Demo: ";
    	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
    	std::cout << std::endl << std::endl;
 
 
    	size_t total = 1000, block = 0, limit = 0;
    	std::string play_args, evil_args;
    	std::string load, save;
    	bool summary = false;
     
      //確認初始設置
    	for (int i = 1; i < argc; i++) {
    		std::string para(argv[i]);
    		if (para.find("--total=") == 0) {
    			total = std::stoull(para.substr(para.find("=") + 1));
    		} else if (para.find("--block=") == 0) {
    			block = std::stoull(para.substr(para.find("=") + 1));
    		} else if (para.find("--limit=") == 0) {
    			limit = std::stoull(para.substr(para.find("=") + 1));
    		} else if (para.find("--play=") == 0) {      //Project-02，權重表使用
    			play_args = para.substr(para.find("=") + 1);
    		} else if (para.find("--evil=") == 0) {
    			evil_args = para.substr(para.find("=") + 1);
    		} else if (para.find("--load=") == 0) {     
    			load = para.substr(para.find("=") + 1);
    		} else if (para.find("--save=") == 0) {      
    			save = para.substr(para.find("=") + 1);
    		} else if (para.find("--summary") == 0) {
    			summary = true;
    		} 
       
    	}
    
      
    	statistic stat(total, block, limit);
    
    	if (load.size()) {
    		std::ifstream in(load, std::ios::in);
    		in >> stat;
    		in.close();
    		summary |= stat.is_finished();
    	}
    
    	player play(play_args);
    	rndenv evil(evil_args);
    
      //是否重置訓練Table (如果未讀取的話的話，自動使用)
      if(play.using_net == false)
      {
         play.init_weights("reset");
      }
      play.init_bitboard();
    
    	while (!stat.is_finished()) {
    		play.open_episode("~:" + evil.name());
    		evil.open_episode(play.name() + ":~");
    
    		stat.open_episode(play.name() + ":" + evil.name());
    		episode& game = stat.back();
       
        //每輪開始時，重整背包。
        evil.rebag();  
        evil.RandomCoolDown = 100;
        
        //設置一些需要的東西。
        board lastBoard = game.state();
        double thisValue = 0, lastValue = play.countValue(lastBoard);
        float thisScroe = 0 , lastScore = play.countScore(lastBoard);
        bool Start = false;
        int RandomCoolDown = 0;
    
        while (true) {
            
          //檢查換誰了( episode.h的take_turns中有初始輪替的順序設置 )
    	  	agent& who = game.take_turns(play, evil);
         
          //第一次輪到玩家時，紀錄目前的數值與盤面。  
          if(Start == false && who.name() == play.name() ) 
          {      
             Start = true;
             lastBoard = game.state();
             lastValue = play.countValue(lastBoard);
             lastScore = play.countScore(lastBoard);
           } 
          
          //確認Bouns格的冷卻時間 
          RandomCoolDown = evil.RandomCoolDown;
                
          //進行行動(玩家移動，敵方生成)      
    			action move = who.take_action(game.state());
    			if (game.apply_action(move) != true) break; //如果無法繼續遊戲，GG。
          
          //當輪到玩家時，  
          if( who.name() == play.name() )
          {      
             //確認目前的權重數值與分數。
             thisValue = play.countValue(game.state(),RandomCoolDown);
             thisScroe = play.countScore(game.state());
            
             //藉此更新上次版面的數值。         
             float delta = (thisScroe - lastScore) + (thisValue - lastValue);
             play.updateValue(lastBoard , delta, RandomCoolDown);
            
             //然後紀錄這次的版面。
             lastBoard = game.state();
             lastValue = thisValue;
             lastScore = thisScroe;       
           }
             
    		}
        
        //每輪結束後，將最終版面的權重值設為 0 
        double finalDelta = 0 - play.countValue(lastBoard);    
        play.updateValue( lastBoard , finalDelta);
       
        //每輪結束後，清除盤面上的100。
        play.reset100( game.state());
        
        //確認勝者
    		agent& win = game.last_turns(play, evil);
         
    		stat.close_episode(win.name());
    		play.close_episode(win.name());  
    		evil.close_episode(win.name());
    	}
    
    	if (summary) {
    		stat.summary();
    	}
    
    	if (save.size()) {
    		std::ofstream out(save, std::ios::out | std::ios::trunc);
    		out << stat;
    		out.close();
    	}
     
  }
  
	return 0;
}
