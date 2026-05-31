#include"Scene/SceneTitle.h"
#include"Actor/System/Option.h"
#include"Field.h"
#include"Light.h"
#include"Actor/System/Camera.h"
#include"Util/UtilNumber.h"
#include<fstream>
#include<string>
#include"Actor/Core/Core.h"
#include"Actor/Player/Player.h"
#include"Actor/System/TimeSave.h"
#include"Actor/Paddle/Paddle.h"
#include"Actor/Gimmick/MidLine.h"
#include"Actor/Gimmick/WallDrawManager.h"
#include"Actor/Effect/LogoFade.h"

enum class PlayMode
{
	Player2,
	CPU,
	Credit,
};

const GSvector2 ui_size_{ 512.0f,128.0f };
const GSvector2 yellowbar_size_{ 297.0f,6.0f };

static PlayMode selected_mode_ = PlayMode::CPU;
static float blink_timer_ = 0.0f;

static const float StanTimeMax{ 5.0f };//オプション戻り時の一瞬スタン
static const int SelectModeMax{ 4 };//モードの数	4つ目クレジット追加
static const GSvector2 play_solo_{ 300.0f,700.0f };
static const GSvector2 play_duo_{ (ScreenWidth / 2.0f)-200.0f,700.0f };
static const GSvector2 play_online_{ 1200.0f,700.0f };
static const GSvector2 credit_{ 1650.0f,700.0f };
static const float yellow_bar_y_add_{ 30.0f };

SceneTitle::SceneTitle() {}

void SceneTitle::start() {

	gsLoadTexture(Imgae_Scene_End_Effect, "Assets/Scene/SceneEndEffect.png");

	world_.add_camera(new Camera{ &world_,camera_first_position_,GSvector3{0.0f,0.0f,-100.0f} });
	world_.add_light(new Light{ &world_ });
	world_.add_option(new Option{ &world_,GKEY_TAB,0 });

	world_.next_scene_change(0);
	world_.log_flag_change(false);
	is_game_end_ = false;
	is_end_ = false;
	play_scene_move_effect_ = TSE_Start;

	scene_move_clear_ = 1.0f;

	csv_.load(option_save_file_);

	blink_timer_ = 0.0f;
	test_online_ = false;
	test_online_server_ = false;
	credit_move_ = false;

	for (int i = 0; i < SelectModeMax; i++) {

		switch (i) {

		case 0:
			mode_ui_position_.push_back(play_solo_);
			break;

		case 1:
			mode_ui_position_.push_back(play_duo_);
			break;

		case 2:
			mode_ui_position_.push_back(play_online_);
			break;

		case 3:
			mode_ui_position_.push_back(credit_);
			break;

		}

	}

}

void SceneTitle::update(float delta_time) {

	//サーバー起動
	if (gsGetKeyTrigger(GKEY_1) && gsGetMouseButtonState(GMOUSE_BUTTON_1)) {

		test_online_server_ = true;
		play_scene_move_effect_ = TSE_End;

	}

	world_.update(delta_time);

	blink_timer_ += delta_time * 0.1f;

	stic_0_ = { 0.0f,0.0f };
	stic_1_ = { 0.0f,0.0f };
	gsXBoxPadGetLeftAxis(0, &stic_0_);
	gsXBoxPadGetLeftAxis(1, &stic_1_);

	//オプション中はタイトル画面動かない
	if (!world_.option_enable()) {

		if (stan_time_ <= 0.0f) {

			//キー移動 スティックが爆速やからそれを直す
			if (gsGetKeyState(GKEY_A) || gsXBoxPadButtonState(0, GS_XBOX_PAD_LEFT) || stic_0_.x < 0.0f || stic_1_.x < 0.0f)
			{
				select_change(-1, delta_time, SelectModeMax);
			}

			else if (gsGetKeyState(GKEY_D) || gsXBoxPadButtonState(0, GS_XBOX_PAD_RIGHT) || stic_0_.x > 0.0f || stic_1_.x < 0.0f)
			{
				select_change(1, delta_time, SelectModeMax);
			}

			else {

				down_speed_ = 0.0f;
				down_wait_ = 0.0f;
				first_cont_flag_ = true;

			}

			if (gsGetKeyTrigger(GKEY_SPACE) || gsXBoxPadButtonTrigger(0, GS_XBOX_PAD_A) || gsXBoxPadButtonTrigger(1, GS_XBOX_PAD_A)) {

				gsPlaySE(SE_Select);

				bool online = false;
				bool credit = false;

				//モード選択
				switch (current_serrect_) {

				case 0:
					selected_mode_ = PlayMode::CPU;
					break;

				case 1:
					selected_mode_ = PlayMode::Player2;
					break;

				case 2:
					online = true;
					break;

				case 3:
					credit = true;
					break;

				default:
					return;
					break;

				}

				//オフラインプレイ
				if (!online && !credit) {

					std::ofstream time_file("Assets/SaveData/Time.txt", std::ios::trunc);
					if (time_file)
					{
						time_file << "60.0000000,0,0";
					}

					std::ofstream player_file("Assets/SaveData/Player.txt", std::ios::trunc);
					if (player_file)
					{
						player_file << ((selected_mode_ == PlayMode::CPU) ? "CPU" : "Player2");
					}

				}

				//オンラインプレイ
				else if(online){

					test_online_ = true;

				}

				//クレジット
				else if (credit)
				{
					credit_move_ = true;
				}

				play_scene_move_effect_ = TSE_End;

			}

		}

		else {

			stan_time_ -= delta_time;

		}

	}

	else {

		stan_time_ = StanTimeMax;

	}


	if (world_.next_scene() == game_end_number_) {
		is_game_end_ = true;
		play_scene_move_effect_ = TSE_End;
	}

	//暗転システム
	switch (play_scene_move_effect_) {

	case TSE_Start:

		scene_move_change_ = delta_time * end_clear_speed_;
		scene_move_clear_ -= scene_move_change_;
		gsSetVolumeBGM(scene_move_clear_ * csv_.getf(0, 1));

		if (scene_move_clear_ <= 0.0f) {
			gsDeleteBGM(BGM_Title);
			gsDeleteBGM(BGM_Play);
			gsDeleteBGM(BGM_Play_Crymax);
			gsDeleteBGM(BGM_Result);
			gsLoadBGM(BGM_Title, "Assets/BGM/Title/title_bgm.ogg", GS_TRUE);
			gsPlayBGM(BGM_Title);
			play_scene_move_effect_ = TSE_DontPlay;
			scene_move_clear_ = 0.0f;
			gsSetVolumeBGM(csv_.getf(0, 1));

		}

		break;

	case TSE_End:

		scene_move_change_ = delta_time * end_clear_speed_;
		scene_move_clear_ += scene_move_change_;
		gsSetVolumeBGM((1.0f - (scene_move_clear_ / 2.0f)) * csv_.getf(0, 1));

		if (scene_move_clear_ >= 1.1f) {

			is_end_ = true;

			if (is_game_end_)next_scene_ = scene_game_end_name_;
			else if (test_online_) next_scene_ = scene_online_test_name_;
			else if (test_online_server_)next_scene_ = scene_online_test_server_name_;
			else if (credit_move_)next_scene_ = scene_credit_name_;
			else next_scene_ = scene_play_name_;
			
		}

		break;

	default:
		break;

	}

	ui_timer_ += delta_time;

}

