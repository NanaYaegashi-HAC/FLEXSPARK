#ifndef SCENE_TITLE_H_
#define SCENE_TITLE_H_


#include "Scene/IScene.h"
#include"Util/CSVReader.h"

class SceneTitle :public IScene {

public:

	SceneTitle();

	virtual void start()override;
	virtual void update(float delta_tiime)override;
	virtual void draw()const override;
	virtual bool is_end()const override;
	virtual string next()const override;
	virtual void end()override;

	virtual void before(string& name) override {};

	virtual string want_change_next()const override;
	virtual void get_wont_change_scene(string& name)override {}

private:

	void select_change(int change, float delta_time, int max);

private:

	CSVReader csv_;
	World world_;
	bool is_end_{ false };
	bool is_game_end_{ false };
	int play_scene_move_effect_{ 0 };
	float scene_move_clear_{0.0f};

	float down_speed_{ 0.0f };//長押しで下に移動するスピード
	float down_wait_{ 0.0f };//長押しの待機時間
	bool first_cont_flag_{ true };//最初の一回の入力か

	float ui_timer_{ 0.0f };

	float scene_move_change_{0.0f};
	float dec_or_not_{0.0f};

	enum TSEeffectID {

		TSE_Start,
		TSE_DontPlay,
		TSE_End,

	};

	//コントローラーのスティック
	GSvector2 stic_0_{ 0.0f,0.0f };
	GSvector2 stic_1_{ 0.0f,0.0f };

	vector<GSvector2>mode_ui_position_;//ロゴの座標

	bool test_online_{ false };//オンラインテスト用
	bool test_online_server_{ false };//オンラインテスト用

	bool credit_move_{ false };//クレジット用

	float stan_time_{ 0.0f };//オプション閉じたら一瞬スタンかける
	int current_serrect_{ 0 };//次のシーン選ぶ

	string next_scene_;

};

#endif