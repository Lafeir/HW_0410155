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
     
      //�T�{��l�]�m
    	for (int i = 1; i < argc; i++) {
    		std::string para(argv[i]);
    		if (para.find("--total=") == 0) {
    			total = std::stoull(para.substr(para.find("=") + 1));
    		} else if (para.find("--block=") == 0) {
    			block = std::stoull(para.substr(para.find("=") + 1));
    		} else if (para.find("--limit=") == 0) {
    			limit = std::stoull(para.substr(para.find("=") + 1));
    		} else if (para.find("--play=") == 0) {      //Project-02�A�v����ϥ�
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
    
      //�O�_���m�V�mTable (�p�G��Ū�����ܪ��ܡA�۰ʨϥ�)
      if(play.using_net == false)
      {
         play.init_weights("reset");
      }
      play.init_bitboard();
    
      std::vector<board> boardEpisode1;
      std::vector<float> boardEpisode2;
      std::vector<float> boardEpisode3;
    
      //�C�@���}�l
    	while (!stat.is_finished()) {
     
    		play.open_episode("~:" + evil.name());
    		evil.open_episode(play.name() + ":~");
    
    		stat.open_episode(play.name() + ":" + evil.name());
    		episode& game = stat.back();
       
        //�C���}�l�ɡA����I�]�C
        evil.rebag();  
        evil.RandomCoolDown = 100;
        evil.nextTile = 100;
        play.RandomCoolDown = 100;
        play.nextTile = 100;
        evil.bounsCounter = 0;
        evil.normalCounter = 0;
        
        //�]�m�@�ǻݭn���F��C
        board lastBoard = game.state();
        double thisValue = 0, lastValue = 0;
        float thisScroe = 0 , lastScore = 0;
        bool Start = false;
        int RandomCoolDown = 100 , lastRandomCoolDown = 100;
        int nextTile = 100 , lastNextTile = 100;
    
        boardEpisode1.clear();
	    	boardEpisode1.reserve(32768);
        boardEpisode2.clear();
	    	boardEpisode2.reserve(32768);
        boardEpisode3.clear();
	    	boardEpisode3.reserve(32768);
      

        while (true) {
                         
          //�ˬd���֤F( episode.h��take_turns������l���������ǳ]�m )
    	  	agent& who = game.take_turns(play, evil);       
         
          //���쪱�a�ɡA  
          if(who.name() == play.name() ) 
          {                                   
                //��s�U������l�M���y��N�o
                play.RandomCoolDown = RandomCoolDown;
                play.nextTile = nextTile;
                
                if(Start == false) //�Ĥ@�������ثe���ƭȻP�L���C
                {
                    Start = true;
                   lastBoard = game.state();
                   lastValue = play.countValue(lastBoard, lastRandomCoolDown , lastNextTile);
                   lastScore = play.countScore(lastBoard);
                }
                  
          }
          else  if(who.name() == evil.name() )  //���Ĥ�ɡA�����W������l�M���y��N�o
          {
           lastRandomCoolDown = RandomCoolDown;
           lastNextTile = nextTile;
          } 
          
        
           
                
          //�i����(���a���ʡA�Ĥ�ͦ�)      
    			action move = who.take_action(game.state());
    			if (game.apply_action(move) != true) break; //�p�G�L�k�~��C���AGG�C
          
          //����쪱�a�ɡA  
          if( who.name() == play.name() )
          {      
             //�T�{�ثe���v���ƭȻP���ơC
             //thisValue = play.countValue(game.state(),RandomCoolDown,nextTile);
             //thisScroe = play.countScore(game.state());
            
             //�Ǧ���s�W���������ƭȡC         
             //float delta = (thisScroe - lastScore) + (thisValue - lastValue);
             //play.updateValue(lastBoard , delta, lastRandomCoolDown,lastNextTile);
            
             boardEpisode1.push_back(lastBoard);
             boardEpisode2.push_back(lastRandomCoolDown);
             boardEpisode3.push_back(lastNextTile);
             
             //�M������o���������C
             lastBoard = game.state();
             lastValue = thisValue;
             lastScore = thisScroe;                         
                        
           }
           else if(who.name() == evil.name() )   //���Ĥ�ɡA��s�U������l�M���y��N�o
           {
                    
            nextTile = evil.nextTile;
            RandomCoolDown = evil.RandomCoolDown;
          }
             
    		}
        
        //�C��������A�N�̲ת������v���ȳ]�� 0 
        double finalDelta = 0 - play.countValue(boardEpisode1[boardEpisode1.size()-1],boardEpisode2[boardEpisode1.size()-1] ,boardEpisode3[boardEpisode1.size()-1]);    
        play.updateValue( boardEpisode1[boardEpisode1.size() - 1] , finalDelta, boardEpisode2[boardEpisode1.size() - 1] ,boardEpisode3[boardEpisode1.size() - 1] );
       
	    	for(int i = boardEpisode1.size() - 2; i >= 0; i--){
		    	//state step_next = episode[i + 1];
	    		//train_weights(episode[i].after, step_next.after, step_next.reward);
                                                             
          //�T�{�ثe���v���ƭȻP���ơC
          thisValue = play.countValue(boardEpisode1[i+1],boardEpisode2[i+1],boardEpisode3[i+1]);
          thisScroe = play.countScore(boardEpisode1[i+1]);
          
          lastValue = play.countValue(boardEpisode1[i],boardEpisode2[i],boardEpisode3[i]);
          lastScore = play.countScore(boardEpisode1[i]);
            
          //�Ǧ���s�W���������ƭȡC         
          float delta = (thisScroe - lastScore) + (thisValue - lastValue);
          
          play.updateValue( boardEpisode1[i], delta,boardEpisode2[i],boardEpisode3[i]);
                                                    
	    	}
       
        //�C��������A�M���L���W��100�C
        play.reset100( game.state());
        
        //�T�{�Ӫ�
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
