﻿
class Ai{
	public function Ai(game: Game, map: Map, player: Number){
		game_ = game;
		map_ = map;
		player_n = player;
		travel_limit = 3;//設定暴力搜尋時的搜尋極限(到高為11-travel_limit為止)
		best_chain_point = [0,0,0];
		best_chain_cubes = [null,null,null];//儲存最佳的3個發火點方塊
		shooted = false;
		chaining = false;
		trace("Hi,I am Ai!");
	}
	
	public function process_bg(map: Map){
		//消去broken與garbage方塊s
		var bg_list: Array = bk_gbg_travel(map);
		if(bg_list != null){
			//trace("find broken or garbage !!");
			for (var i =0; i<bg_list.length; ++i){
				shooting_arr(map, bg_list[i]);
				this.shooted = false;
			}
		}
	}
	
	public function process_chain(map: Map){
		//找出前三名取得最高分方塊
		var c_cube: Square = null;
		var c_point: Number = 0;
		for(var i = 0; i<best_chain_cubes.length; ++i){
			if(best_chain_cubes[i] != null && best_chain_point[i]>c_point){
				c_point = best_chain_point[i];
				c_cube = best_chain_cubes[i];
			}
		}
		//讓AI進行連鎖的條件
		if(count_square_Num(map)>=20 && this.shooted == false && this.chaining == false && c_point>20){
			//進行連鎖
			if(c_cube != null){
				trace("**********shoot for chain***********");
				shooting(map,c_cube);
				this.chaining = true;
			}
		}
	}
	
	public function process_high(map: Map){
		var too_high_list: Array = too_high_travel(map);
		//讓AI進行消去過高列的條件
		if(too_high_list != null && this.shooted == false){
			//消去過高列
			trace("**********shoot too high column !!**********");
			for (var i =0; i<too_high_list.length; ++i){
				shooting_arr(map, too_high_list[i]);
			}
		}
	}
	
	public function process_amass(map: Map){
		//讓AI進行整理盤面的條件
		if(count_square_Num(map)>=20 && this.shooted == false && this.chaining == false){
			var amass_cube: Square = amass_travel_deep(map);
			if(amass_cube != null){
				trace("***********shoot for amass***********");
				shooting(map,amass_travel_deep(map));
			}
		}
	}
	
	public function process_rtc(map: Map){
		//讓AI進行動態連鎖的條件
		if(this.shooted == true && this.chaining == true){
			//即時連鎖
			var rtc_cube = rtc_checker(map);
			if(rtc_cube != null){
				trace("***********shoot for rtc_chain***********");
				shooting(map, rtc_cube);
			}			
		}
	}
	
	//暴搜初落下方塊找出會造成誤消去的方塊並打掉
	public function process_drop(map: Map){
		var del_arr = Dropping_checker(map);
		if(del_arr != null){
			trace_map_rgb(map);
			for(var del_i = 0; del_i < del_arr.length; ++del_i){
				trace("**********shoot_dropping(" + del_arr[del_i].x + "," + del_arr[del_i].y + ")**********");
				shooting(map,del_arr[del_i]);
				this.shooted = false;//射擊初落下方塊不影響盤面判斷
			}
		}
	}
	
	
	//一般完整流程
	public function process_mix(map: Map){
		update_ai_state(map);
		process_bg(map);
		//update_ai_state(map);
		process_chain(map);
		//update_ai_state(map);
		process_high(map);
		//update_ai_state(map);
		process_amass(map);
	}
	
