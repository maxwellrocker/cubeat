
local ffi      = require 'ffi'
local C        = ffi.C

local view     = require 'rc/script/ui/test/view'

-------------------- scripts above this line should be separated -----------------
----------------------------------------------------------------------------------

local scene_, sp

function init(self)
  self    = ffi.cast("TestUI*", self);
  scene_  = self:get_ui_scene()
  sp      = view.new_sprite("area_rect", scene_, 64, 64, true)
  sp:set_texture("cubes/cube-b-1")
  sp:set_pos(640, 360)
  sp:set_scale(1.5, 1.5, 1)
  sp:set_visible(true)
  sp:set_center_aligned(false)
  sp:move_to(100, 100)
  sp:move_tween(0,0,5000)
  sp:texture_flipH()
  sp:texture_flipV()
  sp:set_size(32, 32)
  
  sp:on_release(C.Input_get_trig1(C.Input_get_input1()), function(self)
    print 'hi' 
    self:on_release(C.Input_get_trig1(C.Input_get_input1()), function(self)
      print 'hello'
    end)
  end)  
end

local select_char_bg_,
      select_char_title_,
      select_char_img_,
      select_char_ok_
local select_char_btn = {}

function init_select_char_panel(self)
  select_char_bg_ = view.new_sprite("area_rect", scene_, 600, 720, false)
  select_char_bg_:set_pos( 300, 0)
  select_char_bg_:set_depth(1000)
  
  select_char_title_ = view.new_sprite_text_from_sprite("Please select character",
                                                        select_char_bg_,
                                                        "Star Jedi",
                                                        24,
                                                        true,
                                                        255, 255, 255)
  select_char_title_:set_pos(300, 60)
  select_char_title_:set_depth(1000)
  
  select_char_img_ = view.new_sprite_from_sprite("char1/full", select_char_bg_, 200, 600, false)
  select_char_img_:set_pos(40, 100)
  select_char_img_:set_depth(1000)
  
  select_char_ok_ = view.new_sprite_text_from_sprite("ok", select_char_bg_, "Star Jedi", 24, false, 255, 255, 255)
  select_char_ok_:set_pos(340, 400)
  select_char_ok_:on_enter_focus( C.Input_get_input1(), function(self, x, y) select_char_ok_:set_red(100) end )
  select_char_ok_:on_leave_focus( C.Input_get_input1(), function(self, x, y) select_char_ok_:set_red(255) end )
  select_char_ok_:set_depth(-100)
  
  for i=1,5 do
    select_char_btn[i] = view.new_sprite_text_from_sprite("charactor"..i, select_char_bg_, "Star Jedi", 24, false, 255, 255, 255)
    select_char_btn[i]:set_pos(340, 100+i*40)
    select_char_btn[i]:set_depth(-100)
  end
  
  local function sel_char1(self) select_char_img_:set_texture("char1/full") print("push") end
  local function sel_char2(self) select_char_img_:set_texture("char2/full") end
  local function sel_char3(self) select_char_img_:set_texture("char3/full") end
  local function sel_char4(self) select_char_img_:set_texture("char4/full") end
  local function sel_char5(self) select_char_img_:set_texture("char5/full") end
  select_char_btn[1]:on_press( C.Input_get_trig1(C.Input_get_input1()), sel_char1 )
  select_char_btn[2]:on_press( C.Input_get_trig1(C.Input_get_input1()), sel_char2 )
  select_char_btn[3]:on_press( C.Input_get_trig1(C.Input_get_input1()), sel_char3 )
  select_char_btn[4]:on_press( C.Input_get_trig1(C.Input_get_input1()), sel_char4 )
  select_char_btn[5]:on_press( C.Input_get_trig1(C.Input_get_input1()), sel_char5 )
  
  local btn_focus_in  = {}
  local btn_focus_out = {}
  for i=1,5 do
    btn_focus_in[i]   = function(self, x, y) select_char_btn[i]:set_red(100) end
    btn_focus_out[i]  = function(self, x, y) select_char_btn[i]:set_red(255) end
    select_char_btn[i]:on_enter_focus( C.Input_get_input1(), btn_focus_in[i] )
    select_char_btn[i]:on_leave_focus( C.Input_get_input1(), btn_focus_out[i] )
  end
  
  select_char_bg_:set_visible(false)
