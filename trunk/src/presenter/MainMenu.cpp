
#include "presenter/MainMenu.hpp"
#include "presenter/PlayerView.hpp"
#include "view/Scene.hpp"
#include "view/Menu.hpp"
#include "view/Sprite.hpp"
#include "view/SpriteText.hpp"
#include "data/Color.hpp"
#include "utils/to_s.hpp"

#include "EasingEquations.hpp"
#include "Accessors.hpp"
#include "App.hpp"
#include "Conf.hpp"
#include "EventDispatcher.hpp"
#include "Input.hpp"
#include "Sound.hpp"

#include <tr1/functional>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/function.hpp>
#include <utility>
#include <sstream>
#include <iostream>

using namespace psc;
using namespace presenter;
using namespace accessor;
using namespace easing;
using utils::to_s;

using std::tr1::bind;
using std::tr1::ref;
using std::tr1::cref;
using std::tr1::function;
using namespace std::tr1::placeholders;

namespace BLL = boost::lambda;

MainMenu::~MainMenu()
{
    std::cout << "MainMenu destructing ..." << std::endl;
}

pMainMenu MainMenu::init()
{
    App::i().setLoading(1);

    config = utils::map_any::construct(
        utils::fetchConfig( Conf::i().CONFIG_PRESENTER_PATH +"mainmenu.zzml" ) );

    utils::map_any title = config.M("title");
    utils::map_any text = config.M("text");
    int const m_text_size = Conf::i().SCREEN_W/30;

    mainmenu_scene_ = view::Scene::create("MainMenu");
    mainmenu_scene_->setTo2DView();

    int const width = title.I("orig_w") * (Conf::i().SCREEN_W/1280.0f); //1280 is best screen size.
    int const height= title.I("orig_h") * (Conf::i().SCREEN_W/1280.0f); //1280 is best screen size.

    view::pMenu temp =
        view::Menu::create(title.S("path"), mainmenu_scene_, width, height, true);

    menus_.insert( std::make_pair("start_menu", temp) );
    temp->moveTo( Conf::i().SCREEN_W/2, Conf::i().SCREEN_H/2 - temp->get<Size2D>().Y/3 )
         .setDepth( title.I("depth") );

    function<void(int, int)> clickA = bind(&MainMenu::push_start, this);

    temp->addSpriteText("text", text.S("text"), text.S("font"), 0, m_text_size, true)
         .getSprite("text").set<Pos2D>( vec2( 0, height ) )
         .tween<SineCirc, Alpha>(0, (unsigned int)text.I("glow_period"), -1);

    setupMenus();

    ctrl::EventDispatcher::i().subscribe_timer( bind(&MainMenu::initDecorator, this), shared_from_this(), 30 );

    btn_start_ = pDummy(new int);
    BOOST_FOREACH(ctrl::Input const* input, ctrl::Input::getInputs())
        ctrl::EventDispatcher::i().subscribe_btn_event( clickA, btn_start_, &input->trig1(), ctrl::BTN_PRESS);

    player1focus_ = 0;
    player2focus_ = 0;
    player1num_ = 0;
    player2num_ = 0;
    stagenum_ = 0;
    game_mode_ = MULTI;
    two_players_ = true;

    return shared_from_this();
}

