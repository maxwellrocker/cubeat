﻿
class Ai{
	public function Ai(game: Game, map: Map){
		game_ = game;
		map_ = map;
		best_amass_point = 0;
		best_chain_point = 0;
		best_amass_step_x = -1;
		best_amass_step_y = -1;
		best_chain_step_x = -1;
		best_chain_step_y = -1;
		test_shoot = false;
		test_shoot_x = -1;
		test_shoot_y = -1;
		trace("Hi,I am Ai!");
	}
	public function starter(){
		trace("travel start");
	}
	private function first_shoot(brain_map: Map, sx: Number, sy: Number, ts: Boolean){
		if(ts == true){
			trace("test shoot(" + sx + "," + sy + ")");
			del_block(brain_map, sx, sy, true);
			this.test_shoot = false;
		}
	}
	
	public function full_process(map: Map){
		reset_best_step();
		var unchanged_map: Map = set_brain_map(map);
		//first_shoot(unchanged_map, test_shoot_x, test_shoot_y, test_shoot);
		trace_map_state(unchanged_map);
		trace_map_rgb(unchanged_map);
		//combo_counter(brain_map, 4, 10);
		/*for(var h = unchanged_map.Height-1; h > 0; --h){
			for(var w = unchanged_map.Width-1; w >= 0 ; --w){
				if(unchanged_map.lookup(w,h)!= null){
					var temp_map: Map = set_brain_map(map);
					del_block(temp_map, w, h, true);
					combo_counter(temp_map, w, h);
				}
			}
		}*/
		amass_counter(unchanged_map,this.best_amass_step_x, this.best_amass_step_y);
		for(var h = unchanged_map.Height-1; h > 0; --h){
			for(var w = unchanged_map.Width-1; w >= 0 ; --w){
				if(unchanged_map.lookup(w,h)!= null){
					var temp_map: Map = set_brain_map(map);
					del_block(temp_map, w, h, true);
					trace_map_rgb(temp_map);
					amass_counter(temp_map, w, h);
				}
			}
		}
		trace("Best combo step(" + best_chain_step_x + "," + best_chain_step_y + ")");
		trace("Best amass step(" + best_amass_step_x + "," + best_amass_step_y + ")");
	}
	
	//clone true map to think
	private function set_brain_map (true_map: Map): Map{
		var clone_map: Map = true_map.clone();
		trace("<map cloned>");
		//trace_map_state(clone_map);
		//trace_map_rgb(clone_map);
		return clone_map;
	}
	//count amass then get point
	private function amass_counter(brain_map: Map, ac_x: Number, ac_y: Number){
		if(contact_checker(brain_map, 3) == null){
			var get_point: Number = 0;
			get_point = contact_checker(brain_map, 2).length;
			if(get_point > this.best_amass_point){
				this.best_amass_point = get_point;
				this.best_amass_step_x = ac_x;
				this.best_amass_step_y = ac_y;
			}
			trace("amass point:" + get_point);
		}
	}
	
	//count chain combo then get point
	private function combo_counter(brain_map: Map, cc_x: Number, cc_y: Number){
		var square_count: Array = new Array();
		var chain_count: Number = 0;
		var get_point: Number = 0;
		while(contact_checker(brain_map,3) != null && chain_count < 10){
			//use Array to save contace_checker
			var chain_arr: Array = new Array();
			chain_arr = contact_checker(brain_map,3);
			trace("del chain(len=" + chain_arr.length + ") : "+ chain_arr );
			++chain_count;
			square_count.push(chain_arr.length);
			chain_del(brain_map, chain_arr);
			trace("chain_count = " + chain_count);
			trace("square_count = " + square_count[chain_count-1]);
			trace("next chain : "  + contact_checker(brain_map,3));
		}
		//(del_square_in_this_chain-3)*this_chain_Num+2^(this_chain_Num-2)
		for(var i = 0; i < square_count.length; ++i){
			get_point += (square_count[i]-3)*i+1;
			if( i == 0 ){
				get_point += (square_count[i]-3)*(i+1);
			}else{
				get_point += (square_count[i]-3)*(i+1) + Math.pow(2, i-1);
			}
		}
		if(get_point > this.best_chain_point){
			this.best_chain_point = get_point;
			this.best_chain_step_x = cc_x;
			this.best_chain_step_y = cc_y;
		}
		trace("Final_chain_count = " + chain_count);
		trace("All_square_count  = " + square_count);
		trace("chain point:" + get_point);
		trace_map_state(brain_map);
		trace_map_rgb(brain_map);
	}
	//treavl map to find chain or two_contact
	private function contact_checker(brain_map: Map, contact_Num: Number): Array{
		var temp_arr: Array = new Array();
		var push_arr: Array = new Array();
		for(var h = brain_map.Height-1; h > 0; --h){
			for(var w = brain_map.Width-1; w >= 0 ; --w){
				if(brain_map.make_row(brain_map.lookup(w,h)).length >= contact_Num-1){
					push_arr = brain_map.make_row( brain_map.lookup(w,h) );
					push_arr.push( brain_map.lookup(w,h) );
					//trace("MR(len=" + push_arr.length + ") : " + "find row from (" + w + "," + h + ")");
					for(var push_row = 0; push_row < push_arr.length; ++push_row){
						if(push_arr[push_row].cycled == false){
							push_arr[push_row].cycled = true;
							temp_arr.push(push_arr[push_row]);
						}
					}
				}
				if(brain_map.make_column(brain_map.lookup(w,h)).length >= contact_Num-1){
					push_arr = brain_map.make_column( brain_map.lookup(w,h) );
					push_arr.push( brain_map.lookup(w,h) );
					//trace("MC(len=" + push_arr.length + ") : "+ "find col from (" + w + "," + h + ")");
					for(var push_col = 0; push_col < push_arr.length; ++push_col){
						if(push_arr[push_col].cycled == false){
							push_arr[push_col].cycled = true;
							temp_arr.push(push_arr[push_col]);
						}
					}
				}
			}
		}
		for(var re_cy = 0; re_cy < temp_arr.length; ++re_cy){
			temp_arr[re_cy].cycled = false;
		}
		//trace("squares in this check(len=" + temp_arr.length + ") : ");
		if(temp_arr.length == 0){
			return null;
		}else{
			return temp_arr;
		}
	}
	
