#include"Scene/ScenePlay.h"
#include"Actor/System/Option.h"
#include"Actor/Player/Player.h"
#include"Actor/CPU/CPU.h"
#include"Actor/Paddle/Paddle.h"
#include"Actor/Core/Core.h"
#include"Actor/Gimmick/MidLine.h"
#include"Light.h"
#include"Actor/System/Camera.h"
#include"Actor/System/StageMaker.h"
#include"Actor/System/StageMakerRandom.h"
#include"Actor/System/StageMakerRandom2.h"
#include"Actor/System/TimeSave.h"
#include<GSstandard_shader.h>
#include"Util/UtilNumber.h"
#include"Actor/Effect/PlayReady.h"
#include"Actor/Gimmick/WallDrawManager.h"
#include<gsEffect.h>
#include<fstream>
#include<sstream>
#include"Actor/Effect/SingularityEffect.h"

std::string loadPlayMode()
{
	std::ifstream file("Assets/SaveData/Time.txt");
	std::string mode = "CPU";

	if (file.is_open())
	{
		std::getline(file, mode);
		file.close();
	}
	return mode;
}

static const float ResultTime{ 360.0f };
static const float RerodeTime{ 5.0f };
static const float ReadyTime{ 30.0f };

ScenePlay::ScenePlay() {}

void ScenePlay::start() {

	gsInitDefaultShader();

	gsLoadTexture(Imgae_Scene_End_Effect, "Assets/Scene/SceneEndEffect.png");

	gsLoadBGM(BGM_Play, "Assets/BGM/Play/Cybernetic.ogg", GS_TRUE);
	gsLoadBGM(BGM_Play_Crymax, "Assets/BGM/Play/Unwanted strife.ogg", GS_TRUE);

	world_.next_scene_change(0);
	world_.log_flag_change(false);
	is_game_end_ = false;
	is_end_ = false;
	back_title_ = false;
	is_game_continue_ = false;
	is_result_end_ = false;
	result_timer_ = ResultTime;

	//フィールドのサイズを設定
	world_.field_size_change(GSvector3{ field_min_.x,field_max_.y,0.0f }, 0, 0);
	world_.field_size_change(GSvector3{ 0.0f,field_min_.y,0.0f }, 0, 1);
	world_.field_size_change(GSvector3{ 0.0f,field_max_.y,0.0f }, 1, 0);
	world_.field_size_change(GSvector3{ field_max_.x,field_min_.y,0.0f }, 1, 1);

	world_.in_game_time_change(world_timer_id_, 0.0f);

	CSVReader time_csv("Assets/SaveData/Time.txt");
	CSVReader player_csv("Assets/SaveData/Player.txt");
	std::string play_mode_ = player_csv.get(0, 0);

	world_.add_camera(new Camera{ &world_,camera_first_position_,GSvector3{0.0f,0.0f,-100.0f} });
	world_.add_light(new Light{ &world_ });

	world_.add_option(new Option{ &world_,GKEY_TAB,0 });

	csv_.load(option_save_file_);
	gsSetVolumeBGM(0.5f * csv_.getf(0, 1));

	world_.add_actor(new TimeSave{ &world_,false });

	//初代ステージメーカー
	//world_.add_actor(new StageMaker{ &world_,0 });

	//2代ステージメーカー
	//world_.add_actor(new StageMakerRandom{ &world_,0 });
	//world_.add_actor(new StageMakerRandom{ &world_,1 });

	//3代ステージメーカー
	world_.add_actor(new StageMakerRandom2{ &world_,0});
	world_.stage_make_type(1);

	world_.add_actor(new MidLine(&world_, GSvector3{ 0.0f,0.0f,0.0f }));

	//プレイヤー1生成
	world_.add_actor(new Player(&world_, player_1_start_position_, player_1_name_, "PlayerTag"));
	//パドル生成
	world_.add_actor(new Paddle(&world_, ppaddle_start_position_));

	if (play_mode_ == "Player2")
	{
		//プレイヤー2生成
		world_.add_actor(new Player(&world_, player_2_start_position_, player_2_name_, "PlayerTag"));
	}
	else if (play_mode_ == "CPU")
	{
		//CPU生成
		world_.add_actor(new Player(&world_, player_2_start_position_, cpu_name_, "PlayerTag"));
		world_.add_actor(new CPU{ &world_,0,0 });

	}

	//gsPlayBGM(BGM_Play);
	csv_.load(option_save_file_);
	gsSetVolumeBGM(csv_.getf(0, 1) * 0.5f);
	play_scene_move_effect_ = PSE_Start;
	scene_move_clear_ = 1.0f;

	world_.add_actor(new WallDrawManager{ &world_ });

	//デバッグ用
	//world_.add_actor(new SingularityEffect{ &world_,GSvector3{120.0f,-20.0f,0.0f},0,90.0f });
	//world_.add_actor(new SingularityEffect{ &world_,GSvector3{-120.0f,20.0f,0.0f},0,-90.0f });

}