//temporary menu layer setup, need config.
void MainMenu::setupMenus()
{
    view::pMenu mode = view::Menu::create("", mainmenu_scene_, 1, 1, true);
    menus_.insert( std::make_pair("mode_select", mode) );

    function<void(view::pSprite&)> mode1 = bind(&MainMenu::mode_select, this, _1, (int)MULTI);
    function<void(view::pSprite&)> mode2 = bind(&MainMenu::mode_select, this, _1, (int)PUZZLE);

    mode->addSpriteText("text", "choose a game mode", "Star Jedi", 0, 40, true);
    mode->getSprite("text").set<Pos2D>( vec2(-150, -160) );
    mode->addSpriteText("multi", "multiplayer", "Star Jedi", mode1, 40, true);
    mode->getSprite("multi").set<Pos2D>( vec2(-50, -100) );
    mode->addSpriteText("puzzle", "puzzle game", "Star Jedi", mode2, 40, true);
    mode->getSprite("puzzle").set<Pos2D>( vec2(-50, -40) );

    view::pMenu temp = view::Menu::create("", mainmenu_scene_, 1, 1, true);
    menus_.insert( std::make_pair("player_select", temp) );

    temp->addSpriteText("text", "choose character", "Star Jedi", 0, 40, true);
    temp->getSprite("text").set<Pos2D>( vec2(-150, -160) );
    for( int i = 1; i <= 3; ++i ) {
        pvlist_.push_back(
            PlayerView::create(("config/char/char"+to_s(i))+".zzml", temp, vec2(-500 + i*200, 20) ) );
        pvlist_.back()->switchCharacterState( PlayerView::STAND );
        pvlist_.back()->switchCharacterFace( PlayerView::NORMAL );
        if( i != 3 ) pvlist_.back()->flipPosition();
        if( i == 3 ) {
            vec2 pos = pvlist_.back()->getView()->get<Pos2D>(); pos.X += 100;
            pvlist_.back()->getView()->set<Pos2D>( pos );
        }
        pvlist_.back()->getView()->set<Scale>(vec3(1.3, 1.3, 1.3));
    }

    view::pMenu temp2= view::Menu::create("", mainmenu_scene_, 1, 1, true);
    menus_.insert( std::make_pair("stage_select", temp2) );

    function<void(view::pSprite&)> stage1 = bind(&MainMenu::stage_select, this, _1, "jungle");
    function<void(view::pSprite&)> stage2 = bind(&MainMenu::stage_select, this, _1, "school");
    temp2->addSpriteText("text", "choose stage", "Star Jedi", 0, 40, true);
    temp2->addSprite("jungle_thumb", stage1, 300, 180, true)
          .addSprite("school_thumb", stage2, 300, 180, true);
    temp2->getSprite("text").set<Pos2D>( vec2(-150, -160) );
    temp2->getSprite("jungle_thumb").set<Pos2D>( vec2(-200, 50) );
    temp2->getSprite("school_thumb").set<Pos2D>( vec2(200, 50) );

    temp->set<Pos2D>( vec2(-300, Conf::i().SCREEN_H/2) );
    temp2->set<Pos2D>( vec2(-400, Conf::i().SCREEN_H/2) );
    mode->set<Pos2D>( vec2(-300, Conf::i().SCREEN_H/2) );

    player1text_ = view::SpriteText::create("player1", temp, "Star Jedi", 24, true, data::Color(255,0,0));
    player2text_ = view::SpriteText::create("player2", temp, "Star Jedi", 24, true, data::Color(0,0,255));
    player1text_->set<Pos2D>( vec2(-250, 150) ).set<Alpha>(128);
    player2text_->set<Pos2D>( vec2(-250, 180) ).set<Alpha>(128);

    hideMenu("mode_select").hideMenu("player_select").hideMenu("stage_select");
}

void MainMenu::end()
{
    if( game_mode_ == MULTI )
        App::i().launchMultiplayer(conf1p_, conf2p_, stage_);
    else if( game_mode_ == PUZZLE )
        App::i().launchPuzzle(conf1p_, stage_, 3);
    std::cout << "MainMenu end call finished." << std::endl;
}