	//檢查盤面是否在連鎖中or剛進行射擊過
	public function update_ai_state(map: Map){
		for(var h = map.Height-1; h > 0; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if(map.lookup(w, h).state instanceof Dropping && map.lookup(w, h).first_drop == false){
					this.shooted = true;
					break;
				}else{
					this.shooted = false;
				}
			}
			if(this.shooted == true){
				//trace("==========map is shooted!!==========");
				break;
			}
		}
		if(this.shooted == true && this.chaining == true){
			//trace("==========map is chaining!!==========");
			this.chaining = true;
		}else if(this.shooted == true){
			var temp_map = set_brain_map(map);
			failing_dropping(temp_map);
			if(contact_checker(temp_map,3)!=null){
				//trace("==========map is chaining!!==========");
				this.chaining = true;
			}
		}else if(this.shooted == false){
			this.chaining = false;
		}
	}
	
	//回傳複製盤面
	private function set_brain_map (map: Map): Map{
		var clone_map: Map = map.clone();
		return clone_map;
	}
	
	//射擊目標方塊,對最佳發火點分數進行更新
	private function shooting(map:Map, cube: Square){
		Player.change_player_ver2( map_.id() );
		this.shooted = true;
		trace("shoot (" + cube.x + "," +cube.y + ")");
		cube.i_am_hit(1);
		best_chain_point_update(map);
		if(map_.id() == 1){
			Player.change_player_ver2( 2 );
		}else if(map_.id() == 2){
			Player.change_player_ver2( 1 );
		}
	}
	
	//射擊目標方塊陣列,對最佳發火點分數進行更新
	private function shooting_arr(map: Map, s_list: Array){
		for(var i=0; i<s_list.length; ++i){
			Player.change_player_ver2( map_.id() );
			this.shooted = true;
			trace("shoot (" + s_list[i].x + "," + s_list[i].y + ")");
			s_list[i].i_am_hit(1);
			best_chain_point_update(map);
			if(map_.id() == 1){
				Player.change_player_ver2( 2 );
			}else if(map_.id() == 2){
				Player.change_player_ver2( 1 );
			}
		}
		best_chain_point_update(map);
	}
	
	//更新最佳的3個發火點
	public function combo_rank_update(map: Map){
		var get_point: Number = 0;
		for(var h = map.Height-1; h > this.travel_limit; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if(map.lookup(w,h)== null) continue;
				if( !(map.lookup(w,h).state instanceof Waiting) ) continue;
				if(map.lookup(w,h).if_broken() != true && map.lookup(w,h).if_garbage() != true){
					var temp_map: Map = set_brain_map(map);
					del_block(temp_map, w, h, true);
					get_point = combo_counter(temp_map);
					for(var i = best_chain_point.length; i >=0; --i){
						if(get_point > best_chain_point[i]){
							for(var j = 0; j < i; ++j){
								best_chain_point[j] = best_chain_point[j+1];
								best_chain_cubes[j] = best_chain_cubes[j+1];
							}
							best_chain_point[i] = get_point;
							best_chain_cubes[i] = map.lookup(w,h);
							break;
						}
					}
					delete temp_map;
				}
			}
		}
		for(var k = 0; k < this.best_chain_cubes.length; ++k){
			//trace("Best combo<" + best_chain_point[k] + ">(" + best_chain_cubes[k].x + "," + best_chain_cubes[k].y + ")");
		}
	}
	
	//最佳發火點分數進行更新
	private function best_chain_point_update(map: Map){
		for(var i = 0; i<this.best_chain_cubes.length; ++i){
			var temp_map: Map = set_brain_map(map);
			del_block(temp_map, this.best_chain_cubes[i].x, this.best_chain_cubes[i].y, true);
			this.best_chain_point[i] = combo_counter(temp_map);
		}
	}
	
	//計算map中的cube總數
	private function count_square_Num(map: Map): Number{
		var count:Number = 0;
		for(var h = map.Height-1; h > 0; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if( map.lookup(w,h).state instanceof Waiting ){
					++count;
				}
			}
		}
		return count;
	}
	
	//檢查盤面高度
	private function top_checker(map: Map): Array{
		var map_tops = new Array(map.Width+2);
		for(var i = 0; i < map.Width+2; i++){
			map_tops[i] = 0;
		}
		//map_tops[0~5] = every top in map, map_tops[6] = average, map_tops[7] = highest
		for(var h = map.Height-1; h > 0; --h){
			for(var w = map.Width-1; w >= 0; --w){
				if( map.lookup(w,h).state instanceof Waiting){
					map_tops[w] = map.Height-h;
				}
			}
		}
		for(var w = map.Width-1; w >= 0; --w){
			map_tops[map.Width-1+1] += map_tops[w];
			if(map_tops[w] > map_tops[map.Width-1+2]){
				map_tops[map.Width-1+2] = map_tops[w];
			}
		}
		map_tops[map.Width-1+1] = int(map_tops[map.Width-1+1]/map.Width);
		//trace("map_tops:" + map_tops);
		return map_tops;
	}
	
	//找即時連鎖發火點
	private function rtc_checker(map): Square{
		trace("==========start rtc travel==========");
		var temp_map = set_brain_map(map);
		var temp_map_2: Map = null;
		var true_failing_cubes: Array = new Array;
		var temp_failing_cubes: Array = new Array;
		var temp_failing_cubes_2: Array = new Array;
		var temp_chain_list: Array = new Array;
		var rtc_list: Array = new Array;
		var rtc_cube: Square = null;
		update_ai_state(map);
		//搜尋整個map,將2次落下方塊加入failing_cubes中
		for(var h = map.Height-1; h > 0; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if(map.lookup(w,h).if_broken() == true || map.lookup(w,h).if_garbage() == true) continue;
				if(map.lookup(w, h).state instanceof Dropping && map.lookup(w, h).first_drop != true){
					true_failing_cubes.push(map.lookup(w, h));
					temp_failing_cubes.push(temp_map.lookup(w,h));
				}
			}
		}
		if(true_failing_cubes == null){
			trace("==========no 2nd dropping cubes!!==========");
			this.shooted = false;
			this.chaining = false;
			return null;
		}
		failing_dropping(temp_map,false);
		if(contact_checker(temp_map,3)!=null){
			trace("==========map keep chaining!!==========");
			this.chaining = true;
			return null;
		}else{
			trace("==========now check rtc!!==========");
			for(var h_ = map.Height-1; h_ > 0; --h_){
				for(var w_ = map.Width-1; w_ >= 0 ; --w_){
					if(map.lookup(w_,h_).if_broken() == true || map.lookup(w_,h_).if_garbage() == true) continue;
					if(map.lookup(w_,h_)!= null && map.lookup(w_,h_).first_drop == false){
						temp_map_2 = set_brain_map(temp_map);
						temp_failing_cubes_2 = new Array;
						temp_chain_list = new Array;
						rtc_list = new Array;
						rtc_cube = null;
						for(var i = 0; i<temp_failing_cubes.length; ++i){
							temp_failing_cubes_2.push(temp_map_2.lookup(temp_failing_cubes[i].x,temp_failing_cubes[i].y));
						}
						for(var j = 0; j<temp_failing_cubes_2.length; ++j){
							if(temp_failing_cubes_2[j].x == w_ && temp_failing_cubes_2[j].y == h_){
								rtc_cube = true_failing_cubes[j];
								break;
							}else{
								rtc_cube = map.lookup(w_,h_);
							}
						}
						del_block(temp_map_2, w_, h_, true);
						for(var k = 0; k<temp_failing_cubes_2.length; ++k){
							if(temp_failing_cubes_2[k] == null) continue;
							if(temp_failing_cubes_2[k].x==w_ && temp_failing_cubes_2[k].y==h_) continue;
							rtc_list = contact_checker_unit(temp_map_2, temp_map_2.lookup(temp_failing_cubes_2[k].x,temp_failing_cubes_2[k].y), rtc_list, 3);
						
						}
						if(rtc_list.length !=0){
							trace("find rtc_cube(" + rtc_cube.x + "," + rtc_cube.y + ")");
							return rtc_cube;
						}
						delete temp_map_2;
					}
				}
			}
		}
		return null;
	}
	
	//消立柱相關,暴搜盤面,回傳過高列的目標可消去方塊
	private function too_high_travel(map: Map): Array{
		var map_tops: Array = top_checker(map);//get high of each column
		var del_list: Array = new Array(map.Width);//to save cubes that need delete
		for(var i = 0; i < del_list.length; ++i){
			del_list[i] = null;
		}
		for(var w = map.Width-1; w >= 0; --w){
			//判定此列過高的條件
			if(map_tops[w] > map_tops[map.Width-1+1] + 3 || map_tops[w] > 6){
				trace("==========column(" + w + ") is too high==========");
				del_list[w] = column_travel_type1(map, w);
			}
		}
		for(var j = 0; j < del_list.length; ++j){
			if(del_list[j]!=null){
				return del_list;
			}
		}
		return null;
	}
	
	//消立柱相關,搜尋此列的一組22相鄰方塊 找出第3個方塊 將中間的方塊s回傳
	private function column_travel_type1(map: Map, w: Number): Array{
		var del_list: Array = new Array;
		for(var h = map.Height-1; h > 0; --h){
			if(map.lookup(w,h).if_broken() == true || map.lookup(w,h).if_garbage() == true) continue;
			if(map.lookup(w,h)!= null && map.lookup(w,h).state instanceof Waiting){
				if(map.make_column(map.lookup(w,h)).length >= 1){
					var del_list_up: Array = new Array;
					var del_list_down: Array = new Array;
					for(var ph = 1; ph < 6; ++ph){
						//ph為與前一方塊距離, 因此可設定搜尋極限(目前為6)
						if(map.lookup(w,h+ph).rgb == map.lookup(w,h).rgb && map.lookup(w,h+ph).state instanceof Waiting && h+ph < 11){
							del_list = del_list_down;
							break;
						}else if(map.lookup(w,h+ph).state instanceof Waiting && h+ph < 11){
							del_list_down.push(map.lookup(w,h + ph));
						}
						if(map.lookup(w,h-1-ph).rgb == map.lookup(w,h).rgb && map.lookup(w,h-1-ph).state instanceof Waiting && h-1-ph > 0){
							del_list = del_list_up;
							break;
						}else if(map.lookup(w,h-1-ph).state instanceof Waiting && h-1-ph > 0){
							del_list_up.push(map.lookup(w,h-1-ph));
						}
					}
					if(del_list.length != 0){
						break;
					}
					--h;
				}
			}
		}
		if(del_list.length != 0){
			return del_list;
		}else{
			return column_travel_type2(map, w);
		}
	}
	
	//消立柱相關,以此列一個方塊發跡向上找同色,找到後再往上找另一個同色 將中間的方塊s回傳
	private function column_travel_type2(map: Map, w: Number): Array{
		var del_list: Array = new Array;
		var color_list: Array = new Array;
		for(var h = map.Height-1; h > 0; --h){
			if(map.lookup(w,h).if_broken() == true || map.lookup(w,h).if_garbage() == true) continue;
			if(map.lookup(w,h)!= null && map.lookup(w,h).state instanceof Waiting){
				del_list = new Array;
				color_list = new Array;
				color_list.push(map.lookup(w,h));
				del_list = column_color_checker(map, del_list, color_list);
			}
			if(del_list!=null){
				return del_list;
			}
		}
		return column_travel_type3(map, w);
	}
	
	//接續column_travel_type2
	private function column_color_checker(map: Map, del_list: Array, color_list: Array): Array{
		if(color_list.length > 0){
			for(var ph = 1; ph <=3 && color_list[color_list.length-1].y-ph > 0 && color_list.length < 3; ++ph){
				//ph為與前一方塊距離, 因此可設定搜尋極限(目前為6)
				if( !(map.lookup(color_list[color_list.length-1].x,color_list[color_list.length-1].y-ph).state instanceof Waiting) ) continue;
				if(map.lookup(color_list[color_list.length-1].x,color_list[color_list.length-1].y-ph).rgb == color_list[color_list.length-1].rgb){
					color_list.push(map.lookup(color_list[color_list.length-1].x,color_list[color_list.length-1].y-ph));
					del_list = column_color_checker(map, del_list, color_list);
				}else{
					del_list.push(map.lookup(color_list[color_list.length-1].x,color_list[color_list.length-1].y-ph));
				}
			}
			if(color_list.length >= 3){
				return del_list;
			}else{
				return null;
			}
		}else{
			return null;
		}
	}
	
	//消立柱相關,直接打掉最高的兩顆方塊....orz
	private function column_travel_type3(map: Map, w: Number): Array{
		var del_list: Array = new Array;
		for(var h = map.Height-1; h > 0; --h){
			if(map.lookup(w,h) == null && map.lookup(w,h+1).state instanceof Waiting){
				del_list.push(map.lookup(w,h+1));
				del_list.push(map.lookup(w,h+2));
				break;
			}
		}
		if(del_list.length>0){
			return del_list;
		}else{
			return null;
		}
	}
	
	//暴搜盤面,找出broken或是garbage方塊並回傳
	private function bk_gbg_travel(map: Map): Array{
		var bg_cubes: Array = new Array(2);
		bg_cubes[0] = new Array;//儲存broken_cubes
		bg_cubes[1] = new Array;//儲存garbage_cubes
		for(var h = map.Height-1; h > 0; --h){
			for(var w = map.Width-1; w >= 0; --w){
				if(map.lookup(w,h).state instanceof Waiting && map.lookup(w,h).if_broken() == true){
					bg_cubes[0].push(map.lookup(w,h));
				}else if(map.lookup(w,h).state instanceof Waiting && map.lookup(w,h).if_garbage() == true){
					bg_cubes[1].push(map.lookup(w,h));
				}
			}
		}
		if(bg_cubes[0].length!=0 || bg_cubes[1].length!=0){
			return bg_cubes;
		}else{
			return null;
		}
	}
		
	//暴搜盤面找出最佳發火點並回傳
	private function combo_travel(map: Map): Square{
		var combo_point: Number = 0;
		var best_cube: Square = null;
		for(var h = map.Height-1; h > this.travel_limit; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if(map.lookup(w,h)!= null && map.lookup(w,h).state instanceof Waiting){
					var temp_map: Map = set_brain_map(map);
					del_block(temp_map, w, h, true);
					if(combo_counter(temp_map)>combo_point){
						combo_point = combo_counter(temp_map);
						best_cube = map.lookup(w,h);
					}
					delete temp_map;
				}
			}
		}
		return best_cube;
	}
	
	//暴搜盤面找出最佳排列點並回傳
	private function amass_travel(map: Map): Square{
		var get_point: Number = 0;
		var best_cube: Square = null;
		var amass_point = amass_counter(map);
		for(var h = map.Height-1; h > this.travel_limit; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if(map.lookup(w,h)!= null && map.lookup(w,h).state instanceof Waiting){
					var temp_map: Map = set_brain_map(map);
					del_block(temp_map, w, h, true);
					get_point = amass_counter(temp_map);
					if(get_point > amass_point){
						amass_point = get_point;
						best_cube = map.lookup(w,h);
					}
					delete temp_map;
				}
			}
		}
		return best_cube;
	}
	
	//深層暴搜盤面找出最佳排列點並回傳
	private function amass_travel_deep(map: Map): Square{
		var best_cube: Square = null;//最佳排列點方塊
		var in_best_cubes: Boolean = false;
		var best_point: Number = 0;
		best_point = amass_deep_formula(amass_counter(map),this.best_chain_point);
		for(var h = map.Height-1; h > this.travel_limit; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				for(var i = this.best_chain_cubes.length; i >=0; --i){
					if(w == this.best_chain_cubes[a].x && h == this.best_chain_cubes[a].y){
						in_best_cubes = true;
						break;
					}
				}
				if(map.lookup(w,h)!= null && map.lookup(w,h).state instanceof Waiting && in_best_cubes != true){
					var temp_map: Map = set_brain_map(map);//temp_map為消去被評價點(w,h)之後的map
					var temp_best_chain_cubes: Array = [null, null, null];//暫存最佳發火點,以防消去被評價點後造成最佳發火點位址變動
					for(var a = 0; a < this.best_chain_cubes.length; ++a){
						temp_best_chain_cubes[a]=temp_map.lookup(this.best_chain_cubes[a].x,this.best_chain_cubes[a].y);
					}
					del_block(temp_map, w, h, true);//消去被評價點(w,h)
					var chain_point: Array = [0,0,0];
					for(var j = this.best_chain_cubes.length; j >=0; --j){
						if(temp_best_chain_cubes[j]!=null){
							var temp_map_2: Map = set_brain_map(temp_map);
							del_block(temp_map_2,temp_best_chain_cubes[j].x,temp_best_chain_cubes[j].y,true);
							chain_point[j] = combo_counter(temp_map_2);
							delete temp_map_2;
						}
					}
					if(amass_deep_formula(amass_counter(temp_map),chain_point) > best_point){
						//trace("find best amass(" + w + "," + h + ")");
						best_point = amass_deep_formula(amass_counter(temp_map),chain_point);
						best_cube = map.lookup(w,h);
					}
					delete temp_map;
				}
				in_best_cubes = false;
			}
		}
		return best_cube;
	}
	
	//深層暴搜盤面最佳排列點計算公式(這裡是打掉此點後的22相鄰方塊數+前3名發火點更新後的分數)
	private function amass_deep_formula(amass_point: Number,chain_points: Array): Number{
		var amass_deep_point = amass_point;
		for(var i = 0; i < chain_points.length; ++i){
			amass_deep_point += chain_points[i];
		}
		return amass_deep_point;
	}
	
	//計算此盤面的兩兩相臨方塊個數 並回傳
	private function amass_counter(map: Map): Number{
		if(contact_checker(map, 3) == null){
			var get_point: Number = 0;
			get_point = contact_checker(map, 2).length;
			return get_point;
		}
		return 0;
	}
	
	//計算此盤面的連鎖分數並回傳
	private function combo_counter(map: Map): Number{
		var square_count: Array = new Array();
		var chain_count: Number = 0;
		var get_point: Number = 0;
		while(contact_checker(map,3) != null){
			//use Array to save contace_checker
			var chain_arr: Array = new Array();
			chain_arr = contact_checker(map,3);
			++chain_count;
			square_count.push(chain_arr.length);
			chain_del(map, chain_arr);
		}
		//point_formula : (del_square_in_each_chain-3)*each_chain_Num+2^(each_chain_Num-2)
		for(var i = 0; i < square_count.length; ++i){
			get_point += (square_count[i]-3)*i+1;
			if( i == 0 ){
				get_point += (square_count[i]-3)*(i+1);
			}else{
				get_point += (square_count[i]-3)*(i+1) + Math.pow(2, i-1);
			}
		}
		return get_point;
	}
	
	//暴搜盤面,找出兩兩相鄰或是可連鎖消去的方塊並回傳,以contact_Num決定有幾個同色相連才要回傳
	private function contact_checker(map: Map, contact_Num: Number): Array{
		var cube_list: Array = new Array();
		for(var h = map.Height-1; h > 0; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if(map.lookup(w,h).if_broken() == true || map.lookup(w,h).if_garbage() == true) continue;
				cube_list = contact_checker_unit(map, map.lookup(w,h), cube_list, contact_Num);
			}
		}
		for(var re_cy = 0; re_cy < cube_list.length; ++re_cy){
			cube_list[re_cy].cycled = false;
		}
		if(cube_list.length == 0){
			return null;
		}else{
			return cube_list;
		}
	}
	
	//接續contact_checker
	private function contact_checker_unit(map: Map, cube: Square, list: Array, contact_Num: Number): Array{
		var temp_list: Array = new Array;
		if(map.make_row(cube).length >= contact_Num-1){
			temp_list = map.make_row( cube );
			temp_list.push( cube );
			for(var push_row = 0; push_row < temp_list.length; ++push_row){
				if(temp_list[push_row].cycled == false){
					temp_list[push_row].cycled = true;
					list.push(temp_list[push_row]);
				}
			}
		}
		if(map.make_column(cube).length >= contact_Num-1){
			temp_list = map.make_column( cube );
			temp_list.push( cube );
			for(var push_col = 0; push_col < temp_list.length; ++push_col){
				if(temp_list[push_col].cycled == false){
					temp_list[push_col].cycled = true;
					list.push(temp_list[push_col]);
				}
			}
		}
		return list;
	}
	
	//檢查初落下方塊是否會造成誤消去,有則回傳這些方塊s
	private function Dropping_checker(map: Map): Array{
		var temp_map: Map = set_brain_map(map);
		var check_arr: Array = new Array();
		var dropping_arr: Array = new Array();
		var del_arr: Array = new Array();
		for(var h = map.Height-1; h > 0; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if(map.lookup(w,h).if_broken() == true || map.lookup(w,h).if_garbage() == true) continue;
				if(map.lookup(w, h).state instanceof Dropping && map.lookup(w, h).first_drop == true){
					dropping_arr.push(map.lookup(w, h));
					check_arr.push(temp_map.lookup(w,h));
				}
			}
		}
		failing_dropping(temp_map,true);
		if(contact_checker(temp_map, 3) == null){
			delete temp_map;
			return null;
		}else{
			for(var check_i = 0; check_i < check_arr.length; ++check_i){
				if(temp_map.make_row(check_arr[check_i]).length >= 2){
					del_arr.push(dropping_arr[check_i]);
				}else if(temp_map.make_column(check_arr[check_i]).length >= 2){
					del_arr.push(dropping_arr[check_i]);
				}
			}
			delete temp_map;
			return del_arr;
		}
	}
	
	//虛擬盤面操作
	//傳入一cube array,先消去這些cube,再將消去後造成的空格填滿
	private function chain_del(map: Map, del_arr: Array){
		//delete cubes
		for(var cube_Num = 0; cube_Num < del_arr.length; ++cube_Num){
			del_block(map, del_arr[cube_Num].x, del_arr[cube_Num].y, false);
		}
		//填補空格
		for(var h = map.Height-1; h > 0; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if(map.lookup(w, h) == null){
					move_block(map, w, h, false);
				}
			}
		}
	}
	
	//虛擬盤面操作
	//消去map中位於(x,y)的cube,由need_move傳入之值決定是否要將消去後造成的空格填起
	private function del_block(map: Map, x: Number, y: Number, need_move: Boolean){
		var del_x = x;
		var del_y = y;
		map[del_y][del_x].suicide();
		map.delData(del_x, del_y);
		if(need_move == true){
			move_block(map, del_x, del_y, false);
		}
	}
	
	//虛擬盤面操作
	//傳入map中之空格座標(x,y),向(x,y)上方進行搜尋,將(x,y)上方之方塊強制下移填補空格
	private function move_block(map: Map, x: Number, y: Number){
		var locx = x;
		var locy = y;
		for(var target_y = locy-1;
			target_y > 0 &&
			map.lookup(locx,locy) == null;
			--target_y){
			if(map.lookup(locx,target_y) != null){
				if(map.lookup(locx,target_y).state instanceof Waiting){
					map.setup(locx, locy, map.lookup(locx, target_y));
					map.lookup(locx, locy).y = locy;
					map.lookup(locx, locy).state = new Waiting(map);
					//trace("move dorpping cube(" + locx + "," + target_y + ") to (" + locx + "," + locy + ")");
					del_block(map, locx, target_y, true);
				}
			}
		}
	}
	
	//虛擬盤面操作
	//將落下中方塊強制落下,由first_drop決定目標是否為初落下方塊
	private function failing_dropping(map: Map, first_dropping: Boolean){
		for(var h = map.Height-1; h > 0; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if( map.lookup(w,h) == null) continue;
				if( !(map.lookup(w,h).state instanceof Dropping) ) continue;
				
				if( map.lookup(w,h).first_drop == true && first_dropping == true ){
					for(var move_to = h+1; move_to < map.Height && map.lookup(w,move_to) == null; ++move_to){
						map.setup(w, move_to, map.lookup(w, h));
						map.lookup(w, move_to).y = move_to;
						del_block(map, w, h, false);
						if(move_to+1 == map.Height || map.lookup(w,move_to+1) != null){
							map.lookup(w, move_to).state = new Waiting(map);
						}
					}
				}else if( map.lookup(w,h).first_drop == false && first_dropping == false ){
					for(var move_to = h; move_to < map.Height; ++move_to){
						if( map.lookup(w,move_to+1) != null ){
							map.setup(w, move_to, map.lookup(w, h));
							map.lookup(w, move_to).y = move_to;
							if( h != move_to ) 
							    del_block(map, w, h, false);
							map.lookup(w, move_to).state = new Waiting(map);
							break;
						}
					}
				}
			}
		}
	}
	
	//trace盤面資訊 state
	private function trace_map_state(map: Map){
		var state_arr: Array = new Array(map.Height);
        for(var i = 0; i < map.Height; ++i){
			state_arr[i] = new Array(map.Width);
		}
		for(var h = map.Height-1; h >= 0; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if( map.lookup(w,h).state instanceof Waiting ){
					var waiting: Waiting = Waiting(map.lookup(w,h).state);
					if( waiting.state_checked() == true){
						state_arr[h][w] = "WT";
					}else if( waiting.state_checked() == false){
						state_arr[h][w] = "WF";
					}else{
						state_arr[h][w] = "Wa";
					}
				}else if ( map.lookup(w,h).state instanceof Dropping ){
					if(map.lookup(w,h).first_drop == true){
						state_arr[h][w] = "DT";
					}else if(map.lookup(w,h).first_drop == false){
						state_arr[h][w] = "DF";
					}else{
						state_arr[h][w] = "Dr";
					}
				}else if ( map.lookup(w,h).state instanceof Dying )
				    state_arr[h][w] = "Dy";
				else
				    state_arr[h][w] = "--";
			}
		}
		trace("map_state:");
		for(var a = 0; a < map.Height; ++a){
			trace(state_arr[a]);
		}
	}
	
	//trace盤面資訊 RGB
	private function trace_map_rgb(map: Map){
		var rgb_arr: Array = new Array(map.Height);
        for(var i = 0; i < map.Height; ++i){
			rgb_arr[i] = new Array(map.Width);
		}
		for(var h = map.Height-1; h >= 0; --h){
			for(var w = map.Width-1; w >= 0 ; --w){
				if( map.lookup(w,h).rgb == 255 << 16 )
				    rgb_arr[h][w] = "Re";
				else if ( map.lookup(w,h).rgb == 255 << 8 )
				    rgb_arr[h][w] = "Gr";
				else if ( map.lookup(w,h).rgb == 255 )
				    rgb_arr[h][w] = "Bl";
				else if ( map.lookup(w,h).rgb == (255<<16) + (255<<8) )
				    rgb_arr[h][w] = "Ye";
				else
				    rgb_arr[h][w] = "--";
			}
		}
		trace("map_RGB:");
		for(var a = 0; a < map.Height; ++a){
			trace(rgb_arr[a]);
		}
	}
	
	private var travel_limit: Number;
	private var best_chain_point: Array;
	private var best_chain_cubes: Array;
	private var shooted: Boolean;
	private var chaining: Boolean;
	private var game_: Game;
	private var map_: Map;
	private var player_n: Number;
}