#include"Scene/SceneResult.h"
#include"Actor/System/Option.h"
#include"Field.h"
#include"Light.h"
#include"Actor/System/Camera.h"
#include"Util/UtilNumber.h"
#include <fstream>
#include<sstream>
#include <string>

SceneResult::SceneResult() :
	selected_index_(0)
{
}

const GSvector2 image_size_{ 256.0f,256.0f };
const GSvector2 ui_size_{ 512.0f,128.0f };
const GSvector2 yellowbar_size_{ 296.0f,6.0f };

static float blink_timer_ = 0.0f;

void SceneResult::start() {

	gsLoadTexture(Imgae_Scene_End_Effect, "Assets/Scene/SceneEndEffect.png");
	gsLoadBGM(BGM_Result, "Assets/BGM/Result/resault_bgm.ogg", GS_TRUE);

	world_.add_camera(new Camera{ &world_,GSvector3{0.0f,3.0f,-5.0f},GSvector3{0.0f,1.7f,0.0f} });
	world_.add_light(new Light{ &world_ });

	world_.add_option(new Option{ &world_,GKEY_TAB,-1 });
	play_scene_move_effect_ = RSE_Start;
	scene_move_clear_ = 1.0f;

	csv_.load(option_save_file_);
	world_.next_scene_change(0);
	world_.log_flag_change(false);
	is_game_end_ = false;
	is_end_ = false;

	std::ifstream time_file("Assets/SaveData/Time.txt");
	if (time_file)
	{
		std::string line;
		if (std::getline(time_file, line))
		{
			std::stringstream ss(line);
			char comma;

			ss >> game_time_ >> comma >> score_p1_ >> comma >> score_p2_;
		}
	}
	else
	{
		game_time_ = 0.0f;
		score_p1_ = 0;
		score_p2_ = 0;

	}

	std::ifstream player_file("Assets/SaveData/Player.txt");
	if (player_file)
	{
		std::getline(player_file, opponent_mode_);
	}
	else
	{
		opponent_mode_ = "CPU"; //ファイルがなければCPU扱い
	}

	blink_timer_ = 0.0f;
}

void SceneResult::update(float delta_time) {

	world_.update(delta_time);

	blink_timer_ += delta_time * 0.1f;

	stic_0_ = { 0.0f,0.0f };
	stic_1_ = { 0.0f,0.0f };
	gsXBoxPadGetLeftAxis(0, &stic_0_);
	gsXBoxPadGetLeftAxis(1, &stic_1_);

	if (gsGetKeyTrigger(GKEY_A) || gsXBoxPadButtonState(0, GS_XBOX_PAD_RIGHT) || stic_0_.x < 0.0f || stic_1_.x < 0.0f)
	{
		gsPlaySE(SE_ChangeSelect);
		selected_index_ = 0; // リトライ
	}
	else if (gsGetKeyTrigger(GKEY_D) || gsXBoxPadButtonState(0, GS_XBOX_PAD_RIGHT) || stic_0_.x > 0.0f || stic_1_.x > 0.0f)
	{
		gsPlaySE(SE_ChangeSelect);
		selected_index_ = 1; // タイトル
	}

	if (gsGetKeyTrigger(GKEY_SPACE)/* || gsGetMouseButtonTrigger(GMOUSE_BUTTON_1) */ || gsXBoxPadButtonTrigger(0, GS_XBOX_PAD_A) || gsXBoxPadButtonTrigger(1, GS_XBOX_PAD_A)) {

		gsPlaySE(SE_Select);
		play_scene_move_effect_ = RSE_End;

	}

	if (world_.next_scene() == game_end_number_) {

		is_game_end_ = true;
		play_scene_move_effect_ = RSE_End;

	}

	//暗転システム
	switch (play_scene_move_effect_) {

	case RSE_Start:

		scene_move_change_ = delta_time * end_clear_speed_;
		scene_move_clear_ -= scene_move_change_;
		gsSetVolumeBGM(scene_move_clear_ * csv_.getf(0, 1));

		if (scene_move_clear_ <= 0.0f) {

			play_scene_move_effect_ = RSE_DontPlay;
			scene_move_clear_ = 0.0f;
			gsDeleteBGM(BGM_Play);
			gsPlayBGM(BGM_Result);
			gsSetVolumeBGM(csv_.getf(0, 1));

		}

		break;

	case RSE_End:

		scene_move_change_ = delta_time * end_clear_speed_;
		scene_move_clear_ += scene_move_change_;
		gsSetVolumeBGM((1.0f - (scene_move_clear_ / 2.0f)) * csv_.getf(0, 1));

		if (scene_move_clear_ >= 1.1f) {

			is_end_ = true;

			if (is_game_end_ == true)next_scene_=scene_game_end_name_;
			else if (selected_index_ == 0)next_scene_= before_scene_name_;
			else next_scene_= scene_title_name_;


		}

		break;

	default:
		break;

	}

}