void MainMenu::initDecorator()
{
    utils::map_any deco = config.M("decorator");

    int const w = Conf::i().SCREEN_W;
    int const h = Conf::i().SCREEN_H;
    int const size = w * deco.F("size_factor");
    int const num_w = (w/size)*2;
    int const num_h = (h/size)*2 + 3;
    int const outgoing = size * 1.41f;
    int const contract = size / 4;

    vec2 start1( -outgoing, contract),  end1(w+outgoing, contract);     //line1
    vec2 start2(w-contract, -outgoing), end2(w-contract, h+outgoing);   //line2
    vec2 start3(w+outgoing, h-contract),end3( -outgoing, h-contract);   //line3
    vec2 start4( contract,  h+outgoing),end4( contract,  -outgoing);    //line4

//    int const time_w = 10000;            //1280 768 is best factor
//    int const time_h = time_w * (w/h);   //1280 768 is best factor

    int const color_num = deco.I("color_num");
    for(int i = 0; i < color_num; ++i ) {
        paths.push_back( deco.S("path") +
            std::string( utils::to_s((i%4)+1) ) );
    }

    std::vector<vec3> waypoints;
    waypoints.push_back( vec3(contract, -contract, 200) );
    waypoints.push_back( vec3(w-contract, -(contract), 100) );
    waypoints.push_back( vec3(w-contract, -(h-contract), 0) );
    waypoints.push_back( vec3(contract, -(h-contract), -100) );
    waypoints.push_back( vec3(contract, outgoing, -200) );

    int capacity = (num_w + num_h - 6)*2;
    for(int i=0; i < capacity; ++i ) {
        data::Color col = data::Color::from_id(0, color_num);
        col.offset();
        int rand_size = size * (utils::random(33)/100.0f + 1);
        view::pSprite temp = view::Sprite::create(
            paths[utils::random(paths.size())], mainmenu_scene_, rand_size, rand_size, true);

        data::WaypointParam<Linear, Pos3D> p1;
        p1.waypoints(waypoints).duration(20000).loop(-1).delay(-(float)i/capacity*20000);
        data::AnimatorParam<Linear, Rotation> p2;
        p2.end(vec3(0,0,360)).duration(utils::random(2000)+3000).loop(-1);

        temp->set<ColorDiffuse>( 0xff000000 | col.rgb() ).tween(p1).tween(p2);

        deco_cubes_.push_back( temp );
    }

    Sound::i().play("title.mp3", true);

    App::i().setLoading(100);
}

void MainMenu::menu1_1_click(view::pSprite& sprite)
{
    if( animating_ ) return;
    animating_ = true;
}

void MainMenu::menu2_1_click(view::pSprite& sprite)
{
    if( animating_ ) return;
    animating_ = true;
}

void MainMenu::fadeAllOut(int dur)
{
    BOOST_FOREACH(MenuPair& mp, menus_)
        mp.second->tweenAll<Linear, Alpha>(0, dur);

    BOOST_FOREACH(view::pSprite& sp, deco_cubes_)
        sp->tween<Linear, Alpha>(0, dur);

    ctrl::EventDispatcher::i().subscribe_timer(bind(&Sound::stopAll, &Sound::i()), dur);
    ctrl::EventDispatcher::i().subscribe_timer(bind(&App::setLoading, &App::i(), 1), dur);
}

void MainMenu::push_start()
{
    Sound::i().play("4/4b.wav");
    btn_start_.reset();
    menus_["start_menu"]->getSprite("text").tween<SineCirc, Alpha>(0, 255, 120u, 2);

    //hideMenu("start_menu").showMenu("player_select");
    hideMenu("start_menu").showMenu("mode_select");
    animating_ = true;
}

void MainMenu::mode_select(view::pSprite& sv, int mode)
{
    if( animating_ ) return;

    game_mode_ = mode;
    Sound::i().play("4/4b.wav");

    menus_["mode_select"]->setCallbackToSprite("multi", 0);
    menus_["mode_select"]->setCallbackToSprite("puzzle", 0);

    if( game_mode_ == SINGLE || game_mode_ == PUZZLE )
        two_players_ = false;

    hideMenu("mode_select").showMenu("player_select");
    animating_ = true;

//--------------------------------------------------------------------
    ctrl::Input const* input1 = ctrl::Input::getInputByIndex(0);
    ctrl::Input const* input2 = ctrl::Input::getInputByIndex(1);

    btn_choose_player1_ = pDummy(new int);

    ctrl::EventDispatcher::i().subscribe_btn_event(
        bind(&MainMenu::player1_select, this, -1), btn_choose_player1_, &input1->wep1(), ctrl::BTN_PRESS);
    ctrl::EventDispatcher::i().subscribe_btn_event(
        bind(&MainMenu::player1_select, this, 1), btn_choose_player1_, &input1->wep2(), ctrl::BTN_PRESS);
    ctrl::EventDispatcher::i().subscribe_btn_event(
        bind(&MainMenu::player1_checked, this), btn_choose_player1_, &input1->trig1(), ctrl::BTN_PRESS);

    player1focus_ = 1;

    if( two_players_ ) {
        btn_choose_player2_ = pDummy(new int);

        ctrl::EventDispatcher::i().subscribe_btn_event(
            bind(&MainMenu::player2_select, this, -1), btn_choose_player2_, &input2->wep1(), ctrl::BTN_PRESS);
        ctrl::EventDispatcher::i().subscribe_btn_event(
            bind(&MainMenu::player2_select, this, 1), btn_choose_player2_, &input2->wep2(), ctrl::BTN_PRESS);
        ctrl::EventDispatcher::i().subscribe_btn_event(
            bind(&MainMenu::player2_checked, this), btn_choose_player2_, &input2->trig1(), ctrl::BTN_PRESS);

        player2focus_ = 1;
    }
    else {
        player2text_->set<Visible>(false);
    }
}