void SceneTitle::draw()const {

	GSvector2 log_pos{ ScreenWidth / 2.0f,400.0f };
	GSvector2 logo_scale{ 1.5f,1.5f };

	GSvector2 ui_pos1{ mode_ui_position_.at(0) };
	GSvector2 ui_pos2{ mode_ui_position_.at(1) };
	GSvector2 ui_pos3{ mode_ui_position_.at(2) };
	GSvector2 ui_pos4{ mode_ui_position_.at(3) };
	GSrect ui_rect{ 0.0f,0.0f,ui_size_.x,ui_size_.y };
	GSvector2 ui_center{ ui_size_.x / 2.0f,ui_size_.y / 2.0f };
	GSvector2 ui_scale{ 0.6f,0.6f };

	//タイトルロゴ
	gsDrawSprite2D(Image_Title_Logo, &log_pos, &ui_rect, &ui_center, NULL, &logo_scale, 0.0f);
	//選択肢の描画
	gsDrawSprite2D(Image_Title_P1, &ui_pos1, &ui_rect, &ui_center, NULL, &ui_scale, 0.0f);
	gsDrawSprite2D(Image_Title_P2, &ui_pos2, &ui_rect, &ui_center, NULL, &ui_scale, 0.0f);
	gsDrawSprite2D(Image_Title_Online, &ui_pos3, &ui_rect, &ui_center, NULL, &ui_scale, 0.0f);
	gsDrawSprite2D(Imgae_Scene_Credit_logo, &ui_pos4, &ui_rect, &ui_center, NULL, &ui_scale, 0.0f);//クレジットに行く画像

	GSvector2 yellowbar_pos{ mode_ui_position_.at(current_serrect_).x,mode_ui_position_.at(current_serrect_).y + yellow_bar_y_add_ };
	GSrect yellowbar_rect{ 0.0f,0.0f,yellowbar_size_.x,yellowbar_size_.y };
	GSvector2 yellowbar_center{ yellowbar_size_.x / 2.0f,yellowbar_size_.y / 2.0f };

	float alpha = 0.5f + 0.5f * sinf(blink_timer_);
	GScolor color(1.0f, 1.0f, 1.0f, alpha);

	gsDrawSprite2D(Image_UI_YellowBar, &yellowbar_pos, &yellowbar_rect, &yellowbar_center, &color, NULL, 0.0f);

	world_.draw();

	if (play_scene_move_effect_ != 1) {

		GSvector2 pos{ 0.0f,0.0f };
		GSrect rect{ 0.0f,0.0f,ScreenWidth,ScreenHeight };
		GScolor color{ 1.0f,1.0f,1.0f,scene_move_clear_ };
		gsDrawSprite2D(Imgae_Scene_End_Effect, &pos, &rect, NULL, &color, NULL, NULL);

	}

}

void SceneTitle::end() {

	world_.clear();
	gsDeleteTexture(Imgae_Scene_End_Effect);

}

bool SceneTitle::is_end()const {

	return is_end_;
}

string SceneTitle::next()const {

	return scene_load_name_;

}

string SceneTitle::want_change_next()const {

	return next_scene_;

}

//選択位置スクロール
void SceneTitle::select_change(int change, float delta_time, int max) {

	if (down_speed_ <= 0.0f) {

		gsPlaySE(SE_ChangeSelect);
		current_serrect_ += change;
		current_serrect_ = CLAMP(current_serrect_, 0, max - 1);

		if (down_wait_ <= 0.0f) {

			if (first_cont_flag_) {

				down_speed_ = down_wait_ = down_wait_max_;
				first_cont_flag_ = false;

			}

			else {

				down_wait_ = 0.0f;

			}

		}

		else {

			down_wait_ -= delta_time;
			down_speed_ = down_wait_;

		}

	}

	down_speed_ -= delta_time;

}