void SceneResult::draw()const {
	//UIのrect、center
	GSrect ui_rect{ 0.0f,0.0f,ui_size_.x,ui_size_.y };
	GSvector2 ui_center{ ui_size_.x / 2.0f,ui_size_.y / 2.0f };

	//Player、Numberなどのrect、center
	GSrect image_rect{ 0.0f,0.0f,image_size_.x,image_size_.y };
	GSvector2 image_center{ image_size_.x / 2.0f,image_size_.y / 2.0f };

	//リザルトロゴ
	GSvector2 log_pos{ ScreenWidth / 2.0f,200.0f };
	gsDrawSprite2D(Image_Result_Logo, &log_pos, &ui_rect, &ui_center, NULL, NULL, 0.0f);

	//プレイヤーテクスチャ
	float player_offset_{ 400.0f };
	GSvector2 player1_pos{ (ScreenWidth / 2.0f) - player_offset_,ScreenHeight / 2.0f };
	GSvector2 player2_pos{ (ScreenWidth / 2.0f) + player_offset_,ScreenHeight / 2.0f };
	gsDrawSprite2D(Image_Player_Default, &player1_pos, &image_rect, &image_center, NULL, NULL, 0.0f);
	gsDrawSprite2D(Image_Player2_Default, &player2_pos, &image_rect, &image_center, NULL, NULL, 0.0f);

	//スコア棒
	GSvector2 bar_pos{ ScreenWidth / 2.0f,ScreenHeight / 2.0f };
	gsDrawSprite2D(Image_Result_Scorebar, &bar_pos, &image_rect, &image_center, NULL, NULL, 0.0f);

	//Winテクスチャ
	GSvector2 win_pos{ 0.0f,350.0f };
	GSvector2 win_scale{ 0.5f,0.5f };

	//勝ったプレイヤーの数字
	GSvector2 winplayer_pos{ ScreenWidth / 2.0f,730.0f };
	GSvector2 winplayer_scale{ 0.7f,0.7f };

	//スコア表示
	GSvector2 number_pos{ 980.0f,730.0f };
	GSrect number1_rect{ 256.0f,0.0f,512.0f,256.0f };
	GSrect number2_rect{ 512.0f,0.0f,768.0f,256.0f };
	GSvector2 number_scale{ 0.24f,0.24f };

	//スコアの位置
	float score_offset_{ 200.0f };
	GSvector2 score_p1_pos = { (ScreenWidth / 2.0f) - score_offset_,ScreenHeight / 2.0f };
	GSvector2 score_p2_pos = { (ScreenWidth / 2.0f) + score_offset_,ScreenHeight / 2.0f };
	float score_scale = 1.0f;

	draw_number(score_p1_, score_p1_pos, score_scale);
	draw_number(score_p2_, score_p2_pos, score_scale);

	if (score_p1_ > score_p2_)
	{
		win_pos = { score_p1_pos.x,360.0f };
		gsDrawSprite2D(Image_Result_Win, &win_pos, &ui_rect, &ui_center, NULL, &win_scale, 0.0f);
		gsDrawSprite2D(Image_Result_Winplayer, &winplayer_pos, &ui_rect, &ui_center, NULL, &winplayer_scale, 0.0f);
		gsDrawSprite2D(Image_Number_Font, &number_pos, &number1_rect, &image_center, NULL, &number_scale, 0.0f);;
	}
	else if (score_p2_ > score_p1_)
	{
		win_pos = { score_p2_pos.x,360.0f };
		gsDrawSprite2D(Image_Result_Win, &win_pos, &ui_rect, &ui_center, NULL, &win_scale, 0.0f);
		gsDrawSprite2D(Image_Number_Font, &number_pos, &number2_rect, &image_center, NULL, &number_scale, 0.0f);

		if (opponent_mode_ == "Player2")
		{
			gsDrawSprite2D(Image_Result_Winplayer, &winplayer_pos, &ui_rect, &ui_center, NULL, &winplayer_scale, 0.0f);
		}
		else
		{
			gsDrawSprite2D(Image_Result_Winplayer, &winplayer_pos, &ui_rect, &ui_center, NULL, &winplayer_scale, 0.0f);
		}
	}

	GSvector2 select1_pos = { 600.0f,900.0f };
	GSvector2 select2_pos = { 1300.0f,900.0f };
	GSvector2 select_scale{ 0.6f,0.6f };


	GSvector2 yellowbar_pos{ select1_pos.x,930.0f };
	GSrect yellowbar_rect{ 0.0f,0.0f,yellowbar_size_.x,yellowbar_size_.y };
	GSvector2 yellowbar_center{ yellowbar_size_.x / 2.0f,yellowbar_size_.y / 2.0f };
	GSvector2 yellow_scale = { 1.2f,1.0f };

	float alpha = 0.5f + 0.5f * sinf(blink_timer_);
	GScolor color(1.0f, 1.0f, 1.0f, alpha);

	if (selected_index_ == 0)
	{
		gsDrawSprite2D(Image_Result_Retry, &select1_pos, &ui_rect, &ui_center, NULL, &select_scale, 0.0f);
		yellowbar_pos = { select1_pos.x,930.0f };
		gsDrawSprite2D(Image_UI_YellowBar, &yellowbar_pos, &yellowbar_rect, &yellowbar_center, &color, NULL, 0.0f);

	}
	else
		gsDrawSprite2D(Image_Result_Retry, &select1_pos, &ui_rect, &ui_center, NULL, &select_scale, 0.0f);

	if (selected_index_ == 1)
	{
		gsDrawSprite2D(Image_Result_Backtitle, &select2_pos, &ui_rect, &ui_center, NULL, &select_scale, 0.0f);
		yellowbar_pos = { select2_pos.x,930.0f };
		gsDrawSprite2D(Image_UI_YellowBar, &yellowbar_pos, &yellowbar_rect, &yellowbar_center, &color, &yellow_scale, 0.0f);
	}
	else
		gsDrawSprite2D(Image_Result_Backtitle, &select2_pos, &ui_rect, &ui_center, NULL, &select_scale, 0.0f);

	world_.draw();

	if (play_scene_move_effect_ != 1) {

		GSvector2 pos{ 0.0f,0.0f };
		GSrect rect{ 0.0f,0.0f,ScreenWidth,ScreenHeight };
		GScolor color{ 1.0f,1.0f,1.0f,scene_move_clear_ };
		gsDrawSprite2D(Imgae_Scene_End_Effect, &pos, &rect, NULL, &color, NULL, NULL);
	}
}

void SceneResult::draw_number(int number, const GSvector2& pos, float scale)const
{
	float font_width = 256.0f;
	float font_height = 256.0f;

	std::string number_str_ = std::to_string(number);

	for (size_t i = 0; i < number_str_.size(); i++)
	{
		int digit = number_str_[i] - '0';

		GSrect rect;
		rect.left = font_width * digit;
		rect.top = 0.0f;
		rect.right = rect.left + font_width;
		rect.bottom = font_height;

		// 描画位置
		GSvector2 number_pos = { pos.x + (font_width * scale * i), pos.y };

		// 中心（必要なら調整）
		GSvector2 number_center = { font_width / 2.0f, font_height / 2.0f };

		GSvector2 number_scale_ = { scale,scale };

		gsDrawSprite2D(Image_Number_Font, &number_pos, &rect, &number_center, NULL, &number_scale_, 0.0f);
	}
}

void SceneResult::end() {

	world_.clear();
	gsDeleteTexture(Imgae_Scene_End_Effect);

}

bool SceneResult::is_end()const {

	return is_end_;

}

string SceneResult::next()const {

	return scene_load_name_;

}

string SceneResult::want_change_next()const {

	return next_scene_;

}

void SceneResult::before(string& name){

	before_scene_name_ = name;

}