	private function Dropping_checker(brain_map: Map){
		
	}
	
	private function chain_del(brain_map: Map, del_arr: Array){
		trace("start chain_del");
		//delete del_arr without sort
		for(var cube_Num = 0; cube_Num < del_arr.length; ++cube_Num){
			del_block(brain_map, del_arr[cube_Num].x, del_arr[cube_Num].y, false);
			trace("del(" + del_arr.length + "-" + cube_Num + ") = " + del_arr);
		}
		//sort map
		for(var h = brain_map.Height-1; h > 0; --h){
			for(var w = brain_map.Width-1; w >= 0 ; --w){
				if(brain_map.lookup(w, h) == null){
					move_block(brain_map, w, h);					
				}
			}
		}
	}
	private function del_block(brain_map: Map, x: Number, y: Number, need_move: Boolean){
		var temp_map: Map = brain_map;
		var del_x = x;
		var del_y = y;
		trace("delete(" + del_x + "," + del_y + ")");
		temp_map[del_y][del_x].suicide();
		temp_map.delData(del_x, del_y);
		if(need_move == true){
			move_block(temp_map, del_x, del_y);
		}
	}
	
	private function move_block(brain_map: Map, x: Number, y: Number){
		var temp_map: Map = brain_map;
		var locx = x;
		var locy = y;
		for(var target_y = locy-1;
			target_y > 0 &&
			temp_map.lookup(locx,locy) == null;
			--target_y){
			if(temp_map.lookup(locx,target_y) != null && temp_map.lookup(locx,target_y).state instanceof Waiting){
				temp_map.setup(locx, locy, temp_map.lookup(locx, target_y));
				temp_map.lookup(locx, locy).y = locy;
				trace("move (" + locx + "," + target_y + ") to (" + locx + "," + locy + ")");
				del_block(temp_map, locx, target_y, true);
			}
		}
	}
	
	private function trace_map_state(brain_map: Map){
		var state_arr: Array = new Array(brain_map.Height);
        for(var i = 0; i < brain_map.Height; ++i){
			state_arr[i] = new Array(brain_map.Width);
		}
		for(var h = brain_map.Height-1; h > 0; --h){
			for(var w = brain_map.Width-1; w >= 0 ; --w){
				if( brain_map.lookup(w,h).state instanceof Waiting )
				    state_arr[h][w] = "Wa";
				else if ( brain_map.lookup(w,h).state instanceof Dropping )
				    state_arr[h][w] = "Dr";
				else if ( brain_map.lookup(w,h).state instanceof Dying )
				    state_arr[h][w] = "Dy";
				else
				    state_arr[h][w] = "--";
			}
		}
		trace("map_state:");
		for(var a = 0; a < brain_map.Height; ++a){
			trace(state_arr[a]);
		}
	}
	
	private function trace_map_rgb(brain_map: Map){
		var rgb_arr: Array = new Array(brain_map.Height);
        for(var i = 0; i < brain_map.Height; ++i){
			rgb_arr[i] = new Array(brain_map.Width);
		}
		for(var h = brain_map.Height-1; h > 0; --h){
			for(var w = brain_map.Width-1; w >= 0 ; --w){
				if( brain_map.lookup(w,h).rgb == 255 << 16 )
				    rgb_arr[h][w] = "Re";
				else if ( brain_map.lookup(w,h).rgb == 255 << 8 )
				    rgb_arr[h][w] = "Gr";
				else if ( brain_map.lookup(w,h).rgb == 255 )
				    rgb_arr[h][w] = "Bl";
				else if ( brain_map.lookup(w,h).rgb == (255<<16) + (255<<8) )
				    rgb_arr[h][w] = "Ye";
				else
				    rgb_arr[h][w] = "--";
			}
		}
		trace("map_RGB:");
		for(var a = 0; a < brain_map.Height; ++a){
			trace(rgb_arr[a]);
		}
	}
	
	private function reset_best_step(){
		this.best_amass_step_x = -1;
		this.best_amass_step_y = -1;
		this.best_chain_step_x = -1;
		this.best_chain_step_y = -1;
		this.best_chain_point = 0;
		this.best_amass_point = 0;
	}
	
	private var best_amass_step_x: Number;
	private var best_amass_step_y: Number;
	private var best_chain_step_x: Number;
	private var best_chain_step_y: Number;
	private var best_amass_point: Number;
	private var best_chain_point: Number;
	private var game_: Game;
	private var map_: Map;
	private var test_shoot: Boolean;
	private var test_shoot_x: Number;
	private var test_shoot_y: Number;
}