end

local start_game_title_

function init_game_title(self)
  start_game_title_ = view.new_sprite_text("Select Charactor", scene_, "Star Jedi", 36, true, 255, 255, 255)
  start_game_title_:set_pos(688, 480)
  
  local title_focus_in  = function(self, x, y) start_game_title_:set_red(100) end
  local title_focus_out = function(self, x, y) start_game_title_:set_red(255) end
  local title_press     = function(self)
                            select_char_bg_:set_visible(true)
                            start_game_title_:set_visible(false)
                          end
  start_game_title_:on_enter_focus( C.Input_get_input1(), title_focus_in )
  start_game_title_:on_leave_focus( C.Input_get_input1(), title_focus_out)
  start_game_title_:on_press( C.Input_get_trig1(C.Input_get_input1()), title_press )
end

local panel1
local button1 = {}
local button2 = {}
local button3 = {}
local button4 = {}
local panel2,
      back_button
local ratio1 = {}
local ratio2 = {}
local arrow_left,
      arrow_right,
      arrow_title,
      arrow_debug_text
local scrollbar_front,
      scrollbar_back,
      scrollbar_title
local title_tb = {"TITLE1", "TITLE2", "TITLE3"}
local title_index = 1
local scrollbar_on = false
local scrollbar_on_title
local test_ui_ratio = {}
local test_ui_button = {}

