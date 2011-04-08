
#include "presenter/game/Puzzle.hpp"
#include "view/Scene.hpp"
#include "view/AnimatedSprite.hpp"
#include "view/SpriteText.hpp"
#include "view/Menu.hpp"
#include "Accessors.hpp"
#include "EasingEquations.hpp"

#include "presenter/Stage.hpp"
#include "presenter/PlayerView.hpp"
#include "presenter/Map.hpp"
#include "presenter/cube/ViewSprite.hpp"

#include "EventDispatcher.hpp"
#include "Input.hpp"
#include "Player.hpp"
#include "Weapon.hpp"
#include "audio/Sound.hpp"
#include "Conf.hpp"
#include "App.hpp"

#include "utils/Random.hpp"
#include "utils/dictionary.hpp"
#include "utils/MapLoader.hpp"
#include "utils/to_s.hpp"
#include "utils/Logger.hpp"
#include <boost/foreach.hpp>

using namespace psc;
using namespace presenter;
using namespace game;
using namespace easing;
using namespace accessor;
using utils::to_s;
using namespace std::tr1::placeholders;

Puzzle::Puzzle()
{
}

Puzzle::~Puzzle()
{
    std::cout << "Puzzle Game destructing ..." << std::endl;
    std::cout << " player0 use count: " << player0_.use_count() << std::endl;
}

pPuzzle Puzzle::init(std::string const& c1p, std::string const& sc, int puzzle_level)
{
    //App::i().setLoading(1);
    scene_ = psc::view::Scene::create("random Puzzle game");
    //scene_->setTo2DView().enableGlobalHittingEvent(); //2011.03.28 weapon temporary removal
    scene_->setTo2DView();

    c1p_ = c1p; sconf_ = sc; puzzle_level_ = puzzle_level;

    data::pViewSetting s0, s1;

    s0 = data::ViewSetting::create(64);   //must use config
    s0->x_offset(159).y_offset(684);
    s1 = data::ViewSetting::create(64);   //must use config
    s1->x_offset(740).y_offset(684);

    ///THIS IS IMPORTANT, ALL PLAYERS MUST BE DEFINED FIRST.
    player0_ = ctrl::Player::create(ctrl::InputMgr::i().getInputByIndex(0), 0, false);
    player0_->weapon(0)->ammo(0);
    player0_->weapon(1)->ammo(0);
    player0_->weapon(2)->ammo(0);
    player0_->push_ally(0);

    // setup map0
    map0_ = utils::MapLoader::generate( puzzle_level );
    map0_->set_view_master( presenter::cube::ViewSpriteMaster::create(scene_, s0, player0_) );

    // setup map1
    data::pMapSetting set1 = data::MapSetting::create();
    set1->starting_line(0).dropping_creatable(false);
    map1_ = presenter::Map::create(set1);
    map1_->set_view_master( presenter::cube::ViewSpriteMaster::create(scene_, s1) );

    ///NEW: MAKE PLAYER KNOWS ABOUT MAP
    std::vector< presenter::wpMap > map_list;
    map_list.push_back(map0_);
    player0_->setMapList( map_list );

    // setup garbage land
    map0_->push_garbage_land(map1_);
    map1_->push_garbage_land(map0_);
    //map0_->set_endgame(bind(&Puzzle::end, this, _1)); no endgame right now.. testing

    // setup stage & ui & player's view objects:
    stage_ = presenter::Stage::create( sc.size() ? sc : "stage/jungle" );
    setup_ui_by_config( c1p, std::string(), "ui/in_game_2p_layout" );

    min_ = 0, sec_ = 0 ,last_garbage_1p_ = 0, last_garbage_2p_ = 0;
    win_ = false, fired_ = false, end_ = false;

    //start music
    stage_->playBGM();

    //note: bad area
    timer_ui_   = pDummy(new int);
    btn_single_shot_ = pDummy(new int);
    //note: end of bad area

    using std::tr1::bind;
    ctrl::EventDispatcher::i().subscribe_timer(
        bind(&Puzzle::update_ui_by_second, this), timer_ui_, 1000, -1);
    ctrl::EventDispatcher::i().subscribe_timer(
        bind(&App::setLoading, &App::i(), 100), 100); //stupid and must?

    ctrl::EventDispatcher::i().subscribe_btn_event(
        bind(&Puzzle::single_shot, this), btn_single_shot_, &player0_->input()->trig1(), ctrl::BTN_PRESS);

    return shared_from_this();
}