void ScenePlay::update(float delta_time) {

	world_.update(delta_time);

	//if (gsGetMouseButtonTrigger(GMOUSE_BUTTON_1) || gsXBoxPadButtonTrigger(0, GS_XBOX_PAD_BACK) || gsXBoxPadButtonTrigger(1, GS_XBOX_PAD_BACK)) {

	//	is_end_ = true;

	//}

	if (world_.next_scene() == 4) {

		game_rerode_time_ -= delta_time;

		if (game_rerode_time_ <= 0.0f) {

			world_.next_scene_change(0);
			world_.in_game_time_change(actor_timer_id_, 1.0f);
			world_.in_game_time_change(system_timer_id_, 1.0f);

		}

	}

	else {

		game_rerode_time_ = RerodeTime;

	}

	if (world_.log_flag() >= world_.log_count()) {

		switch (world_.next_scene()) {

			//リザルト
		case 1:
			play_scene_move_effect_ = PSE_End;
			break;

			//タイトル
		case 2:
			play_scene_move_effect_ = PSE_End;
			back_title_ = true;
			break;

			//強制終了
		case game_end_number_:
			play_scene_move_effect_ = PSE_End;
			is_game_end_ = true;
			break;

		default:
			break;

		}

	}

	switch (play_scene_move_effect_) {

	case PSE_Start:

		csv_.load(option_save_file_);
		scene_move_change_ = delta_time * end_clear_speed_;
		scene_move_clear_ -= scene_move_change_;

		if (scene_move_clear_ <= 0.0f) {

			world_.in_game_time_change(world_timer_id_, 1.0f);
			gsDeleteBGM(BGM_Title);
			gsDeleteBGM(BGM_Result);
			play_scene_move_effect_ = PSE_DontPlay;
			scene_move_clear_ = 0.0f;
			gsSetVolumeBGM(csv_.getf(0, 1));
			world_.add_actor(new PlayReady{ &world_,GSvector2{ScreenWidth / 2.0f,ScreenHeight / 2.0f},ReadyTime });

		}

		break;

	case PSE_End:

		csv_.load(option_save_file_);
		scene_move_change_ = delta_time * end_clear_speed_;
		scene_move_clear_ += scene_move_change_;
		gsSetVolumeBGM((1.0f - (scene_move_clear_ / 2.0f)) * csv_.getf(0, 1));

		if (scene_move_clear_ >= 1.1f) {

			is_end_ = true;

			if (is_game_continue_)next_scene_=scene_play_name_;
			if (is_game_end_)next_scene_=scene_game_end_name_;
			else if (back_title_)next_scene_=scene_title_name_;
			else next_scene_=scene_result_name_;

		}

		break;

	default:
		break;

	}

}

void ScenePlay::draw()const {

	//gsFontParameter(GS_FONT_NORMAL, 60, "MSゴシック");
	//gsTextPos(200, 200);
	world_.draw();

	if (play_scene_move_effect_ != 1) {

		GSvector2 pos{ 0.0f,0.0f };
		GSrect rect{ 0.0f,0.0f,ScreenWidth,ScreenHeight };
		GScolor color{ 1.0f,1.0f,1.0f,scene_move_clear_ };
		gsDrawSprite2D(Imgae_Scene_End_Effect, &pos, &rect, NULL, &color, NULL, NULL);

	}

}

void ScenePlay::end() {

	world_.clear();//ワールドのクリアー処理

	gsDeleteTexture(Imgae_Scene_End_Effect);

	gsDeleteBGM(BGM_Play);//プレイ中のBGMの消去
	gsDeleteBGM(BGM_Play_Crymax);

}

bool ScenePlay::is_end()const {

	return is_end_;

}

string ScenePlay::next()const {

	return scene_load_name_;
}

string ScenePlay::want_change_next()const {

	return next_scene_;

}

void ScenePlay::before(string& name) {

	before_scene_name_ = name;

}