function init_test_menu(self)
  --=======================INIT PANEL1=======================--
  panel1 = view.new_sprite("area_rect", scene_, 256, 256, true)
  panel1:set_pos(688, 384)
  button1 = view.new_ui_button(button1, "button1", panel1)
  button2 = view.new_ui_button(button2, "button2", panel1)
  button3 = view.new_ui_button(button3, "button3", panel1)
  button4 = view.new_ui_button(button4, "button4", panel1)
  button1:set_pos(-100, -100)
  button2:set_pos(-100, -50)
  button3:set_pos(-100, 0)
  button4:set_pos(-100, 50)
  
  --=======================INIT PANEL2=======================--
  panel2  = view.new_sprite("area_rect", scene_, 512, 512, true)
  panel2:set_pos(688, 384)
  panel2:set_alpha(0)
  --
  back_button = view.new_sprite_text_from_sprite("BACK", panel2, "Star Jedi", 24, false, 255, 255, 0)
  back_button:set_pos(-200, 150)
  back_button:set_alpha(0)
  back_button:set_depth(-1)
  --
  ratio1  = view.new_ui_ratio(ratio1, "ratio1", panel2)
  ratio2  = view.new_ui_ratio(ratio2, "ratio2", panel2)
  ratio1:set_pos(-200, -200)
  ratio2:set_pos(-200, -150)
  ratio1:set_alpha(0)
  ratio2:set_alpha(0)
  local ratio1_press = function(self) end
  local ratio2_press = function(self) end
  ratio1:on_press(ratio1_press)
  ratio2:on_press(ratio2_press)
  --
  arrow_left        = view.new_sprite_from_sprite("cubes/cube-b-1", panel2, 32, 32, false)
  arrow_right       = view.new_sprite_from_sprite("cubes/cube-b-1", panel2, 32, 32, false)
  arrow_title       = view.new_sprite_text_from_sprite(title_tb[title_index], panel2, "Star Jedi", 24, true, 255, 255, 0)
  arrow_debug_text  = view.new_sprite_text_from_sprite(tostring(title_index), panel2, "Star Jedi", 24, true, 100, 100, 255)
  scrollbar_back    = view.new_sprite_from_sprite("cubes/cube1", panel2, 256, 16, false)
  scrollbar_front   = view.new_sprite_from_sprite("cubes/cube-b-1", panel2, 32, 32, false)
  scrollbar_title   = view.new_sprite_text_from_sprite("0", panel2, "Star Jedi", 24, true, 255, 255, 0)
  scrollbar_on_title= view.new_sprite_text_from_sprite("off", panel2, "Star Jedi", 24, true, 100, 100, 255)
  --
  arrow_left:set_pos(-200, -100)
  arrow_right:set_pos(80, -100)
  arrow_title:set_pos(-50, -90)
  arrow_debug_text:set_pos(170, -90)
  arrow_left:set_alpha(0)
  arrow_right:set_alpha(0)
  arrow_title:set_alpha(0)
  arrow_debug_text:set_alpha(0)
  arrow_left:set_depth(-1)
  arrow_right:set_depth(-1)
  --
  scrollbar_back:set_pos(-200, -40)
  scrollbar_front:set_pos(-200, -50)
  scrollbar_title:set_pos(100, -40)
  scrollbar_on_title:set_pos(170, -40)
  scrollbar_back:set_alpha(0)
  scrollbar_front:set_alpha(0)
  scrollbar_title:set_alpha(0)
  scrollbar_on_title:set_alpha(0)
  scrollbar_front:set_depth(-100)
  
  --=======================LEFT&RIGHT ARROW=======================--
  local arrow_left_press = function(self)
    title_index = title_index - 1
    if title_index < 1 then title_index = table.getn(title_tb) end
    arrow_title:change_text(title_tb[title_index])
    arrow_debug_text:change_text(tostring(title_index))
  end
  local arrow_right_press = function(self)
    title_index = title_index + 1
    if title_index > table.getn(title_tb) then title_index = 1 end
    arrow_title:change_text(title_tb[title_index])
    arrow_debug_text:change_text(tostring(title_index))
  end
  arrow_left:on_press( C.Input_get_trig1(C.Input_get_input1()), arrow_left_press )
  arrow_right:on_press( C.Input_get_trig1(C.Input_get_input1()), arrow_right_press )
  --
  local arrow_left_focus = function(self) arrow_left:set_blue(0) end
  local arrow_left_leave = function(self) arrow_left:set_blue(255) end
  local arrow_right_focus = function(self) arrow_right:set_blue(0) end
  local arrow_right_leave = function(self) arrow_right:set_blue(255) end
  arrow_left:on_enter_focus( C.Input_get_input1(), arrow_left_focus )
  arrow_left:on_leave_focus( C.Input_get_input1(), arrow_left_leave )
  arrow_right:on_enter_focus( C.Input_get_input1(), arrow_right_focus )
  arrow_right:on_leave_focus( C.Input_get_input1(), arrow_right_leave )
  
  --=======================SCROLLBAR=======================--
  local scrollbar_front_press   = function(self)
    scrollbar_on = true
    scrollbar_on_title:change_text("on")
  end
  local scrollbar_front_release = function(self)
    scrollbar_on = false
    scrollbar_on_title:change_text("off")
  end
  scrollbar_front:on_press( C.Input_get_trig1(C.Input_get_input1()), scrollbar_front_press )
  scrollbar_front:on_release( C.Input_get_trig1(C.Input_get_input1()), scrollbar_front_release )
  --
  local scrollbar_front_focus = function(self) scrollbar_front:set_blue(0) end
  local scrollbar_front_leave = function(self)
                                  scrollbar_front:set_blue(255)
                                  scrollbar_on = false
                                  scrollbar_on_title:change_text("off")
                                end
  scrollbar_front:on_enter_focus( C.Input_get_input1(), scrollbar_front_focus )
  scrollbar_front:on_leave_focus( C.Input_get_input1(), scrollbar_front_leave )
  --
  local scrollbar_front_down  = function(self)
    if scrollbar_on == true then
      local pos_x = C.Input_get_cursor_x(C.Input_get_input1()) - 704
      local pos_y = scrollbar_front:get_pos_y()
      local bg_left = scrollbar_back:get_pos_x()
      local bg_right= bg_left + scrollbar_back:get_size_x() - 32
      if pos_x < bg_left then pos_x = bg_left end
      if pos_x > bg_right then pos_x = bg_right end
      scrollbar_front:set_pos(pos_x, pos_y)
      --
      local scroll_value = math.floor( (pos_x-bg_left)*100/(scrollbar_back:get_size_x()-32) )
      scrollbar_title:change_text(tostring(scroll_value))
    end
  end
  scrollbar_front:on_down( C.Input_get_trig1(C.Input_get_input1()), scrollbar_front_down )
  
  --=======================BACK BUTTON=======================--
  local tween_cb =  function(self) end
  local back_btn_press =  function(self)
                            panel1:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                            button1:set_fade(255)
                            button2:set_fade(255)
                            button3:set_fade(255)
                            button4:set_fade(255)
                            panel2:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                            ratio1:set_fade(0)
                            ratio2:set_fade(0)
                            arrow_left:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                            arrow_right:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                            arrow_title:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                            arrow_debug_text:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                            scrollbar_back:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                            scrollbar_front:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                            scrollbar_title:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                            scrollbar_on_title:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                            back_button:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                          end
  back_button:on_press( C.Input_get_trig1(C.Input_get_input1()), back_btn_press )
  --
  local back_btn_focus  = function(self)back_button:set_blue(255) end
  local back_btn_leave  = function(self) back_button:set_blue(0) end
  back_button:on_enter_focus( C.Input_get_input1(), back_btn_focus )
  back_button:on_leave_focus( C.Input_get_input1(), back_btn_leave )
  
  --=======================PANEL1 BUTTON2=======================--
  local tween_cb =  function(self) end
  local button2_press = function(self)
                          panel1:on_tween_line_alpha(0, 500, 0, tween_cb, 0)
                          button1:set_fade(0)
                          button2:set_fade(0)
                          button3:set_fade(0)
                          button4:set_fade(0)
                          panel2:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                          ratio1:set_fade(255)
                          ratio2:set_fade(255)
                          arrow_left:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                          arrow_right:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                          arrow_title:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                          arrow_debug_text:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                          scrollbar_back:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                          scrollbar_front:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                          scrollbar_title:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                          scrollbar_on_title:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                          back_button:on_tween_line_alpha(255, 500, 0, tween_cb, 0)
                        end
  button2:on_press( button2_press )
