
#include "view/Scene.hpp"
#include "view/Sprite.hpp"

#include "presenter/Map.hpp"
#include "presenter/cube/ViewSprite.hpp"
#include "presenter/cube/ViewStdout.hpp"

#include "utils/MapLoader.hpp"

#include "EventDispatcher.hpp"
#include "Input.hpp"
#include "Player.hpp"

using namespace psc;

class TestPuzzle{
public:
    TestPuzzle(){
        scene_ = psc::view::Scene::create("TestMapViewScene");
        scene_->setTo2DView().enableGlobalHittingEvent();     //important

        data::pViewSpriteSetting s0, s1;

        s0 = data::ViewSpriteSetting::create(50, 500, 50);
        s0->push_ally(1).push_enemy(0);
        s1 = data::ViewSpriteSetting::create(450, 500, 50);
        s1->push_ally(0).push_enemy(1);

        ///THIS IS IMPORTANT, ALL PLAYERS MUST BE DEFINED FIRST.
        player0_ = ctrl::Player::create(ctrl::Input::getInputByIndex(1), s0->ally_input_ids(), s0->enemy_input_ids());
        player1_ = ctrl::Player::create(ctrl::Input::getInputByIndex(0), s1->ally_input_ids(), s1->enemy_input_ids());

        // setup map0
        map0_ = utils::MapLoader::load("config/puzzle.zzml");
        map0_->set_view_master( presenter::cube::ViewSpriteMaster::create(scene_, s0, player0_) );

        // setup map1
        map1_ = presenter::Map::create();
        map1_->set_view_master( presenter::cube::ViewSpriteMaster::create(scene_, s1, player1_) );

        // mini view
        map1_->push_view_slave( presenter::cube::ViewSpriteMaster::create(scene_,
            data::ViewSpriteSetting::create(800, 300, 25) ) );

        // setup garbage land
        map0_->push_garbage_land(map1_);
        map1_->push_garbage_land(map0_);
    }
    void cycle(){
        scene_->redraw();
        map0_->redraw().cycle();
        map1_->redraw().cycle();
    }

private:
    view::pScene scene_;
    presenter::pMap map0_;
    presenter::pMap map1_;
    ctrl::pPlayer player0_;
    ctrl::pPlayer player1_;
};

#include "App.hpp"
#include <cstdlib> // for srand
#include <ctime> // for time, clock

#include <tr1/functional> // for bind

int main(){
    std::srand(std::time(0)^std::clock()); //  init srand for global rand...
    psc::App::i();
    TestPuzzle tester;
    return psc::App::i().run(std::tr1::bind(&TestPuzzle::cycle, &tester));
}