void Puzzle::setup_ui_by_config( std::string const& c1p, std::string const& c2p, std::string const& path )
{
    uiconf_ = Conf::i().config_of(path);
    utils::map_any const& base = uiconf_.M("base");
    ui_layout_ = view::Menu::create( base.S("layout_tex"), scene_, base.I("w"), base.I("h") );
    ui_layout_->set<Alpha>(192);

    utils::map_any const& misc = uiconf_.M("misc");
    BOOST_FOREACH(utils::pair_any const& it, misc) {
        std::string    const& key  = boost::any_cast<std::string const>(it.first);
        utils::map_any const& attr = boost::any_cast<utils::map_any const>(it.second);
        ui_layout_->
            addSpriteText(key, attr.S("text"), attr.S("font"), 0, attr.I("fsize"), attr.I("center") )
           .getSpriteText(key).set<Pos2D>( vec2(attr.I("x"), attr.I("y")) );
    }
    //2011.04.05 make stage number equal to puzzle level.
    ui_layout_->getSpriteText("stage").changeText( "level" + to_s(puzzle_level_ - 2) ); //first puzzle have 3 chains.

    vec2 center_pos( uiconf_.I("character_center_x"), uiconf_.I("character_center_y") );
    pview1_ = presenter::PlayerView::create( c1p.size() ? c1p : "config/char/char1.zzml", scene_, center_pos );
    pview1_->setMap( map0_ );
}

void Puzzle::update_ui(){
    int new_garbage_1p_ = map0_->garbage_left() + map1_->current_sum_of_attack();
    int new_garbage_2p_ = map1_->garbage_left() + map0_->current_sum_of_attack();
    ui_layout_->getSpriteText("gar1p").showNumber(new_garbage_1p_, 0);
    ui_layout_->getSpriteText("gar2p").showNumber(new_garbage_2p_, 0);
    ui_layout_->getSpriteText("scr1p").showNumber(map0_->score(), 5);
    ui_layout_->getSpriteText("scr2p").showNumber(map1_->score(), 5);
    //ui_layout_->getSpriteText("wep1p1").showNumber(player0_->weapon(0)->ammo(), 2);
    //ui_layout_->getSpriteText("wep1p2").showNumber(player0_->weapon(1)->ammo(), 2);
    //ui_layout_->getSpriteText("wep1p3").showNumber(player0_->weapon(2)->ammo(), 2);

    last_garbage_1p_ = new_garbage_1p_;
    last_garbage_2p_ = new_garbage_2p_;
}

void Puzzle::update_ui_by_second(){
    ++sec_;
    if( sec_ > 59 ) ++min_, sec_ = 0;
    std::string sec = to_s(sec_); if( sec.size() < 2 ) sec = "0" + sec;
    std::string min = to_s(min_); if( min.size() < 2 ) min = "0" + min;
    ui_layout_->getSpriteText("time").changeText( min + ":" + sec );
}

void Puzzle::single_shot(){
    btn_single_shot_.reset();
    ctrl::EventDispatcher::i().subscribe_timer(
        bind(&ctrl::EventDispatcher::clear_obj_event, &ctrl::EventDispatcher::i(), std::tr1::ref(scene_)), 100);
    fired_ = true;
    //ctrl::EventDispatcher::i().clear_obj_event( scene_ );
}