end

local test1,
      test2,
      test3
      
function init_test_sprite(self)
  test1 = view.new_sprite("cubes/cube1", scene_, 64, 64, true)
  test1:set_pos(688, 384)
  test2 = view.new_sprite("cubes/cube1", scene_, 64, 64, true)
  test2:set_pos(840, 384)
  local tween_cb =  function(self)
                      test2:set_red(100)
                    end
  local test1_on_press =  function(self)
                            --test2:on_tween_line_pos(940, 384, 4000, 0, tween_cb, 500)
                            --test2:on_tween_line_rotation(0, 45, 0, 4000, 0, tween_cb, 500)
                            --test2:on_tween_line_scale(2, 1, 0, 4000, 0, tween_cb, 500)
                            --test2:on_tween_line_color_diffuse(50, 4000, 0, tween_cb, 500)
                            --test2:on_tween_line_blue(50, 4000, 0, tween_cb, 500)
                            test2:on_tween_line_alpha(50, 4000, 0, tween_cb, 500)
                          end
  test1:on_press( C.Input_get_trig1(C.Input_get_input1()), test1_on_press )
  
  test3 = view.new_sprite_text("click", scene_, "Star Jedi", 24, true, 255, 255, 255)
  test3:set_pos(688, 324)
end

function cycle(self)
  self = ffi.cast("TestUI*", self);
end

function destroy(self)
  self = ffi.cast("TestUI*", self);
end