void MainMenu::player1_select(int num)
{
    if( animating_ ) return;
    if( player1focus_ + num < 1 ) return;
    if( player1focus_ + num > 3 ) return;
    Sound::i().play("4/4a.wav");
    player1focus_ += num;
    vec2 pos = player1text_->get<Pos2D>();
    pos.X += 200 * num;
    player1text_->set<Pos2D>(pos);
}

void MainMenu::player2_select(int num)
{
    if( animating_ ) return;
    if( player2focus_ + num < 1 ) return;
    if( player2focus_ + num > 3 ) return;
    Sound::i().play("4/4a.wav");
    player2focus_ += num;
    vec2 pos = player2text_->get<Pos2D>();
    pos.X += 200 * num;
    player2text_->set<Pos2D>(pos);
}

void MainMenu::player1_checked()
{
    Sound::i().play("4/4b.wav");
    player1num_ = player1focus_;
    player1text_->set<Alpha>(255);
    btn_choose_player1_.reset();
    if( two_players_ ) {
        if( player1num_ != 0 && player2num_ != 0 )
            stage_choosing();
    }
    else
        stage_choosing();
}

void MainMenu::player2_checked()
{
    Sound::i().play("4/4b.wav");
    player2num_ = player2focus_;
    player2text_->set<Alpha>(255);
    btn_choose_player2_.reset();
    if( player1num_ != 0 && player2num_ != 0 )
        stage_choosing();
}

void MainMenu::stage_choosing()
{
    animating_ = true;
    hideMenu("player_select").showMenu("stage_select");
}

void MainMenu::stage_select(view::pSprite& sp, std::string name)
{
    if( animating_ ) return;
    conf1p_ = "config/char/char"+to_s(player1num_)+".zzml";
    stage_ = "config/stage/"+name+".zzml";

    if( two_players_ )
        conf2p_ = "config/char/char"+to_s(player2num_)+".zzml";

    Sound::i().play("4/4b.wav");

    fadeAllOut(1000);
    function<void()> cb = bind(&MainMenu::end, this);
    ctrl::EventDispatcher::i().subscribe_timer(cb, shared_from_this(), 1100);

    menus_["stage_select"]->setCallbackToSprite("jungle_thumb", 0);
    menus_["stage_select"]->setCallbackToSprite("school_thumb", 0);
}

MainMenu& MainMenu::showMenu(std::string const& name)
{
    view::pMenu sprite = menus_[name];
    boost::function<void()> const& f = (BLL::var(animating_) = false);
    sprite->tween<OCirc, Pos2D>(vec2(Conf::i().SCREEN_W/2, Conf::i().SCREEN_H/2), 2000, 0, f );
    sprite->tweenAll<Linear, Alpha>(255, 1000);
    sprite->set<Visible>(true);
    return *this;
}

MainMenu& MainMenu::hideMenu(std::string const& name)
{
    view::pMenu sprite = menus_[name];
    function<void()> const& endcall = bind(&view::Sprite::set<Visible>, sprite.get(), false);
    sprite->tween<ICirc, Pos2D>(vec2(-200, Conf::i().SCREEN_H/2), 2000, 0, endcall);
    sprite->tweenAll<Linear, Alpha>(0, 1000);
    return *this;
}

MainMenu& MainMenu::cubeRearrange()
{
    return *this;
}

void MainMenu::cycle()
{
    BOOST_FOREACH(pPlayerView& pv, pvlist_) {
        if( pv->getState() == PlayerView::NONE )
            pv->switchCharacterState( PlayerView::STAND );
    }
    mainmenu_scene_->redraw();
}