//note: temp code
void Puzzle::end(pMap lose_map)
{
    end_ = true;
    timer_ui_.reset();
    ctrl::EventDispatcher::i().clear_btn_event();
    audio::Sound::i().stopAll();
    map0_->stop_dropping();
    map1_->stop_dropping();
    player0_->stopAllActions();

    audio::Sound::i().playBuffer( win_ ? "3/3c/win.mp3" : "3/3c/lose.mp3" );
    blocker_ = view::Sprite::create("blocker", scene_, Conf::i().SCREEN_W(), 350, true);
    blocker_->set<Pos2D>( vec2(Conf::i().SCREEN_W() / 2, Conf::i().SCREEN_H() / 2) );
    blocker_->setDepth(-100).set<GradientDiffuse>(0).tween<Linear, Alpha>(0, 100, 500u);

    win_t_  = view::Sprite::create( win_ ? "win" : "lose" , scene_, 384, 192, true);
    //lose_t_ = view::Sprite::create("lose", scene_, 384, 192, true);

    vec2 pos = vec2(Conf::i().SCREEN_W() / 2, Conf::i().SCREEN_H() / 2 - 50);
    win_t_->set<Pos2D>( pos );

    vec3 v0(0,0,0), v1(1,1,1);
    win_t_->setDepth(-450).tween<OElastic, Scale>(v0, v1, 1000u, 0);
    //lose_t_->setDepth(-450).tween<OElastic, Scale>(v0, v1, 1000u, 0);

    end_text_ = view::SpriteText::create("play again?", scene_, "Star Jedi", 30, true);
    end_text2_= view::SpriteText::create("a:yes / b:no", scene_, "Star Jedi", 30, true);
    end_text_->set<Pos2D> ( vec2(Conf::i().SCREEN_W() / 2, Conf::i().SCREEN_H() / 2 + 50) );
    end_text2_->set<Pos2D>( vec2(Conf::i().SCREEN_W() / 2, Conf::i().SCREEN_H() / 2 + 100) );
    end_text_-> set<Alpha>(0).setDepth(-450).tween<Linear, Alpha>(0, 255, 500u, 0, 0, 1000);
    end_text2_->set<Alpha>(0).setDepth(-450).tween<Linear, Alpha>(0, 255, 500u, 0, 0, 1000);
    using std::tr1::bind;
    ctrl::EventDispatcher::i().subscribe_timer(bind(&Puzzle::setup_end_button, this), 1000);
}

void Puzzle::setup_end_button()
{
    using std::tr1::bind;
    std::tr1::function<void(int, int)> clicka = bind(&Puzzle::reinit, this);
    std::tr1::function<void(int, int)> clickb = bind(&Puzzle::end_sequence1, this);

    btn_reinit_ = pDummy(new int);

    ctrl::Input const* input = ctrl::InputMgr::i().getInputByIndex(0);
    ctrl::EventDispatcher::i().subscribe_btn_event(
        clicka, btn_reinit_, &input->trig1(), ctrl::BTN_PRESS);
    ctrl::EventDispatcher::i().subscribe_btn_event(
        clickb, btn_reinit_, &input->trig2(), ctrl::BTN_PRESS);
}

void Puzzle::end_sequence1()
{
    audio::Sound::i().playBuffer("4/4c.wav");
    btn_reinit_.reset();
    stage_->releaseResource(); //release when player isn't going to replay
    App::i().launchMainMenu();
    std::cout << "game_puzzle end call finished." << std::endl;
}

void Puzzle::reinit()
{
    using std::tr1::bind;
    int new_puzzle_lv = win_ ? puzzle_level_+1 : puzzle_level_-1;
    //if( new_puzzle_lv > 8 ) new_puzzle_lv = 8;
    if( new_puzzle_lv > 19 ) new_puzzle_lv = 19;
    else if( new_puzzle_lv < 3 ) new_puzzle_lv = 3;
    audio::Sound::i().playBuffer("4/4b.wav");
    btn_reinit_.reset();
    ctrl::EventDispatcher::i().subscribe_timer(
        bind(&App::launchPuzzle, &App::i(), c1p_, sconf_, new_puzzle_lv), 500);

    std::cout << "game_puzzle end call finished." << std::endl;
}

void Puzzle::cycle()
{
    pview1_->cycle();
    update_ui();
    stage_->cycle();
    scene_->redraw();
    map0_->redraw().cycle();
    map1_->redraw().cycle();

    //note: bad way........ but have no time.
    if( !end_ && map0_->all_empty() ) {
        win_ = true;
        end(map0_);
    }
    else if( !end_ && fired_ && map0_->all_waiting() ) {
        win_ = false;
        end(map0_);
    }
}
