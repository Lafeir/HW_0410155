/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++11 -O3 -g -o 2048 2048.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */

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

int main(int argc, const char* argv[]) {
	std::cout << "Threes!(ver.AI.1)-Demo: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	size_t total = 1000, block = 0, limit = 0;
	std::string play_args, evil_args;
	std::string load, save;
	bool summary = false;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--total=") == 0) {
			total = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--block=") == 0) {
			block = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--limit=") == 0) {
			limit = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--play=") == 0) {
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


  if(play.using_net == false)
  {
     play.init_weights("reset");
  }
  


	while (!stat.is_finished()) {
		play.open_episode("~:" + evil.name());
		evil.open_episode(play.name() + ":~");

		stat.open_episode(play.name() + ":" + evil.name());
		episode& game = stat.back();
   
    
    //紀錄初始盤面(空)
    board lastBoard = game.state();
    double lastValue = play.countValue(lastBoard);
    float lastScore = play.countScore(lastBoard);
    double thisValue = 0;
    float thisScroe = 0;
    bool Start = false;

    while (true) {
        
	  	agent& who = game.take_turns(play, evil);
             
      if( who.name() == play.name() && Start == false )
      {      
         Start = true;
         lastBoard = game.state();
         lastValue = play.countValue(lastBoard);
         lastScore = play.countScore(lastBoard);
         thisValue = 0;
         thisScroe = 0;
       } 
             
			action move = who.take_action(game.state());
			if (game.apply_action(move) != true) break;
      
      if( who.name() == play.name() )
      {      
         thisValue = play.countValue(game.state());
         thisScroe = play.countScore(game.state());
         //更新上次的版面分數         
         float delta = thisScroe - lastScore + thisValue - lastValue;
         play.updateValue(lastBoard , delta);
         //紀錄這次的版面
         lastBoard = game.state();
         lastValue = thisValue;
         lastScore = thisScroe;
       }
         
		}
    
    double finalDelta = -play.countValue(lastBoard);    
    play.updateValue( lastBoard , finalDelta);
   
    play.reset100( game.state());
    
    //play.showBoard(game.state());
    
		agent& win = game.last_turns(play, evil);
   
    //重新設置背包
    evil.rebag();  
		stat.close_episode(win.name());
		play.close_episode(win.name());
   
		evil.close_episode(win.name());
	}

	if (summary) {
		stat.summary();
	}

 //play.showWeight();

	if (save.size()) {
		std::ofstream out(save, std::ios::out | std::ios::trunc);
		out << stat;
		out.close();
	}

	return 0;
}
